#include "grammar.h"

#include <ctype.h>
#include <cstring>

#include "lib/util/dbg/debug.h"

void LexStack_ctor(LexStack* stack, size_t start_size) {
    stack->capacity = start_size;
    stack->buffer = (Lex*) calloc(stack->capacity, sizeof(*stack->buffer));
}

void LexStack_dtor(LexStack* stack) {
    free(stack->buffer);
    stack->capacity = 0;
    stack->size = 0;
}

void LexStack_push(LexStack* stack, Lex lexeme) {
    if (stack->size + 1 >= stack->capacity) {
        stack->capacity *= 2;
        Lex* new_buf = (Lex*)calloc(stack->capacity, sizeof(*stack->buffer));
        _LOG_FAIL_CHECK_(new_buf, "error", ERROR_REPORTS, return, &errno, ENOMEM);
        memcpy(new_buf, stack->buffer, stack->size * sizeof(*stack->buffer));
        free(stack->buffer);
        stack->buffer = new_buf;
        for (size_t id = stack->size; id < stack->capacity; ++id) {
            stack->buffer[id] = {};
        }
    }
    stack->buffer[stack->size++] = lexeme;
}

#define CUR_PTR iter

#define LEXEME(name, condition, shift_formula, value_formula)                                       \
    else if ( condition ) {                                                                         \
        shift = (unsigned int)( shift_formula );                                                    \
        LexStack_push(&stack, { .type = LEX_##name, .value = value_formula, .address = address });  \
    }

LexStack lexify(const char* line) {
    LexStack stack = {};
    LexStack_ctor(&stack, 1);

    const char* iter = line;

    CharAddress address = { .line = 1, .index = 0 };

    bool in_comment = false;

    for (unsigned int shift = 1; *iter != '\0'; iter += shift, address.index += shift, shift = 1) {
        if (*iter == '\n') {
            ++address.line;
            address.index = 0;
        }

        if (begins_with(iter, "/*")) {
            in_comment = true;
            shift = 2;
        } else if (begins_with(iter, "*/")) {
            in_comment = false;
            shift = 2;
            continue;
        }

        if (in_comment) continue;

        if (!isprint(*iter) || isblank(*iter)) continue;
        else if (isdigit(*iter)) {
            const char* num_end = iter;
            LexStack_push(&stack, { .type = LEX_NUM,
                                    .value = { .dbl = strtof(iter, (char**)&num_end) },
                                    .address = address });
            shift = (unsigned int)(num_end - iter);
        } 

        #include "lexemes.hpp"

        else {
            log_dup(ERROR_REPORTS, "error", "Unknown symbol at location %d in line %d.\n",
                                            (int)address.index, (int)address.line);
        }
    }
    LexStack_push(&stack, { .type = LEX_END, .address = address });

    return stack;
}

#undef LEXEME


#define CHECK_LEXEME(lexeme, message) do {          \
    if (CURRENT.type != lexeme) CERROR(message);    \
    ++*caret;                                       \
} while (0)

#define CERROR(...) do {                                            \
    log_dup(ERROR_REPORTS, "error", "At symbol %d (line %d)\n",     \
            (int)CURRENT.address.index, (int)CURRENT.address.line); \
    log_dup(ERROR_REPORTS, "error", __VA_ARGS__);                   \
    return NULL;                                                    \
}while (0)

#define PARSING_FUNCTION(name) TreeNode* parse_##name(const LexStack stack, int* caret)

#define VALIDITY_CHECK() do {                                                               \
    _LOG_FAIL_CHECK_(stack.buffer, "error", ERROR_REPORTS, return NULL, &errno, EFAULT);    \
    _LOG_FAIL_CHECK_(caret, "error", ERROR_REPORTS, return NULL, &errno, EFAULT);           \
    log_printf(STATUS_REPORTS, "status", "Parsing symbol %d (line %d) with function %s\n",  \
                (int)CURRENT.address.index, (int)CURRENT.address.line, __PRETTY_FUNCTION__);\
} while (0)

#define CURRENT ( stack.buffer[*caret] )

PARSING_FUNCTION(program) {
    VALIDITY_CHECK();
    TreeNode* node = parse_def_list(stack, caret);
    if (CURRENT.type != LEX_END) {
        CERROR("Expected end of the program.\n");
    }
    return node;
}

PARSING_FUNCTION(def_list) {
    VALIDITY_CHECK();

    TreeNode* node = NULL;
    TreeNode* child = NULL;
    while ((child = parse_definition(stack, caret))) {
        if (!node) node = child;
        else node = TreeNode_new(N_TYPE_DEF_SEQ, {}, node, child);
    }

    return node;
}

PARSING_FUNCTION(definition) {
    VALIDITY_CHECK();
    log_printf(STATUS_REPORTS, "status", "New definition!\n");
    if (CURRENT.type == LEX_NVAR) {
        TreeNode* node = parse_new_var(stack, caret);
        CHECK_LEXEME(LEX_NEXT_STMT, "\';\' expected.\n");
        return node;
    } else if (CURRENT.type == LEX_NDEF) {
        return parse_new_func(stack, caret);
    } else {
        return NULL;
    }
}

PARSING_FUNCTION(new_var) {
    VALIDITY_CHECK();

    CHECK_LEXEME(LEX_NVAR, "Variable definition expected.\n");
    LimitedString name = CURRENT.value.name;

    CHECK_LEXEME(LEX_NAME, "Variable name expected.\n");

    CHECK_LEXEME(LEX_ASS, "Variable value expected.\n");
    TreeNode* node = parse_expression(stack, caret);

    if (!node) return NULL;
    node = TreeNode_new(N_TYPE_NVAR, { .name = name }, NULL, node);

    return node;
}

PARSING_FUNCTION(expression) {
    VALIDITY_CHECK();

    return parse_expr_logic(stack, caret);
}

static inline bool is_comparison(LexType type) {
    return type == LEX_CMP_EQ ||
           type == LEX_CMP_G ||
           type == LEX_CMP_GEQ ||
           type == LEX_CMP_L ||
           type == LEX_CMP_LEQ;
}

static inline Operator lexeme_to_op(LexType type) {
    switch ((int) type) {
    case LEX_CMP_EQ:    return OP_CMP_EQUAL;
    case LEX_CMP_G:     return OP_CMP_G;
    case LEX_CMP_GEQ:   return OP_CMP_GE;
    case LEX_CMP_L:     return OP_CMP_L;
    case LEX_CMP_LEQ:   return OP_CMP_LE;
    case LEX_AND:       return OP_L_AND;
    case LEX_LOR:       return OP_L_OR;

    case LEX_ADD:       return OP_ADD;
    case LEX_SUB:       return OP_SUB;

    case LEX_MUL:       return OP_MUL;
    case LEX_DIV:       return OP_DIV;
    default: return {};
    }
}

PARSING_FUNCTION(expr_logic) {
    VALIDITY_CHECK();
    
    TreeNode* node = parse_expr_comparison(stack, caret);
    if (!node) CERROR("Expression expected.\n");

    while (CURRENT.type == LEX_AND || CURRENT.type == LEX_LOR) {
        LexType lexeme_type = CURRENT.type;
        ++*caret;

        TreeNode* child = parse_expr_comparison(stack, caret);
        if (!child) CERROR("Expression expected.\n");

        node = TreeNode_new(N_TYPE_OP, { .op = lexeme_to_op(lexeme_type) }, node, child);
    }

    return node;
}

PARSING_FUNCTION(expr_comparison) {
    VALIDITY_CHECK();
    
    TreeNode* node = parse_expr_add(stack, caret);
    if (!node) CERROR("Expression expected.\n");

    while (is_comparison(CURRENT.type)) {
        LexType lexeme_type = CURRENT.type;
        ++*caret;

        TreeNode* child = parse_expr_add(stack, caret);
        if (!child) CERROR("Expression expected.\n");

        node = TreeNode_new(N_TYPE_OP, { .op = lexeme_to_op(lexeme_type) }, node, child);
    }

    return node;
}

PARSING_FUNCTION(expr_add) {
    VALIDITY_CHECK();
    
    TreeNode* node = parse_expr_mult(stack, caret);
    if (!node) CERROR("Expression expected.\n");

    while (CURRENT.type == LEX_ADD || CURRENT.type == LEX_SUB) {
        LexType lexeme_type = CURRENT.type;
        ++*caret;

        TreeNode* child = parse_expr_mult(stack, caret);
        if (!child) CERROR("Expression expected.\n");

        node = TreeNode_new(N_TYPE_OP, { .op = lexeme_to_op(lexeme_type) }, node, child);
    }

    return node;
}

PARSING_FUNCTION(expr_mult) {
    VALIDITY_CHECK();
    
    TreeNode* node = parse_expr_brackets(stack, caret);
    if (!node) CERROR("Expression expected.\n");

    while (CURRENT.type == LEX_MUL || CURRENT.type == LEX_DIV) {
        LexType lexeme_type = CURRENT.type;
        ++*caret;

        TreeNode* child = parse_expr_brackets(stack, caret);
        if (!child) CERROR("Expression expected.\n");

        node = TreeNode_new(N_TYPE_OP, { .op = lexeme_to_op(lexeme_type) }, node, child);
    }

    return node;
}

PARSING_FUNCTION(expr_brackets) {
    VALIDITY_CHECK();

    TreeNode* node = NULL;

    if (CURRENT.type == LEX_BRACK_OP) {
        ++*caret;
        node = parse_expression(stack, caret);
        if (CURRENT.type != LEX_BRACK_CL) {
            TreeNode_dtor(&node);
            CERROR("Closing bracket expected.\n");
        }
        ++*caret;
    } else {
        node = parse_expr_derivative(stack, caret);
    }

    return node;
}

PARSING_FUNCTION(expr_derivative) {
    VALIDITY_CHECK();

    TreeNode* node = NULL;

    if (CURRENT.type == LEX_DIFF_BLOCK_OP) {
        ++*caret;
        node = parse_expression(stack, caret);

        CHECK_LEXEME(LEX_DIFF_BLOCK_CL, "Closing square bracket expected.\n");
        CHECK_LEXEME(LEX_BRACK_OP, "Differentiation variable expected.\n");

        LimitedString var_name = CURRENT.value.name;
        CHECK_LEXEME(LEX_NAME, "Variable name expected.\n");

        TreeNode* derivative = TreeNode_diff(node, var_name);
        TreeNode_dtor(&node);
        node = derivative;

        CHECK_LEXEME(LEX_BRACK_CL, "Closing bracket expected.\n");
    } else {
        node = parse_expr_elementary(stack, caret);
    }

    return node;
}

PARSING_FUNCTION(expr_call) {
    VALIDITY_CHECK();

    TreeNode* node = NULL;
    LimitedString name = CURRENT.value.name;
    CHECK_LEXEME(LEX_NAME, "Function name expected.\n");
    CHECK_LEXEME(LEX_BRACK_OP, "Function arguments expected.\n");

    TreeNode* child = parse_arg_list(stack, caret);

    node = TreeNode_new(N_TYPE_CALL, { .name = name }, NULL, child);

    CHECK_LEXEME(LEX_BRACK_CL, "Closing bracket expected.\n");

    return node;
}

PARSING_FUNCTION(arg_list) {
    VALIDITY_CHECK();
    
    if (CURRENT.type == LEX_BRACK_CL) return NULL;

    TreeNode* node = parse_expression(stack, caret);
    if (!node) return NULL;

    while (CURRENT.type == LEX_NEXT_ARG) {
        ++*caret;

        TreeNode* child = parse_expression(stack, caret);
        if (!child) CERROR("Expression expected.\n");

        node = TreeNode_new(N_TYPE_ARG, {}, node, child);
    }

    return node;
}

PARSING_FUNCTION(expr_elementary) {
    VALIDITY_CHECK();
    
    TreeNode* node = NULL;

    if (CURRENT.type == LEX_NAME) {
        if (stack.buffer[*caret + 1].type == LEX_BRACK_OP) {
            node = parse_expr_call(stack, caret);
        } else {
            node = TreeNode_new(N_TYPE_VAR, { .name = CURRENT.value.name }, NULL, NULL);
            ++*caret;
        }
    } else if (CURRENT.type == LEX_NUM) {
        node = TreeNode_new(N_TYPE_CONST, { .dbl = CURRENT.value.dbl }, NULL, NULL);
        ++*caret;
    } else if (CURRENT.type == LEX_SUB) {
        ++*caret;
        if (CURRENT.type == LEX_NUM) {
            node = TreeNode_new(N_TYPE_CONST, { .dbl = -CURRENT.value.dbl }, NULL, NULL);
            ++*caret;
        } else {
            node = TreeNode_new(N_TYPE_OP, { .op = OP_MUL }, 
                                TreeNode_new(N_TYPE_CONST, { .dbl = -1.0 }, NULL, NULL), 
                                parse_expr_brackets(stack, caret));
        }
    } else CERROR("Value was expected.\n");

    return node;
}

PARSING_FUNCTION(new_func) {
    VALIDITY_CHECK();

    TreeNode* node = NULL;

    CHECK_LEXEME(LEX_NDEF, "Function declaration expected.\n");

    LimitedString name = CURRENT.value.name;
    CHECK_LEXEME(LEX_NAME, "Function name expected.\n");
    CHECK_LEXEME(LEX_BRACK_OP, "Parameter list expected.\n");

    TreeNode* child = parse_param_list(stack, caret);

    node = TreeNode_new(N_TYPE_NFUN, { .name = name }, child, NULL);

    CHECK_LEXEME(LEX_BRACK_CL, "Closing bracket expected.\n");

    CHECK_LEXEME(LEX_BLOCK_BRACK_OP, "Function body expected.\n");

    TreeNode* body = parse_block(stack, caret);
    node->right = body;

    CHECK_LEXEME(LEX_BLOCK_BRACK_CL, "Block end expected.\n");

    return node;
}

PARSING_FUNCTION(param_list) {
    VALIDITY_CHECK();
    
    TreeNode* node = NULL;

    if (CURRENT.type != LEX_NAME) return NULL;
    node = TreeNode_new(N_TYPE_PARAM, { .name = CURRENT.value.name }, NULL, NULL);
    ++*caret;

    if (CURRENT.type == LEX_NEXT_ARG) {
        ++*caret;
        node->right = parse_param_list(stack, caret);
    }

    return node;
}

PARSING_FUNCTION(statement) {
    VALIDITY_CHECK();

    TreeNode* node = NULL;

    if (CURRENT.type == LEX_NVAR) {
        node = parse_new_var(stack, caret);
    } else if (CURRENT.type == LEX_NAME && stack.buffer[*caret + 1].type == LEX_BRACK_OP) {
        node = parse_expr_call(stack, caret);
    } else if (CURRENT.type == LEX_NAME && stack.buffer[*caret + 1].type == LEX_ASS) {
        node = parse_assignment(stack, caret);
    } else if (CURRENT.type == LEX_BLOCK_BRACK_OP) {
        ++*caret;

        node = parse_block(stack, caret);

        CHECK_LEXEME(LEX_BLOCK_BRACK_CL, "Block end expected.\n");
    } else if (CURRENT.type == LEX_IF) {
        node = parse_if_cond(stack, caret);
    } else if (CURRENT.type == LEX_WHILE) {
        node = parse_while_cond(stack, caret);
    } else if (CURRENT.type == LEX_RETURN) {
        log_printf(STATUS_REPORTS, "status", "Return command detected!\n");
        ++*caret;
        node = TreeNode_new(N_TYPE_RETURN, {}, NULL, parse_expression(stack, caret));
    }

    return node;
}

PARSING_FUNCTION(block) {
    VALIDITY_CHECK();

    log_printf(STATUS_REPORTS, "status", "New block!\n");

    TreeNode* node = TreeNode_new(N_TYPE_BLOCK, {}, NULL, parse_command_sequence(stack, caret));

    log_printf(STATUS_REPORTS, "status", "Block content has ended on lexeme %s!\n", LEXEME_NAMES[CURRENT.type]);

    return node;
}

PARSING_FUNCTION(command_sequence) {
    VALIDITY_CHECK();
    
    if (CURRENT.type == LEX_BLOCK_BRACK_CL) return NULL;

    log_printf(STATUS_REPORTS, "status", "New statement!\n");

    TreeNode* node = NULL;

    node = parse_statement(stack, caret);

    if (CURRENT.type == LEX_NEXT_STMT) {
        ++*caret;
        TreeNode* next = parse_command_sequence(stack, caret);
        if (next) {
            node = TreeNode_new(N_TYPE_SEQ, {}, node, next);
        }
    }

    return node;
}

PARSING_FUNCTION(assignment) {
    VALIDITY_CHECK();

    LimitedString name = CURRENT.value.name;
    CHECK_LEXEME(LEX_NAME, "Variable name expected.\n");

    CHECK_LEXEME(LEX_ASS,  "Assignment was expected.\n");

    TreeNode* node = TreeNode_new(N_TYPE_ASS, { .name = name }, NULL, parse_expression(stack, caret));

    return node;
}

PARSING_FUNCTION(if_cond) {
    VALIDITY_CHECK();
    CHECK_LEXEME(LEX_IF, "IF statement expected.\n");
    CHECK_LEXEME(LEX_BRACK_OP, "Condition expected.\n");

    TreeNode* condition = parse_expression(stack, caret);

    CHECK_LEXEME(LEX_BRACK_CL, "Closing bracket expected.\n");

    CHECK_LEXEME(LEX_BLOCK_BRACK_OP, "Block start expected.\n");
    TreeNode* true_block = parse_block(stack, caret);
    CHECK_LEXEME(LEX_BLOCK_BRACK_CL, "Block end expected.\n");

    TreeNode* false_block = NULL;

    if (CURRENT.type == LEX_ELSE) {
        CHECK_LEXEME(LEX_BLOCK_BRACK_OP, "Block start expected.\n");
        false_block = parse_block(stack, caret);
        CHECK_LEXEME(LEX_BLOCK_BRACK_CL, "Block end expected.\n");
    }
    return TreeNode_new(N_TYPE_IF, {}, condition, TreeNode_new(N_TYPE_BRANCH, {}, true_block, false_block));
}

PARSING_FUNCTION(while_cond) {
    VALIDITY_CHECK();

    CHECK_LEXEME(LEX_WHILE, "While loop expected.\n");
    CHECK_LEXEME(LEX_BRACK_OP, "Loop condition expected.\n");
    TreeNode* condition = parse_expression(stack, caret);
    CHECK_LEXEME(LEX_BRACK_CL, "Closing bracket expected.\n");

    CHECK_LEXEME(LEX_BLOCK_BRACK_OP, "Loop body expected.\n");
    TreeNode* body = parse_block(stack, caret);
    CHECK_LEXEME(LEX_BLOCK_BRACK_CL, "Loop body end expected.\n");

    return TreeNode_new(N_TYPE_WHILE, {}, condition, body);
}

static inline NodeType node_by_name(LimitedString name) {
    for (size_t id = 0; id < ARR_SIZE(NODE_NAMES); ++id) {
        if (begins_with(name.pointer, NODE_NAMES[id])) return (NodeType)id;
    }
    return {};
}

static inline Operator operator_by_name(LimitedString name) {
    for (size_t id = 0; id < ARR_SIZE(OP_NAMES); ++id) {
        if (begins_with(name.pointer, OP_NAMES[id])) return (Operator)id;
    }
    return {};
}

PARSING_FUNCTION(program_tree) {
    VALIDITY_CHECK();
    CHECK_LEXEME(LEX_BLOCK_BRACK_OP, "Expected opening figure bracket.\n");

    if (CURRENT.type == LEX_BLOCK_BRACK_CL) {
        ++*caret;
        return NULL;
    }

    NodeType type = node_by_name(CURRENT.value.name);
    CHECK_LEXEME(LEX_NAME, "Node name expected.\n");
    CHECK_LEXEME(LEX_NEXT_ARG, "Comma separator expected.\n");

    NodeValue value = {};

    if (type == N_TYPE_OP) {
        value.op = operator_by_name(CURRENT.value.name);
        CHECK_LEXEME(LEX_NAME, "Operator name expected.\n");
    } else if (type == N_TYPE_CONST) {
        int inverter = 1;
        if (CURRENT.type == LEX_SUB) {
            inverter = -1;
            ++*caret;
        }
        value.dbl = inverter * CURRENT.value.dbl;
        CHECK_LEXEME(LEX_NUM, "Constant value expected.\n");
    } else {
        value.name = CURRENT.value.name;
        CHECK_LEXEME(LEX_NAME, "Node value expected.\n");
    }

    CHECK_LEXEME(LEX_NEXT_ARG, "Comma separator expected.\n");

    TreeNode* left = parse_program_tree(stack, caret);

    CHECK_LEXEME(LEX_NEXT_ARG, "Comma separator expected.\n");

    TreeNode* right = parse_program_tree(stack, caret);

    CHECK_LEXEME(LEX_BLOCK_BRACK_CL, "Closing figure bracket expected.\n");

    return TreeNode_new(type, value, left, right);
}
