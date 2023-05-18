INSTRUCTION(JMP, {
    PUSH(0xe9);  //*                                        Instruction size --v
    u_int32_t shift = ARGUMENT.jump.destination->position - CURRENT.position + 5;
    PUSH_32_BIG_ENDIAN(shift);
})

INSTRUCTION(JE, {
    PUSH(0x0f);
    PUSH(0x84);
    u_int32_t shift = ARGUMENT.jump.destination->position - CURRENT.position + 6;
    PUSH_32_BIG_ENDIAN(shift);
})

INSTRUCTION(JNE, {
    PUSH(0x0f);
    PUSH(0x85);
    u_int32_t shift = ARGUMENT.jump.destination->position - CURRENT.position + 6;
    PUSH_32_BIG_ENDIAN(shift);
})

INSTRUCTION(JG, {
    PUSH(0x0f);
    PUSH(0x8f);
    u_int32_t shift = ARGUMENT.jump.destination->position - CURRENT.position + 6;
    PUSH_32_BIG_ENDIAN(shift);
})

INSTRUCTION(JL, {
    PUSH(0x0f);
    PUSH(0x8c);
    u_int32_t shift = ARGUMENT.jump.destination->position - CURRENT.position + 6;
    PUSH_32_BIG_ENDIAN(shift);
})

INSTRUCTION(JGE, {
    PUSH(0x0f);
    PUSH(0x8d);
    u_int32_t shift = ARGUMENT.jump.destination->position - CURRENT.position + 6;
    PUSH_32_BIG_ENDIAN(shift);
})

INSTRUCTION(JLE, {
    PUSH(0x0f);
    PUSH(0x8e);
    u_int32_t shift = ARGUMENT.jump.destination->position - CURRENT.position + 6;
    PUSH_32_BIG_ENDIAN(shift);
})

INSTRUCTION(CALL, {
    PUSH(0xe8);
    u_int32_t shift = ARGUMENT.jump.destination->position - CURRENT.position + 5;
    PUSH_32_BIG_ENDIAN(shift);
})
