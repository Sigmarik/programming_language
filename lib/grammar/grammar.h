/**
 * @file grammar.h
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief Text parsing functions.
 * @version 0.1
 * @date 2022-11-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef GRAMMAR_H
#define GRAMMAR_H

#include "bin_tree.h"

#define TOKEN(name, condition, shift_formula, value_formula) TOK_##name,

enum TokType {
    TOK_NUM,
    #include "tokens.hpp"
};

#undef TOKEN
#define TOKEN(name, condition, shift_formula, value_formula) "TOK_" #name,

static const char* TOKEN_NAMES[] = {
    "TOK_NUM",
    #include "tokens.hpp"
};

#undef TOKEN

struct CharAddress {
    unsigned int line = 0;
    unsigned int index = 0;
};

union TokValue {
    double dbl;
    LimitedString name;
};

struct Token {
    TokType type = TOK_END;
    TokValue value = { .name = {} };
    CharAddress address = {};
};

struct TokStack {
    Token* buffer = NULL;
    size_t size = 0;
    size_t capacity = 0;
};

void TokStack_ctor(TokStack* stack, size_t start_size);
void TokStack_dtor(TokStack* stack);

void TokStack_push(TokStack* stack, Token token);

/**
 * @brief Turn source text of the program into a stack of tokens.
 * 
 * @param line 
 * @return 
 */
TokStack tokenize(const char* line);

#define PARSING_FUNCTION(name) TreeNode* parse_##name(const TokStack stack, int* caret);

PARSING_FUNCTION(program)

PARSING_FUNCTION(def_list)

PARSING_FUNCTION(definition)

PARSING_FUNCTION(new_var)

PARSING_FUNCTION(expression)
PARSING_FUNCTION(expr_logic)
PARSING_FUNCTION(expr_comparison)
PARSING_FUNCTION(expr_add)
PARSING_FUNCTION(expr_mult)
PARSING_FUNCTION(expr_brackets)
PARSING_FUNCTION(expr_derivative)
PARSING_FUNCTION(expr_call)
PARSING_FUNCTION(arg_list)
PARSING_FUNCTION(expr_elementary)

PARSING_FUNCTION(new_func)
PARSING_FUNCTION(param_list)

PARSING_FUNCTION(statement)

PARSING_FUNCTION(block)
PARSING_FUNCTION(command_sequence)

PARSING_FUNCTION(assignment)

PARSING_FUNCTION(if_cond)

PARSING_FUNCTION(while_cond)

PARSING_FUNCTION(program_tree)

#undef PARSING_FUNCTION

#endif