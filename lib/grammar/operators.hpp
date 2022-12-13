EXPR_OPERATOR(ADD, LEFT->value.dbl + RIGHT->value.dbl, {
    if (IS_CONST_V(LEFT, 0.0)) KEEP_NODE(RIGHT);
    if (IS_CONST_V(RIGHT, 0.0)) KEEP_NODE(LEFT);
}, new_op(OP_ADD, DELTA_L, DELTA_R))

EXPR_OPERATOR(SUB, LEFT->value.dbl - RIGHT->value.dbl, {
    if (IS_CONST_V(RIGHT, 0.0)) KEEP_NODE(LEFT);
}, new_op(OP_SUB, DELTA_L, DELTA_R))

EXPR_OPERATOR(MUL, LEFT->value.dbl * RIGHT->value.dbl, {
    if (IS_CONST_V(LEFT, 1.0)) KEEP_NODE(RIGHT);
    if (IS_CONST_V(RIGHT, 1.0)) KEEP_NODE(LEFT);

    if (IS_CONST_V(LEFT, 0.0)) KEEP_CONST(0.0);
    if (IS_CONST_V(RIGHT, 0.0)) KEEP_CONST(0.0);
}, new_op(OP_ADD, new_op(OP_MUL, DELTA_L, COPY_R), new_op(OP_MUL, COPY_L, DELTA_R)))

EXPR_OPERATOR(DIV, LEFT->value.dbl / RIGHT->value.dbl, {
    if (IS_CONST_V(RIGHT, 1.0)) KEEP_NODE(LEFT);
    if (IS_CONST_V(LEFT, 0.0)) KEEP_CONST(0.0);
}, new_op(OP_DIV, new_op(OP_SUB, new_op(OP_MUL, DELTA_L, COPY_R), new_op(OP_MUL, COPY_L, DELTA_R)), 
          new_op(OP_MUL, COPY_R, COPY_R)))

EXPR_OPERATOR(CMP_G, LEFT->value.dbl > RIGHT->value.dbl, {}, NO_DERIVATIVE)
EXPR_OPERATOR(CMP_L, LEFT->value.dbl < RIGHT->value.dbl, {}, NO_DERIVATIVE)

EXPR_OPERATOR(CMP_GE, LEFT->value.dbl > RIGHT->value.dbl - 0.0005, {}, NO_DERIVATIVE)
EXPR_OPERATOR(CMP_LE, LEFT->value.dbl < RIGHT->value.dbl + 0.0005, {}, NO_DERIVATIVE)

EXPR_OPERATOR(CMP_EQUAL, is_equal(LEFT->value.dbl, RIGHT->value.dbl), {}, NO_DERIVATIVE)

EXPR_OPERATOR(L_AND, !is_equal(LEFT->value.dbl, 0.0) && !is_equal(RIGHT->value.dbl, 0.0), {}, NO_DERIVATIVE)
EXPR_OPERATOR(L_OR, !is_equal(LEFT->value.dbl, 0.0) || !is_equal(RIGHT->value.dbl, 0.0), {}, NO_DERIVATIVE)

EXPR_OPERATOR(ASS, LEFT->value.dbl, {}, NO_DERIVATIVE)