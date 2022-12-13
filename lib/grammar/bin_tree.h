/**
 * @file bin_tree.h
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief Binary tree data structure.
 * @version 0.1
 * @date 2022-11-01
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef BIN_TREE_H
#define BIN_TREE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>

#include "tree_config.h"
#include "bin_tree_reports.h"
#include "lib/file_helper.h"

union NodeValue {
    LimitedString name;
    Operator op;
    double dbl;
};

struct TreeNode {
    NodeType type = {};
    NodeValue value = { .op = {} };

    TreeNode* left = NULL;
    TreeNode* right = NULL;
};

TreeNode* TreeNode_new(NodeType type, NodeValue value, TreeNode* left, TreeNode* right, int* const err_code = &errno);
void TreeNode_dtor(TreeNode** node);

/**
 * @brief Dump the tree into logs.
 * 
 * @param list
 * @param importance message importance
 */
#define TreeNode_dump(tree, importance) do {                            \
    log_printf(importance, TREE_DUMP_TAG, "Called tree dumping.\n");    \
    _TreeNode_dump_graph(tree, importance);                             \
} while (0)

/**
 * @brief Put a picture of the tree into logs.
 * 
 * @param tree 
 * @param importance 
 */
void _TreeNode_dump_graph(const TreeNode* equation, unsigned int importance);

/**
 * @brief Get status of the node.
 * 
 * @param tree 
 * @return (BinaryTree_status_t) binary tree status (0 = OK)
 */
BinaryTree_status_t TreeNode_get_error(const TreeNode* equation);

/**
 * @brief Make a copy of the equation and return pointer to it.
 * 
 * @param equation
 * @return pointer to the copy of the equation
 */
TreeNode* TreeNode_copy(const TreeNode* equation);

/**
 * @brief Differentiate the equation.
 * 
 * @param equation
 * @param var_id ID of the variable to differentiate from
 * @return pointer to the differentiated equation
 */
TreeNode* TreeNode_diff(const TreeNode* equation, const LimitedString var_name, int* const err_code = &errno);

/**
 * @brief Simplify the equation (collapse constants, remove trivial operations).
 * 
 * @param equation 
 */
void TreeNode_simplify(TreeNode* equation, int* const err_code = &errno);

#endif
