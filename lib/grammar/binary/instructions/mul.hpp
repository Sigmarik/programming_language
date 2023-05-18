INSTRUCTION(MUL_R, {
    PUSH(0x48);
    PUSH(0xf7);
    PUSH(0xe0 + REG_A);
})

INSTRUCTION(MUL_CM, {
    PUSH(0x48);
    PUSH(0xf7);
    PUSH(0x24);
    PUSH(0x25);
    PUSH_32_BIG_ENDIAN(T_ARG.constant);
})

INSTRUCTION(MUL_SM, {
    PUSH(0x48);
    PUSH(0xf7);
    PUSH(0xa0 + REG_A);
    PUSH_32_BIG_ENDIAN(T_ARG.constant);
})
