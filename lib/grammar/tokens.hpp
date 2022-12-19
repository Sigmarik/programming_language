#define SIMPLE_TOKEN(name, prefix) TOKEN(name, ( begins_with(CUR_PTR, prefix) ), sizeof(prefix) - 1, {})

TOKEN(END, ( false ), 1, {})

SIMPLE_TOKEN(NEXT_STMT, ";")
SIMPLE_TOKEN(NEXT_ARG, ",")

SIMPLE_TOKEN(NVAR, "var")
SIMPLE_TOKEN(NDEF, "def")

SIMPLE_TOKEN(ADD, "+")
SIMPLE_TOKEN(SUB, "-")
SIMPLE_TOKEN(MUL, "*")
SIMPLE_TOKEN(DIV, "/")

SIMPLE_TOKEN(AND, "&&")
SIMPLE_TOKEN(LOR, "||")

SIMPLE_TOKEN(CMP_EQ, "==")
SIMPLE_TOKEN(GEQ, ">=")
SIMPLE_TOKEN(LEQ, "<=")

SIMPLE_TOKEN(GT, ">")
SIMPLE_TOKEN(LT, "<")

SIMPLE_TOKEN(ASS, "=")

SIMPLE_TOKEN(BRACK_OP, "(")
SIMPLE_TOKEN(BRACK_CL, ")")

SIMPLE_TOKEN(RET, "return")
SIMPLE_TOKEN(IF, "if")
SIMPLE_TOKEN(ELSE, "else")
SIMPLE_TOKEN(WHILE, "while")

SIMPLE_TOKEN(DIFF_BLOCK_OP, "[")
SIMPLE_TOKEN(DIFF_BLOCK_CL, "]")

SIMPLE_TOKEN(BLOCK_BRACK_OP, "{")
SIMPLE_TOKEN(BLOCK_BRACK_CL, "}")

//* TOK_NUM is defined separately!!!

TOKEN(NAME, ( isalpha(*CUR_PTR) || *CUR_PTR == '_' ), identify_name(CUR_PTR).length, 
    { .name = identify_name(CUR_PTR) })

#undef SIMPLE_TOKEN