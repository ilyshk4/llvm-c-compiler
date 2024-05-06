#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <vector>
#include <iostream>
#include <map>

#include "TVar.h"

enum class TType {
    END_OF_FILE,
    L_PAREN, R_PAREN, L_BRACE, R_BRACE, L_SCR, R_SCR, COMMA, DOT, VARARG,
    PLUS, MINUS, STAR, SLASH, D_SLASH, PERCENT,
    BANG, BANG_EQ, EQUAL, D_EQUAL, LESS, LESS_EQ, GREAT, GREAT_EQ, OR, AND, BIN_OR, BIN_AND,
    SEMICOLON,
    RETURN, IF, ELSE, FOR, WHILE, STRUCT, TYPEDEF,
    IDENTIFIER, STRING, INTEGER, FLOAT, CHAR,
};

static std::string TokenNames[]{
        "eof",
        "(", ")", "{", "}", "[", "]", ",", ".", "...",
        "+", "-", "*", "/", "//", "%",
        "!", "!=", "=", "==", "<", "<=", ">", ">=", "||", "&&", "|", "&",
        ";",
        "return", "if", "else", "for", "while", "struct", "typedef",
        "id", "str", "int", "float", "char"
};

struct Token {
public:
    size_t Row, Column;
    TType Type;
    TVar Var;

    Token(TType Type, size_t Row, size_t Column);

    Token(TType Type, TVar Var, size_t Row, size_t Column);

    static std::string GetName(TType Type);
};

#endif