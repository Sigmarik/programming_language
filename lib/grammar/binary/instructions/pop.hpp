INSTRUCTION(POP_R, {
    PUSH(0x58 + REG_A);
})

INSTRUCTION(POP_CM, {
    PUSH(0x8f);
    PUSH(0x04);
    PUSH(0x25);
})

INSTRUCTION(POP_SM, {
    PUSH(0x8f);
    PUSH(0x80 + REG_A);
    PUSH_32_BIG_ENDIAN(T_ARG.constant);
})
