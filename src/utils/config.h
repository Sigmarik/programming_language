/**
 * @file config.h
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief List of constants used inside the main program.
 * @version 0.1
 * @date 2022-11-01
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef MAIN_CONFIG_H
#define MAIN_CONFIG_H

#include <stdlib.h>

static const int NUMBER_OF_OWLS = 10;

static const size_t MAX_FORMULA_LENGTH = 65536;

static const size_t MAX_NAME_LENGTH = 1024;
static const char DEFAULT_IN_FILE_NAME[] = "simple.mech";
static const char DEFAULT_TREE_FILE_NAME[] = "out.tree";
static const char DEFAULT_BINARY_FILE_NAME[] = "bin.instr";
#endif