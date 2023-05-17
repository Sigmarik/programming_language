/**
 * @file bin_program.h
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief Binary executable file utility module.
 * @version 0.1
 * @date 2023-05-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef BINARY_PROGRAM_H
#define BINARY_PROGRAM_H

#include "stdlib.h"
#include "inttypes.h"

#include "lib/generic_array.hpp"

#define INSTRUCTION(type, compilation_code) INSTR_##type,

enum INSTRUCTIONS {
    #include "instructions.hpp"
};

#undef INSTRUCTION
#define INSTRUCTION(type, compilation_code) #type,

const char* const INSTRUCTION_NAMES[] = {
    #include "instructions.hpp"
};

#undef INSTRUCTION

struct Instruction;

struct JumpArgument {
    struct Instruction* destination = NULL;
};

struct TripleArgument {
    unsigned char reg_a = 0;
    unsigned char reg_b = 0;
    u_int64_t constant  = 0;
};

union InstructionArgument {
    JumpArgument jump;
    TripleArgument trio;
};

struct Instruction {
    InstructionArgument argument;
    unsigned long position = UINT64_MAX;
    unsigned char code = INSTR_NONE;
};

MAKE_ARRAY_DECLARATION(InstructionArray, Instruction)
MAKE_ARRAY_DECLARATION(BinaryArray, unsigned char)

void compile(InstructionArray* instructions, BinaryArray* binary);

#endif
