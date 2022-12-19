/**
 * @file tree_config.h
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief Configuration file for tree data structure.
 * @version 0.1
 * @date 2022-11-05
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef TREE_CONFIG_H
#define TREE_CONFIG_H

#include <stdlib.h>

static const size_t TREE_PICT_NAME_SIZE = 256;
static const size_t TREE_DRAW_REQUEST_SIZE = 512;

#define TREE_TEMP_DOT_FNAME "temp.dot"
#define TREE_LOG_ASSET_FOLD_NAME "log_assets"
#define TREE_DUMP_TAG "tree_dump"

#define NODE_TYPE(name, compile_code, restoration_code) N_TYPE_##name,

enum NodeType {
    #include "node_types.hpp"
};

#undef NODE_TYPE
#define NODE_TYPE(name, compile_code, restoration_code) #name,

static const char* const NODE_NAMES[] = {
    #include "node_types.hpp"
};

#undef NODE_TYPE

#define EXPR_OPERATOR(name, action, simplification, derivative, compilation_code, restoration_code) OP_##name,

enum Operator {
    #include "operators.hpp"
};

#undef EXPR_OPERATOR
#define EXPR_OPERATOR(name, action, simplification, derivative, compilation_code, restoration_code) #name,

static const char* const OP_NAMES[] = {
    #include "operators.hpp"
};

#undef EXPR_OPERATOR

static const bool CHRISTMAS_MODE = true;

#endif