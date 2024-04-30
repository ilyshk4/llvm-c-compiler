#include "Token.h"

Token::Token(TType Type, size_t Row, size_t Column) : Type(Type), Var(), Row(Row), Column(Column) {}

Token::Token(TType Type, TVar Var, size_t Row, size_t Column) : Type(Type), Var(Var), Row(Row), Column(Column) {}

std::string Token::GetName(TType Type) {
    return TokenNames[static_cast<int>(Type)];
}
