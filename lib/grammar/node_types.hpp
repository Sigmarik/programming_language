NODE_TYPE(DEF_SEQ, {
    COMPILE(LEFT);
    COMPILE(RIGHT);
})

NODE_TYPE(SEQ, {
    COMPILE(LEFT);
    COMPILE(RIGHT);
})
NODE_TYPE(RETURN, {
    COMPILE(LEFT);
    COMPILE(RIGHT);
    PUT("MOVE RCX  # Return RBX to its original state.\n");
    PUT("MOVE RAX\n");
    PUT("MOVE RBX\n");
    PUT("PUSH RCX\n");
    PUT("RET\n");
})

NODE_TYPE(NFUN, {
    if (begins_with(node->value.name.pointer, "main")) {
        PUT("CALL %.*s__function__body__\n", (int)node->value.name.length, node->value.name.pointer);
    }
    PUT("JMP %.*s__def__end__\n", (int)node->value.name.length, node->value.name.pointer);
    PUT("HERE %.*s__function__body__\n", (int)node->value.name.length, node->value.name.pointer);
    START_LOCAL_NAMESPACE();
    PUT("\tPUSH RAX  # Reserve value of the RAX for restoration\n");
    PUT("\tMOVE RCX\n\n");
    PUT("\tPUSH RBX  # Reserve value of the RBX for restoration\n");
    PUT("\tMOVE RDX\n\n");
    PUT("\tPUSH RAX  # Update RBX value for called function use.\n");
    PUT("\tMOVE RBX\n\n");
    COMPILE_NESTED(LEFT);
    PUT("\tPUSH RDX  # Put RBX value (stored in RDX) back.\n");
    PUT("\tPUSH RCX  # Put RAX value (stored in RCX) back.\n");
    COMPILE_NESTED(RIGHT);
    END_NAMESPACE();
    PUT("HERE %.*s__def__end__\n\n", (int)node->value.name.length, node->value.name.pointer);
})
NODE_TYPE(NVAR, {
    PUT("# Declaration of the \"%.*s\" variable.\n", (int)node->value.name.length, node->value.name.pointer);
    COMPILE(RIGHT);
    ADD_NAME(node->value.name);
    VariableAddress address = GET_VARIABLE(node->value.name);
    if (address.global)
        PUT("MOVE [%d]  # Value of %.*s.\n\n", address.address, 
            (int)node->value.name.length, node->value.name.pointer);
    else
        PUT("MOVE [RBX + %d]  # Value of %.*s.\n\n", address.address, 
            (int)node->value.name.length, node->value.name.pointer);
})

NODE_TYPE(BLOCK, {
    START_GENERAL_NAMESPACE();
    COMPILE_NESTED(LEFT);
    COMPILE_NESTED(RIGHT);
    END_NAMESPACE();
})

NODE_TYPE(OP, {
    //* Operator compilation is defined in compilation function itself (file bin_tree.cpp)
})
NODE_TYPE(ASS, {
    COMPILE(LEFT);
    COMPILE(RIGHT);
    VariableAddress address = GET_VARIABLE(node->value.name);
    if (address.global)
        PUT("MOVE [%d]  # Value of %.*s.\n\n", address.address, 
            (int)node->value.name.length, node->value.name.pointer);
    else
        PUT("MOVE [RBX + %d]  # Value of %.*s.\n\n", address.address, 
            (int)node->value.name.length, node->value.name.pointer);
})
NODE_TYPE(VAR, {
    VariableAddress address = GET_VARIABLE(node->value.name);
    if (address.global)
        PUT("PUSH [%d]  # Value of %.*s.\n", address.address, 
            (int)node->value.name.length, node->value.name.pointer);
    else
        PUT("PUSH [RBX + %d]  # Value of %.*s.\n", address.address, 
            (int)node->value.name.length, node->value.name.pointer);
})
NODE_TYPE(CONST, {
    PUT("PUSH %d\n", (int)(node->value.dbl * 1000));
})

NODE_TYPE(IF, {
    COMPILE(LEFT);
    COMPILE(RIGHT);
})
NODE_TYPE(BRANCH, {
    int label_id_copy = LABEL_ID;
    ++LABEL_ID;
    PUT("PUSH 0\n");
    PUT("JMPE _IF_label_%d\n", label_id_copy);
    COMPILE(LEFT);
    PUT("JMP _IF_label_%d_end_\n", label_id_copy);
    PUT("HERE _IF_label_%d\n", label_id_copy);
    COMPILE(RIGHT);
    PUT("HERE _IF_label_%d_end_\n\n", label_id_copy);
})
NODE_TYPE(WHILE, {
    int label_id_copy = LABEL_ID;
    ++LABEL_ID;
    PUT("HERE _WHILE_label_%d\n", label_id_copy);
    COMPILE(LEFT);
    PUT("PUSH 0\n");
    PUT("JMPE _WHILE_label_%d_end_\n", label_id_copy);
    COMPILE(RIGHT);
    PUT("JMP _WHILE_label_%d\n", label_id_copy);
    PUT("HERE _WHILE_label_%d_end_\n\n", label_id_copy);
})

NODE_TYPE(PARAM, {
    COMPILE(LEFT);
    COMPILE(RIGHT);

    ADD_NAME(node->value.name);

    VariableAddress address = GET_VARIABLE(node->value.name);
    PUT("MOVE [RBX + %d]  # Value of parameter %.*s.\n\n", address.address, 
        (int)node->value.name.length, node->value.name.pointer);
})
NODE_TYPE(CALL, {
    COMPILE(LEFT);
    COMPILE(RIGHT);

    PUT("CALL %.*s__function__body__\n", (int)node->value.name.length, node->value.name.pointer);
})

NODE_TYPE(ARG, {
    COMPILE(LEFT);
    COMPILE(RIGHT);
})
