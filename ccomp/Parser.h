#ifndef PARSER_H
#define PARSER_H

#include "llvm/IR/Value.h"
#include "llvm/ADT/APInt.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "Token.h"
#include "Nodes.h"

class Parser {
private:
    unsigned int Current;
    std::vector<Token> Tokens;
    std::vector<std::string> Types;
public:
    Parser(std::vector<Token> Tokens);

    PNode *Parse();

private:
    bool Check(TType Type);

    Token Advance();

    Token Consume(TType Type, const char *ErrorMsg);

    bool End();

    Token Peek(int Offset = 0);

    Token Previous();

    PNode *LocateNode(PNode *Node, Token Token);

    PNode *ParseBlock();

    PNode *ParseStatement();

    PNode *ParseIfStatement();

    PNode *ParseForStatement();

    PNode *ParseReturnStatement();

    PNode *ParseExpression();

    PNode *ParseAssignment();

    PNode *ParseOr();

    PNode *ParseAnd();

    PNode *ParseEquality();

    PNode *ParseComparison();

    PNode *ParseTerm();

    PNode *ParseFactor();

    PNode *ParseUnary();

    PNode *ParseCall();

    PNode *ParsePrimary();

    PNode *ParseStruct();

    PNode *ParseTypedef();

    PNode *ParseAlloc(std::string type);

    PNode *ParsePrototype(AllocNode *ReturnAlloc, std::string Name, Token ProtToken);

    bool IsSemicolonRequired(PNode *Expr) const;

    bool IsTypeDeclared(const char *Text);

    void DefineType(const AllocNode *Alloc);
};

#endif