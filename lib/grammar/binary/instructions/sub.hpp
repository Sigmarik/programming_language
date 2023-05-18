INSTRUCTION(SUB_R2R, {
    PUSH(0x48);
    PUSH(0x29);
    PUSH(0xc0 + REG_A + REG_B * 8);
})

INSTRUCTION(SUB_C2R, {
    PUSH(0x48);
    PUSH(0x81);
    PUSH(0xe8 + REG_A);
    PUSH_32_BIG_ENDIAN(T_ARG.constant);
})

INSTRUCTION(SUB_CM2R, {
    PUSH(0x48);
    PUSH(0x2b);
    PUSH(0x04 + REG_A);
    PUSH(0x25);
    PUSH_32_BIG_ENDIAN(T_ARG.constant);
})

INSTRUCTION(SUB_SM2R, {
    PUSH(0x48);
    PUSH(0x2b);
    PUSH(0x80 + REG_A * 8 + REG_B);
    PUSH_32_BIG_ENDIAN(T_ARG.constant);
})

INSTRUCTION(SUB_R2CM, {
    PUSH(0x48);
    PUSH(0x29);
    PUSH(0x04 + REG_A * 8);
    PUSH(0x25);
    PUSH_32_BIG_ENDIAN(T_ARG.constant);
})

INSTRUCTION(SUB_R2SM, {
    PUSH(0x48);
    PUSH(0x29);
    PUSH(0x80 + REG_A + REG_B * 8);
    PUSH_32_BIG_ENDIAN(T_ARG.constant);
})
