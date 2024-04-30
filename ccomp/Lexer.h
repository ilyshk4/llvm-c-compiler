#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <map>
#include <iostream>

#include "Token.h"

using namespace std;

class Lexer {
private:
	vector<Token> Tokens;
	string SrcText;
	string ErrorTextMsg;
	bool Error;
	size_t Start = 0;
	size_t Current = 0;
	size_t Row = 0;
    size_t Column = 0;
    size_t CurrentCol = 0;
public:
	Lexer(string SrcText);
	~Lexer();
	void Tokenize();
	vector<Token> GetTokens();
	bool GetError(string& Msg);
private:
	bool End();
	void BeginToken();
	void Put(TType Type);
	void Put(TType Type, TVar var);
	void ParseToken();
	char Advance();
	bool Match(char expected);
	char Peek(int offset = 0);
	void String();
	void Char();
	void Number();
	void Name();
	void ThrowError(const string& Msg);
};

#endif