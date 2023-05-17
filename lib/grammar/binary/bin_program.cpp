#include "bin_program.h"

#include <errno.h>
#include <string.h>

#include "lib/generic_array.hpp"

MAKE_ARRAY_DEFINITION(InstructionArray, Instruction)
MAKE_ARRAY_DEFINITION(BinaryArray, unsigned char)

#define CURRENT (*instr_ptr)
#define ARGUMENT (CURRENT.argument)
#define T_ARG (CURRENT.argument.trio)
#define REG_A (T_ARG.reg_a)
#define REG_B (T_ARG.reg_b)
#define PUSH(value) BinaryArray_push(binary, (unsigned char) (value))

#define PUSH_32_BIG_ENDIAN(number) do {                                                 \
    for (unsigned char byte_id = 0; byte_id < 4 * 8; byte_id = (u_char)(byte_id + 8)) { \
        PUSH(((number & ((u_int64_t)UINT8_MAX << byte_id)) >> byte_id));                \
    }                                                                                   \
} while (0)

#define PUSH_64_BIG_ENDIAN(number) do {                                                 \
    for (unsigned char byte_id = 0; byte_id < 8 * 8; byte_id = (u_char)(byte_id + 8)) { \
        PUSH(((number & ((u_int64_t)UINT8_MAX << byte_id)) >> byte_id));                \
    }                                                                                   \
} while (0)

#define INSTRUCTION(type, compilation_code) \
case INSTR_##type: {                        \
    compilation_code;                       \
    break;                                  \
}

void compile(InstructionArray* instructions, BinaryArray* binary) {
    for (Instruction* instr_ptr = InstructionArray_begin(instructions);
        instr_ptr < InstructionArray_end(instructions); ++instr_ptr) {

        instr_ptr->position = binary->size;

        switch (instr_ptr->code) {
            #include "instructions.hpp"
            default: { errno = EINVAL; return; }
        }
    }
}

#undef INSTRUCTION
#undef PUSH_64_BIG_ENDIAN
#undef PUSH_32_BIG_ENDIAN
#undef CURRENT
#undef ARGUMENT
#undef T_ARG
#undef ARG_A
#undef ARG_B
#undef PUSH
