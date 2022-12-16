EXPR_OPERATOR(ADD, LEFT->value.dbl + RIGHT->value.dbl, {
    if (IS_CONST_V(LEFT, 0.0)) KEEP_NODE(RIGHT);
    if (IS_CONST_V(RIGHT, 0.0)) KEEP_NODE(LEFT);
}, new_op(OP_ADD, DELTA_L, DELTA_R), {
    COMPILE(LEFT);
    COMPILE(RIGHT);
    PUT("ADD\n");
})

EXPR_OPERATOR(SUB, LEFT->value.dbl - RIGHT->value.dbl, {
    if (IS_CONST_V(RIGHT, 0.0)) KEEP_NODE(LEFT);
}, new_op(OP_SUB, DELTA_L, DELTA_R), {
    COMPILE(RIGHT);
    COMPILE(LEFT);
    PUT("SUB\n");
})

EXPR_OPERATOR(MUL, LEFT->value.dbl * RIGHT->value.dbl, {
    if (IS_CONST_V(LEFT, 1.0)) KEEP_NODE(RIGHT);
    if (IS_CONST_V(RIGHT, 1.0)) KEEP_NODE(LEFT);

    if (IS_CONST_V(LEFT, 0.0)) KEEP_CONST(0.0);
    if (IS_CONST_V(RIGHT, 0.0)) KEEP_CONST(0.0);
}, new_op(OP_ADD, new_op(OP_MUL, DELTA_L, COPY_R), new_op(OP_MUL, COPY_L, DELTA_R)), {
    PUT("PUSH 1000\n");
    COMPILE(LEFT);
    COMPILE(RIGHT);
    PUT("MUL\n");
    PUT("DIV\n");
})

EXPR_OPERATOR(DIV, LEFT->value.dbl / RIGHT->value.dbl, {
    if (IS_CONST_V(RIGHT, 1.0)) KEEP_NODE(LEFT);
    if (IS_CONST_V(LEFT, 0.0)) KEEP_CONST(0.0);
}, new_op(OP_DIV, new_op(OP_SUB, new_op(OP_MUL, DELTA_L, COPY_R), new_op(OP_MUL, COPY_L, DELTA_R)), 
          new_op(OP_MUL, COPY_R, COPY_R)), {
    COMPILE(RIGHT);
    COMPILE(LEFT);
    PUT("PUSH 1000\n");
    PUT("MUL\n");
    PUT("DIV\n");
})

EXPR_OPERATOR(CMP_G, LEFT->value.dbl > RIGHT->value.dbl, {}, NO_DERIVATIVE, {
    COMPILE(RIGHT);
    COMPILE(LEFT);
    PUT("JMPG _cmp_G_label_%d\n", LABEL_ID);
    PUT("PUSH 0\n");
    PUT("JMP _cmp_G_label_%d_end_\n", LABEL_ID);
    PUT("HERE _cmp_G_label_%d\n", LABEL_ID);
    PUT("PUSH 1000\n");
    PUT("HERE _cmp_G_label_%d_end_\n", LABEL_ID);
    ++LABEL_ID;
})
EXPR_OPERATOR(CMP_L, LEFT->value.dbl < RIGHT->value.dbl, {}, NO_DERIVATIVE, {
    COMPILE(RIGHT);
    COMPILE(LEFT);
    PUT("JMPL _cmp_L_label_%d\n", LABEL_ID);
    PUT("PUSH 0\n");
    PUT("JMP _cmp_L_label_%d_end_\n", LABEL_ID);
    PUT("HERE _cmp_L_label_%d\n", LABEL_ID);
    PUT("PUSH 1000\n");
    PUT("HERE _cmp_L_label_%d_end_\n", LABEL_ID);
    ++LABEL_ID;
})

EXPR_OPERATOR(CMP_GE, LEFT->value.dbl > RIGHT->value.dbl - 0.0005, {}, NO_DERIVATIVE, {
    COMPILE(RIGHT);
    COMPILE(LEFT);
    PUT("JMPGE _cmp_GE_label_%d\n", LABEL_ID);
    PUT("PUSH 0\n");
    PUT("JMP _cmp_GE_label_%d_end_\n", LABEL_ID);
    PUT("HERE _cmp_GE_label_%d\n", LABEL_ID);
    PUT("PUSH 1000\n");
    PUT("HERE _cmp_GE_label_%d_end_\n", LABEL_ID);
    ++LABEL_ID;
})
EXPR_OPERATOR(CMP_LE, LEFT->value.dbl < RIGHT->value.dbl + 0.0005, {}, NO_DERIVATIVE, {
    COMPILE(RIGHT);
    COMPILE(LEFT);
    PUT("JMPLE _cmp_LE_label_%d\n", LABEL_ID);
    PUT("PUSH 0\n");
    PUT("JMP _cmp_LE_label_%d_end_\n", LABEL_ID);
    PUT("HERE _cmp_LE_label_%d\n", LABEL_ID);
    PUT("PUSH 1000\n");
    PUT("HERE _cmp_LE_label_%d_end_\n", LABEL_ID);
    ++LABEL_ID;
})

EXPR_OPERATOR(CMP_EQUAL, is_equal(LEFT->value.dbl, RIGHT->value.dbl), {}, NO_DERIVATIVE, {
    COMPILE(RIGHT);
    COMPILE(LEFT);
    PUT("JMPE _cmp_E_label_%d\n", LABEL_ID);
    PUT("PUSH 0\n");
    PUT("JMP _cmp_E_label_%d_end_\n", LABEL_ID);
    PUT("HERE _cmp_E_label_%d\n", LABEL_ID);
    PUT("PUSH 1000\n");
    PUT("HERE _cmp_E_label_%d_end_\n", LABEL_ID);
    ++LABEL_ID;
})

EXPR_OPERATOR(L_AND, !is_equal(LEFT->value.dbl, 0.0) && !is_equal(RIGHT->value.dbl, 0.0), {}, NO_DERIVATIVE, {
    COMPILE(RIGHT);
    COMPILE(LEFT);
    PUT("MUL\n");
    PUT("PUSH 0\n");
    PUT("JMPE _log_AND_label_%d\n", LABEL_ID);
    PUT("PUSH 1000\n");
    PUT("JMP _log_AND_label_%d_end_\n", LABEL_ID);
    PUT("HERE _log_AND_label_%d\n", LABEL_ID);
    PUT("PUSH 0\n");
    PUT("HERE _log_AND_label_%d_end_\n", LABEL_ID);
    ++LABEL_ID;
})
EXPR_OPERATOR(L_OR, !is_equal(LEFT->value.dbl, 0.0) || !is_equal(RIGHT->value.dbl, 0.0), {}, NO_DERIVATIVE, {
    int label_id_copy = LABEL_ID;
    ++LABEL_ID;

    COMPILE(RIGHT);
    PUT("PUSH 0\n");
    PUT("JMPE _log_POR_R_label_%d\n", label_id_copy);
    PUT("PUSH 1000\n");
    PUT("JMP _log_POR_R_label_%d_end_\n", label_id_copy);
    PUT("HERE _log_POR_R_label_%d\n", label_id_copy);
    PUT("PUSH 0\n");
    PUT("HERE _log_POR_R_label_%d_end_\n", label_id_copy);

    COMPILE(LEFT);
    PUT("PUSH 0\n");
    PUT("JMPE _log_POR_L_label_%d\n", label_id_copy);
    PUT("PUSH 1000\n");
    PUT("JMP _log_POR_L_label_%d_end_\n", label_id_copy);
    PUT("HERE _log_POR_L_label_%d\n", label_id_copy);
    PUT("PUSH 0\n");
    PUT("HERE _log_POR_L_label_%d_end_\n", label_id_copy);

    PUT("ADD\n");
    PUT("PUSH 0\n");
    PUT("JMPL _log_OR_label_%d\n", label_id_copy);
    PUT("PUSH 1000\n");
    PUT("JMP _log_OR_label_%d_end_\n", label_id_copy);
    PUT("HERE _log_OR_label_%d\n", label_id_copy);
    PUT("PUSH 0\n");
    PUT("HERE _log_OR_label_%d_end_\n", label_id_copy);
})

EXPR_OPERATOR(ASS, LEFT->value.dbl, {}, NO_DERIVATIVE, {
    COMPILE(LEFT);
    COMPILE(RIGHT);
    VariableAddress address = GET_VARIABLE(node->value.name);
    if (address.global)
        PUT("MOVE [%d]\n", address.address);
    else
        PUT("MOVE [RBX + %d]\n", address.address);
})