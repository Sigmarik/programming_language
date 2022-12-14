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

#define LEXEME(name, condition, shift_formula, value_formula) LEX_##name,

enum LexType {
    LEX_NUM,
    #include "lexemes.hpp"
};

#undef LEXEME
#define LEXEME(name, condition, shift_formula, value_formula) "LEX_" #name,

static const char* LEXEME_NAMES[] = {
    "LEX_NUM",
    #include "lexemes.hpp"
};

#undef LEXEME

struct CharAddress {
    unsigned int line = 0;
    unsigned int index = 0;
};

union LexValue {
    double dbl;
    LimitedString name;
};

struct Lex {
    LexType type = LEX_END;
    LexValue value = { .name = {} };
    CharAddress address = {};
};

struct LexStack {
    Lex* buffer = NULL;
    size_t size = 0;
    size_t capacity = 0;
};

void LexStack_ctor(LexStack* stack, size_t start_size);
void LexStack_dtor(LexStack* stack);

void LexStack_push(LexStack* stack, Lex lexeme);

/**
 * @brief Turn source text of the program into a stack of lexemes.
 * 
 * @param line 
 * @return 
 */
LexStack lexify(const char* line);

#define PARSING_FUNCTION(name) TreeNode* parse_##name(const LexStack stack, int* caret);

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