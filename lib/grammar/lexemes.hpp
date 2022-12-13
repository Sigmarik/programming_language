#define SIMPLE_LEX(name, prefix) LEXEME(name, ( begins_with(CUR_PTR, prefix) ), sizeof(prefix) - 1, {})

LEXEME(END, ( false ), 1, {})

SIMPLE_LEX(COMMENT_BGN, "/*")
SIMPLE_LEX(COMMENT_END, "*/")

SIMPLE_LEX(NEXT_STMT, ";")
SIMPLE_LEX(NEXT_ARG, ",")

SIMPLE_LEX(NVAR, "var")
SIMPLE_LEX(NDEF, "def")

SIMPLE_LEX(ADD, "+")
SIMPLE_LEX(SUB, "-")
SIMPLE_LEX(MUL, "*")
SIMPLE_LEX(DIV, "/")

SIMPLE_LEX(AND, "&&")
SIMPLE_LEX(LOR, "||")

SIMPLE_LEX(CMP_G, ">")
SIMPLE_LEX(CMP_L, "<")

SIMPLE_LEX(CMP_EQ, "==")
SIMPLE_LEX(CMP_GEQ, ">=")
SIMPLE_LEX(CMP_LEQ, "<=")

SIMPLE_LEX(ASS, "=")

SIMPLE_LEX(BRACK_OP, "(")
SIMPLE_LEX(BRACK_CL, ")")

SIMPLE_LEX(RETURN, "return")
SIMPLE_LEX(IF, "if")
SIMPLE_LEX(ELSE, "else")
SIMPLE_LEX(WHILE, "while")

SIMPLE_LEX(DIFF_BLOCK_OP, "[")
SIMPLE_LEX(DIFF_BLOCK_CL, "]")

SIMPLE_LEX(BLOCK_BRACK_OP, "{")
SIMPLE_LEX(BLOCK_BRACK_CL, "}")

//* LEX_NUM is defined separately!!!

LEXEME(NAME, ( isalpha(*CUR_PTR) || *CUR_PTR == '_' ), identify_name(CUR_PTR).length, 
    { .name = identify_name(CUR_PTR) })

#undef SIMPLE_LEX