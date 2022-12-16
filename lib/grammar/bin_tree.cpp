#include "bin_tree.h"

#include <sys/stat.h>
#include <cstring>
#include <time.h>
#include <ctype.h>
#include <math.h>

#include "lib/util/util.h"
#include "lib/util/dbg/debug.h"
#include "lib/file_helper.h"
#include "lib/alloc_tracker/alloc_tracker.h"

#include "tree_config.h"

#define OP_SWITCH_END default:                                                                              \
        if (err_code) *err_code = EINVAL;                                                                   \
        log_printf(ERROR_REPORTS, "error",                                                                  \
            "Somehow Operation equation->value.op had an incorrect value of %d.\n", equation->value.op);    \
        break;                                                                                              \

/**
 * @brief Print subtree to the .dot file.
 * 
 * @param equation subtree to print
 * @param file write destination
 * @param err_code variable to use as errno
 */
static void recursive_graph_dump(const TreeNode* equation, FILE* file, int* const err_code = &errno);

/**
 * @brief Collapse the equation to constant.
 * 
 * @param equation 
 */
static void wrap_constants(TreeNode* equation);

/**
 * @brief Resolve useless operations, such as multiplication by 1 or 0.
 * 
 * @param equation
 */
static void rm_useless(TreeNode* equation);

NameStack NameStack_new(size_t capacity) {
    NameStack created = {};
    created.buffer = (VariableAddress*) calloc(capacity, sizeof(*created.buffer));
    if (!created.buffer) return {};
    created.capacity = capacity;
    created.size = 0;
    return created;
}

void NameStack_dtor(NameStack* stack) {
    free(stack->buffer);
    stack->size = 0;
    stack->capacity = 0;
}

void NameStack_dump(const NameStack* stack, unsigned int importance) {
    log_printf(importance, "ns_dump", "Name stack dump:\n");
    _log_printf(importance, "ns_dump", "Stack size: %ld\n", (long int)stack->size);
    for (size_t id = 0; id < stack->size; id++) {
        VariableAddress address = stack->buffer[id];
        _log_printf(importance, "ns_dump", "\t[%ld] %.*s at %d\n", (long int)id,
                    (int)address.name.length, address.name.pointer, address.address);
    }
}

void NameStack_flush_address(NameStack* stack) {
    NameStack_push(stack, {{}, -1, true});
}

void NameStack_add_variable(NameStack* stack, LimitedString name, bool global) {
    NameStack_push(stack, {name, stack->size ? stack->buffer[stack->size - 1].address + 1 : 0, global});
}

void NameStack_push(NameStack* stack, VariableAddress value) {
    _LOG_FAIL_CHECK_(stack,         "error", ERROR_REPORTS, return, &errno, EINVAL);
    _LOG_FAIL_CHECK_(stack->buffer, "error", ERROR_REPORTS, return, &errno, EINVAL);

    if (stack->size == stack->capacity) {
        VariableAddress* new_buf = (VariableAddress*) calloc(stack->capacity * 2, sizeof(*new_buf));
        if (!new_buf) return;
        memcpy(new_buf, stack->buffer, stack->capacity * sizeof(*new_buf));
        stack->capacity *= 2;
    }

    stack->buffer[stack->size++] = value;
}

void NameStack_pop(NameStack* stack) {
    _LOG_FAIL_CHECK_(stack,         "error", ERROR_REPORTS, return, &errno, EINVAL);
    _LOG_FAIL_CHECK_(stack->buffer, "error", ERROR_REPORTS, return, &errno, EINVAL);
    _LOG_FAIL_CHECK_(stack->size,   "error", ERROR_REPORTS, return, &errno, ENOENT);

    --stack->size;

    stack->buffer[stack->size] = {};
}
VariableAddress NameStack_find(const NameStack* stack, LimitedString name) {
    _LOG_FAIL_CHECK_(stack,         "error", ERROR_REPORTS, return {}, &errno, EINVAL);
    _LOG_FAIL_CHECK_(stack->buffer, "error", ERROR_REPORTS, return {}, &errno, EINVAL);

    for (size_t id = stack->size - 1; id != (size_t)-1; --id) {
        if (!stack->buffer[id].name.pointer) continue;
        if (!strncmp(stack->buffer[id].name.pointer, name.pointer, name.length)) return stack->buffer[id];
    }

    return {};
}

TreeNode* TreeNode_new(NodeType type, NodeValue value, TreeNode* left, TreeNode* right, int* const err_code) {
    TreeNode* equation = (TreeNode*) calloc(1, sizeof(*equation));
    _LOG_FAIL_CHECK_(equation, "error", ERROR_REPORTS, return NULL, err_code, ENOMEM);

    equation->type = type;
    equation->value = value;
    equation->left = left;
    equation->right = right;

    return equation;
}

void TreeNode_dtor(TreeNode** equation) {
    if (!equation)  return;
    if (!*equation) return;

    if ((*equation)->left)  TreeNode_dtor(&(*equation)->left);
    if ((*equation)->right) TreeNode_dtor(&(*equation)->right);
    free(*equation);
    *equation = NULL;
}

static size_t PictCount = 0;

void _TreeNode_dump_graph(const TreeNode* equation, unsigned int importance) {
    start_local_tracking();

    BinaryTree_status_t status = TreeNode_get_error(equation);
    _log_printf(importance, "tree_dump", "\tEquation at %p (status = %d):\n", equation, status);
    if (status) {
        for (size_t error_id = 0; error_id < TREE_REPORT_COUNT; ++error_id) {
            if (status & (1 << error_id)) {
                _log_printf(importance, "tree_dump", "\t%s\n", TREE_STATUS_DESCR[error_id]);
            }
        }
    }

    if (status & ~TREE_INV_CONNECTIONS) return_clean();

    FILE* temp_file = fopen(TREE_TEMP_DOT_FNAME, "w");
    
    _LOG_FAIL_CHECK_(temp_file, "error", ERROR_REPORTS, return_clean(), NULL, 0);

    fputs("digraph G {\n", temp_file);
    fputs(  "\trankdir=TB\n"
            "\tlayout=dot\n"
            , temp_file);

    recursive_graph_dump(equation, temp_file);

    fputc('}', temp_file);
    fclose(temp_file);

    _LOG_FAIL_CHECK_(!system("mkdir -p " TREE_LOG_ASSET_FOLD_NAME),
                     "error", ERROR_REPORTS, return_clean(), NULL, EAGAIN);

    time_t raw_time = 0;
    time(&raw_time);

    const char* pict_name = dynamic_sprintf(TREE_LOG_ASSET_FOLD_NAME "/pict%04ld_%ld.png",
                                            (long int)++PictCount, raw_time);
    _LOG_FAIL_CHECK_(pict_name, "error", ERROR_REPORTS, return_clean(), NULL, EFAULT);
    track_allocation(pict_name, free_variable);

    const char* draw_request = dynamic_sprintf("dot -Tpng -o %s " TREE_TEMP_DOT_FNAME, pict_name);
    track_allocation(draw_request, free_variable);

    log_printf(STATUS_REPORTS, "status", "Draw request: %s.\n", draw_request);
    _LOG_FAIL_CHECK_(!system(draw_request), "error", ERROR_REPORTS, return_clean(), NULL, EAGAIN);

    _log_printf(importance, "tree_dump",
                "\n<details><summary>Graph</summary><img src=\"%s\"></details>\n", pict_name);
    
    return_clean();
}

#define in_brackets(left, code, right, condition) do {  \
    bool __cond = (condition);                          \
    if (__cond) fprintf(file, left);                    \
    code                                                \
    if (__cond) fprintf(file, right);                   \
} while (0)

BinaryTree_status_t TreeNode_get_error(const TreeNode* equation) {
    BinaryTree_status_t status = 0;

    if (equation == NULL) return TREE_NULL;

    #ifndef NDEBUG
        if (equation->left)  status |= TreeNode_get_error(equation->left);
        if (equation->right) status |= TreeNode_get_error(equation->right);
    #endif

    return status;
}

TreeNode* TreeNode_copy(const TreeNode* equation) {
    if (!equation) return NULL;
    return TreeNode_new(equation->type, equation->value, TreeNode_copy(equation->left), TreeNode_copy(equation->right));
}

#define COPY_L  TreeNode_copy(equation->left)
#define COPY_R  TreeNode_copy(equation->right)
#define DELTA_L TreeNode_diff(equation->left,  var_name)
#define DELTA_R TreeNode_diff(equation->right, var_name)

static inline TreeNode* new_op(Operator op, TreeNode* left, TreeNode* right) {
    return TreeNode_new(N_TYPE_OP, { .op = op }, left, right);
}

static inline TreeNode* eq_const(double value) {
    return TreeNode_new(N_TYPE_CONST, { .dbl = value }, NULL, NULL);
}

#define NO_DERIVATIVE TreeNode_copy(equation)
#define EXPR_OPERATOR(name, action, simplification, derivative, compilation_code, restoration_code) \
    case OP_##name: return derivative;

TreeNode* TreeNode_diff(const TreeNode* equation, const LimitedString var_name, int* const err_code) {
    if (!equation) return NULL;

    if (equation->type == N_TYPE_VAR) {
        if (!strcmp(equation->value.name.pointer, var_name.pointer))
            return eq_const(1);
        else
            return TreeNode_copy(equation);

    } else if (equation->type == N_TYPE_CONST) return eq_const(0);

    else if (equation->type == N_TYPE_OP) {
        switch (equation->value.op) {

        #include "operators.hpp"

        OP_SWITCH_END
        }
    }

    return TreeNode_copy(equation);
}

#undef NO_DERIVATIVE
#undef EXPR_OPERATOR

static bool eq_t_const(TreeNode* eq) { return eq->type == N_TYPE_CONST;  }
static bool eq_t_op(TreeNode* eq)    { return eq->type == N_TYPE_OP;     }

#define eq_L ( equation->left )
#define eq_R ( equation->right )

void TreeNode_simplify(TreeNode* equation, int* const err_code) {
    if (!equation) return;
    _LOG_FAIL_CHECK_(!TreeNode_get_error(equation), "error", ERROR_REPORTS, return, err_code, EINVAL);

    if (equation->type != N_TYPE_OP) return;

    TreeNode_simplify(eq_L, err_code);
    TreeNode_simplify(eq_R, err_code);

    wrap_constants(equation);

    rm_useless(equation);
}

static inline bool fprintf_value(FILE* const file, const TreeNode* node) {
    bool answer = true;

    if (node->type == N_TYPE_VAR ||
        node->type == N_TYPE_NVAR ||
        node->type == N_TYPE_CALL ||
        node->type == N_TYPE_NFUN ||
        node->type == N_TYPE_PARAM ||
        node->type == N_TYPE_ASS)
        fprintf(file, " %.*s", (int)node->value.name.length, node->value.name.pointer);
    else if (node->type == N_TYPE_CONST)
        fprintf(file, " %lg", node->value.dbl);
    else if (node->type == N_TYPE_OP)
        fprintf(file, " %s", OP_NAMES[node->value.op]);
    else answer = false;

    return answer;
}

static inline void tabulate(FILE* file, int count) {
    for (int id = 0; id < count; id++) {
        fputc('\t', file);
    }
}

void TreeNode_export(const TreeNode* node, FILE* const file, int nesting) {
    tabulate(file, nesting);

    if (!node) {
        fprintf(file, "{}");
        return;
    }

    fprintf(file, "{%s, ", NODE_NAMES[node->type]);

    if (!fprintf_value(file, node))
        fprintf(file, "null");

    fprintf(file, ", \n");
    TreeNode_export(node->left, file, nesting + 1);
    fprintf(file, ", \n");
    TreeNode_export(node->right, file, nesting + 1);
    fputc('\n', file);
    tabulate(file, nesting);
    fputc('}', file);
}

#define COMPILE(child) TreeNode_compile(var_names, child, file, nesting, label_count)
#define COMPILE_NESTED(child) TreeNode_compile(var_names, child, file, nesting + 1, label_count)
#define PUT(...) do {                               \
    for (int count = 0; count < nesting; ++count)   \
        fputc('\t', file);                          \
    fprintf(file, __VA_ARGS__);                     \
} while (0)

#define ADD_NAME(name) do {                             \
    NameStack_add_variable(var_names, name, !nesting);  \
    NameStack_dump(var_names, STATUS_REPORTS);          \
    PUT("PUSH RAX\n");                                  \
    PUT("PUSH 1\n");                                    \
    PUT("ADD\n");                                       \
    PUT("MOVE RAX\n");                                  \
} while (0)

#define LEFT node->left
#define RIGHT node->right
#define GET_VARIABLE(name) NameStack_find(var_names, name)
#define START_GENERAL_NAMESPACE() \
    NameStack_push(var_names, {{}, var_names->size ? var_names->buffer[var_names->size - 1].address : -1})
#define START_LOCAL_NAMESPACE() NameStack_flush_address(var_names)
#define END_NAMESPACE() do {                                                            \
    while (var_names->size && var_names->buffer[var_names->size - 1].name.pointer) {    \
        NameStack_pop(var_names);                                                       \
    }                                                                                   \
    if (!var_names->buffer[var_names->size - 1].name.pointer)                           \
        NameStack_pop(var_names);                                                       \
} while (0)
#define UNPIN_STACK_FRAME() do {            \
    PUT("MOVE RCX  # Unpin stack frame\n"); \
    PUT("MOVE RBX\n");                      \
    PUT("PUSH RCX\n\n");                    \
} while (0)
#define LABEL_ID (*label_count)

static inline void compile_operator() {

}

#define NODE_TYPE(name, compile_code, restoration_code) case N_TYPE_##name: { {compile_code;} break; }

#define EXPR_OPERATOR(name, action, simplification, derivative, compilation_code, restoration_code) \
    case OP_##name: { {compilation_code;} break; }

void TreeNode_compile(NameStack* var_names, const TreeNode* node, 
                      FILE* const file, int nesting, int* label_count) {
    _LOG_FAIL_CHECK_(var_names,   "error", ERROR_REPORTS, return, &errno, EINVAL);
    _LOG_FAIL_CHECK_(label_count, "error", ERROR_REPORTS, return, &errno, EINVAL);
    if (!node) return;

    if (node->type == N_TYPE_OP) {
        switch(node->value.op) {
            #include "operators.hpp"
            default: break;
        }
        return;
    }

    switch(node->type) {
        #include "node_types.hpp"
        default: break;
    }
}

#undef EXPR_OPERATOR
#undef NODE_TYPE

#define RESTORE(node)        TreeNode_restore(node, file, nesting)
#define RESTORE_NESTED(node) TreeNode_restore(node, file, nesting + 1)
#define PRINT(...) fprintf(file, __VA_ARGS__)

#define NODE_TYPE(name, compile_code, restoration_code) case N_TYPE_##name: { {restoration_code;} break; }
#define EXPR_OPERATOR(name, action, simplification, derivative, compilation_code, restoration_code) \
    case OP_##name: { {restoration_code;} break; }

void TreeNode_restore(const TreeNode* node, FILE* const file, int nesting) {
    if (!node) return;

    if (node->type == N_TYPE_OP) {
        switch(node->value.op) {
            #include "operators.hpp"
            default: break;
        }
        return;
    }
    switch(node->type) {
        #include "node_types.hpp"
        default: break;
    }
}

#undef EXPR_OPERATOR
#undef NODE_TYPE

#undef LEFT
#undef RIGHT

//* This is to randomize node color so debug tree would look like a Christmas tree!
static inline const char* node_color() {
    int key = rand() % 100;
    if (key < 90) return "darkgreen";
    if (key < 93) return "gold";
    if (key < 96) return "blue";
    if (key < 101) return "red";
    return "darkgreen";
}

void recursive_graph_dump(const TreeNode* equation, FILE* file, int* const err_code) {
    _LOG_FAIL_CHECK_(!(TreeNode_get_error(equation) & (~TREE_INV_CONNECTIONS)), 
                     "error", ERROR_REPORTS, return, err_code, EINVAL);
    _LOG_FAIL_CHECK_(file, "error", ERROR_REPORTS, return, err_code, ENOENT);

    if (!equation || !file) return;
    fprintf(file, "\tV%p [style=filled, shape=\"box\" fillcolor=\"%s\" label=\"", equation, node_color());
    fprintf(file, "%s", NODE_NAMES[equation->type]);
    fprintf_value(file, equation);
    fprintf(file, "\"]\n");

    if (equation->left) {
        recursive_graph_dump(equation->left, file);
        fprintf(file, "\tV%p -> V%p [arrowhead=\"none\", penwidth=2.5, color=\"saddlebrown\"]\n", 
                equation, equation->left);
    }
    if (equation->right) {
        recursive_graph_dump(equation->right, file);
        fprintf(file, "\tV%p -> V%p [arrowhead=\"none\", penwidth=2.5, color=\"saddlebrown\"]\n", 
                equation, equation->right);
    }
}

#define eq_value ( equation->value.dbl )
#define as_op(operation) eq_value = get_value(eq_L) operation get_value(eq_R); break

#define LEFT  ( equation->left )
#define RIGHT ( equation->right )

static inline double get_value(TreeNode* equation) { return equation->value.dbl; }

#define EXPR_OPERATOR(name, action, simplification, derivative, compilation_code, restoration_code)     \
    case OP_##name: { equation->value.dbl = ( action ); break; }

static void wrap_constants(TreeNode* equation) {
    if (!equation || !eq_t_op(equation)) return;
    if (!eq_t_const(eq_L) || !eq_t_const(eq_R)) return;

    if ( equation->value.op == OP_DIV && ( is_equal(get_value(eq_R), 0.0) ||
        !is_equal(get_value(eq_L) / get_value(eq_R), round(get_value(eq_L) / get_value(eq_R))) ) ) return;
    
    switch (equation->value.op) {
        #include "operators.hpp"
        default: return;
    }

    equation->type = N_TYPE_CONST;

    TreeNode_dtor(&equation->left);
    TreeNode_dtor(&equation->right);
}

#undef EXPR_OPERATOR

static inline void replace_node(TreeNode* alpha, TreeNode* beta) {    
    if (alpha->right != beta)
        TreeNode_dtor(&alpha->right);
    else if (alpha->left != beta)
        TreeNode_dtor(&alpha->left);
    alpha->type  = beta->type;
    alpha->left  = beta->left;
    alpha->right = beta->right;
    alpha->value = beta->value;
    free(beta);
}

static inline void set_to_const(TreeNode* equation, double value) {
    equation->type = N_TYPE_CONST;
    TreeNode_dtor(&equation->left);
    TreeNode_dtor(&equation->right);
    equation->value.dbl = value;
}

#define KEEP_NODE(node) replace_node(equation, node)
#define KEEP_CONST(value) set_to_const(equation, value)

static inline bool is_constant(const TreeNode* node, const double value) {
    return node->type == N_TYPE_CONST && is_equal(node->value.dbl, value);
}

#define IS_CONST_V(node, value) is_constant(node, value)

#define EXPR_OPERATOR(name, action, simplification, derivative, compilation_code, restoration_code)   \
    case OP_##name: { {simplification;} break; }

static void rm_useless(TreeNode* equation) {
    if (!equation || !eq_t_op(equation)) return;
    if (!eq_t_const(eq_L) && !eq_t_const(eq_R)) return;

    switch (equation->value.op) {
        #include "operators.hpp"
        default: return;
    }
}

#undef EXPR_OPERATOR

#undef KEEP_NODE
#undef KEEP_CONST
