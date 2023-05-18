INSTRUCTION(DIV_R, {
    PUSH(0x48);
    PUSH(0xf7);
    PUSH(0xf0 + REG_A);
})

INSTRUCTION(DIV_CM, {
    PUSH(0x48);
    PUSH(0xf7);
    PUSH(0x34);
    PUSH(0x25);
    PUSH_32_BIG_ENDIAN(T_ARG.constant);
})

INSTRUCTION(DIV_SM, {
    PUSH(0x48);
    PUSH(0xf7);
    PUSH(0xb0 + REG_A);
    PUSH_32_BIG_ENDIAN(T_ARG.constant);
})
