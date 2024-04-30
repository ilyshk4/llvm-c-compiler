#include "Lexer.h"

#include <utility>

map<string, TType> LexerKeywords = {
        {"return",  TType::RETURN},
        {"if",      TType::IF},
        {"else",    TType::ELSE},
        {"for",     TType::FOR},
        {"while",   TType::WHILE},
        {"struct",  TType::STRUCT},
        {"typedef", TType::TYPEDEF},

};

static char *AllocCharArray(const std::string &Str) {
    size_t length = Str.length();
    char *ptr = new char[length + 1];
    Str.copy(ptr, length);
    ptr[length] = '\0';
    return ptr;
}

Lexer::Lexer(string SrcText) : SrcText(std::move(SrcText)), Error(false) {}

void Lexer::Tokenize() {
    while (!End() && !Error) {
        Start = Current;
        ParseToken();
    }
    Put(TType::END_OF_FILE);
}

bool Lexer::GetError(string &Msg) {
    if (Error)
        Msg = ErrorTextMsg;
    return Error;
}

vector<Token> Lexer::GetTokens() {
    return Tokens;
}

Lexer::~Lexer()
= default;

bool Lexer::End() {
    return Current >= SrcText.length();
}

void Lexer::BeginToken() {
    Column = CurrentCol;
}

void Lexer::Put(TType Type) {
    Tokens.emplace_back(Type, Row, Column);

}

void Lexer::Put(TType Type, TVar var) {
    Tokens.emplace_back(Type, var, Row, Column);
}

void Lexer::ParseToken() {

    BeginToken();
    char c = Advance();
    switch (c) {
        case '(':
            Put(TType::L_PAREN);
            break;
        case ')':
            Put(TType::R_PAREN);
            break;
        case '[':
            Put(TType::L_SCR);
            break;
        case ']':
            Put(TType::R_SCR);
            break;
        case '{':
            Put(TType::L_BRACE);
            break;
        case '}':
            Put(TType::R_BRACE);
            break;
        case ',':
            Put(TType::COMMA);
            break;
        case '.':
            if (Peek(1) == '.' || Peek(2) == '.') {
                Advance();
                Advance();
                Put(TType::VARARG);
            } else
                Put(TType::DOT);
            break;
        case '-':
            Put(TType::MINUS);
            break;
        case '+':
            Put(TType::PLUS);
            break;
        case ';':
            Put(TType::SEMICOLON);
            break;
        case '*':
            Put(TType::STAR);
            break;
        case '/':
            Put(Match('/') ? TType::D_SLASH : TType::SLASH);
            break;
        case '%':
            Put(TType::PERCENT);
            break;
        case '!':
            Put(Match('=') ? TType::BANG_EQ : TType::BANG);
            break;
        case '=':
            Put(Match('=') ? TType::D_EQUAL : TType::EQUAL);
            break;
        case '<':
            Put(Match('=') ? TType::LESS_EQ : TType::LESS);
            break;
        case '>':
            Put(Match('=') ? TType::GREAT_EQ : TType::GREAT);
            break;
        case '&':
            Put(Match('&') ? TType::AND : TType::BIN_AND);
            break;
        case '|':
            Put(Match('|') ? TType::OR : TType::BIN_OR);
            break;
        case ' ':
        case '\r':
        case '\t':
            break;
        case '\n':
            Row++;
            CurrentCol = 0;
            break;
        case '"':
            String();
            break;
        case '\'':
            Char();
            break;
        default:
            if (isdigit(c))
                Number();
            else if (isalpha(c) || c == '_')
                Name();
            else
                ThrowError("unexpected character");
            break;
    }
}

char Lexer::Advance() {
    CurrentCol++;
    return SrcText.at(Current++);
}

bool Lexer::Match(char expected) {
    if (End() || SrcText.at(Current) != expected)
        return false;
    else {
        Current++;
        CurrentCol++;
        return true;
    }
}

char Lexer::Peek(int offset) {
    if (End())
        return '\0';
    else return SrcText.at(Current + offset);
}

void Lexer::String() {
    while (Peek() != '"' && !End()) {
        if (Peek() == '\n') {
            Row++;
            CurrentCol = 0;
        }
        Advance();
    }

    if (End()) {
        ThrowError("string not terminated");
        return;
    }

    Advance();

    string Text = SrcText.substr(Start + 1, Current - Start - 2);
    Put(TType::STRING, AllocCharArray(Text));
}

void Lexer::Char() {
    Advance();
    Advance();
    Put(TType::CHAR, SrcText[Start + 1]);
}

void Lexer::Number() {
    bool IsInteger = true;
    while (isdigit(Peek()) || isalpha(Peek()))
        Advance();

    if (Peek() == '.' && isdigit(Peek(1))) {
        Advance();
        while (isdigit(Peek()))
            Advance();
        IsInteger = false;
    }

    string Text = SrcText.substr(Start, Current - Start);

    if (IsInteger)
        Put(TType::INTEGER, stoi(Text));
    else
        Put(TType::FLOAT, stod(Text));
}

void Lexer::Name() {
    while (isalpha(Peek()) || isdigit(Peek()) || Peek() == '_') {
        Advance();
    }

    string Text = SrcText.substr(Start, Current - Start);

    if (Text == "const")
        return;

    if (LexerKeywords.find(Text) != LexerKeywords.end())
        Put(LexerKeywords[Text]);
    else
        Put(TType::IDENTIFIER, AllocCharArray(Text));
}

void Lexer::ThrowError(const string &Msg) {
    ErrorTextMsg = to_string(Row) + ":" + to_string(Current) + ": " + Msg;
    Error = true;
}