INSTRUCTION(PUSH_R, {
    PUSH(0x50 + REG_A);
})

INSTRUCTION(PUSH_32, {
    PUSH(0x68);
    PUSH_32_BIG_ENDIAN(T_ARG.constant);
})

INSTRUCTION(PUSH_64, {
    PUSH(0x68);
    PUSH_32_BIG_ENDIAN(T_ARG.constant >> 32);
    PUSH(0x68);
    PUSH_32_BIG_ENDIAN(T_ARG.constant);
})

INSTRUCTION(PUSH_CM, {
    PUSH(0xff);
    PUSH(0x34);
    PUSH(0x25);
    PUSH_32_BIG_ENDIAN(T_ARG.constant);
})

INSTRUCTION(PUSH_SM, {
    PUSH(0xff);
    PUSH(0xb0 + REG_A);
})
