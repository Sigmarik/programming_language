INSTRUCTION(MOV_R2R, {
    PUSH(0x48);
    PUSH(0x89);
    PUSH(0xc0 + REG_A + REG_B * 8);
})

INSTRUCTION(MOV_C2R, {
    PUSH(0x48);
    PUSH(0xb8 + REG_A);
    PUSH_64_BIG_ENDIAN(T_ARG.constant);
})

INSTRUCTION(MOV_CM2R, {
    PUSH(0x48);
    PUSH(0x8b);
    PUSH(0x04 + REG_A * 8);
    PUSH_32_BIG_ENDIAN(T_ARG.constant);
})

INSTRUCTION(MOV_SM2R, {
    PUSH(0x48);
    PUSH(0x8b);
    PUSH(0x80 + REG_A * 8 + REG_B);
})

INSTRUCTION(MOV_R2CM, {
    PUSH(0x48);
    PUSH(0x89);
    PUSH(0x04 + REG_B * 8);
})

INSTRUCTION(MOV_R2SM, {
    PUSH(0x48);
    PUSH(0x89);
    PUSH(0x80 + REG_A + REG_B * 8);
})
