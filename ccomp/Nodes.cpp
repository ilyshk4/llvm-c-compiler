#include "Nodes.h"

static std::string Indent(int depth) {
    std::string Res;
    for (int i = 0; i < depth; i++)
        Res += " ";
    return Res;
}

PNode::~PNode() {}

std::string PNode::ToString(int Depth) {
    return std::string();
}

IdentifierNode::IdentifierNode(std::string Name, PNode *IndexExpr) : Name(std::move(Name)), IndexExpr(IndexExpr) {

}

std::string IdentifierNode::ToString(int Depth) {
    return Indent(Depth) + "id " + std::string(Name);
}

IntegerNode::IntegerNode(uint64_t Value, size_t NumBits) : Value(Value), NumBits(NumBits) {
}

std::string IntegerNode::ToString(int Depth) {
    return Indent(Depth) + "int " + (trunc(Value) == Value ? std::to_string((int) Value) : std::to_string(Value));
}

FloatNode::FloatNode(double Value) : Value(Value) {
}

std::string FloatNode::ToString(int Depth) {
    return Indent(Depth) + (trunc(Value) == Value ? std::to_string(Value) : std::to_string(Value));
}

StringNode::StringNode(std::string Text) : Text(std::move(Text)) {
}

std::string StringNode::ToString(int Depth) {
    return Indent(Depth) + std::string(Text);
}

BinOpNode::BinOpNode(TType OpType, PNode *LHS, PNode *RHS) : OpType(OpType), LHS(LHS), RHS(RHS) {}

BinOpNode::~BinOpNode() {
    delete LHS;
    delete RHS;
}

std::string BinOpNode::ToString(int Depth) {
    return Indent(Depth) + Token::GetName(OpType) + "\n" + LHS->ToString(Depth + 1) + "\n" + RHS->ToString(Depth + 1);
}


UnOpNode::UnOpNode(TType OpType, PNode *Expr) : OpType(OpType), Expr(Expr) {}

UnOpNode::~UnOpNode() {
    delete Expr;
}


std::string UnOpNode::ToString(int Depth) {
    return Indent(Depth) + Token::GetName(OpType) + "\n" + Expr->ToString(Depth + 1);
}

AssignNode::AssignNode(IdentifierNode *Ident, PNode *Expr) : Alloc(nullptr), Ident(Ident), Expr(Expr) {}

AssignNode::AssignNode(AllocNode *Alloc, PNode *Expr) : Alloc(Alloc), Ident(nullptr), Expr(Expr) {}

AssignNode::~AssignNode() {
    delete Expr;
}


std::string AssignNode::ToString(int Depth) {
    if (Alloc)
        return Indent(Depth) + "assign\n" + Indent(Depth + 1) + Alloc->ToString() + "\n" + Expr->ToString(Depth + 1);
    else
        return Indent(Depth) + "assign " + Ident->Name + "\n" + Expr->ToString(Depth + 1);
}


AllocNode::AllocNode(std::string AllocTypeName, std::string Name, size_t PtrDepth, PNode *ArraySizeExpr)
        : AllocTypeName(std::move(AllocTypeName)), Name(std::move(Name)), PtrDepth(PtrDepth), ArraySizeExpr(ArraySizeExpr) {
}

std::string AllocNode::ToString(int Depth) {
    return Indent(Depth) + "alloc " + AllocTypeName + "(* " + std::to_string(PtrDepth) + ") " + Name;
}

AllocNode::~AllocNode() {

}

StructNode::StructNode(std::string Name, std::vector<AllocNode *> AllocNodes) : Name(std::move(Name)),
                                                                                AllocNodes(std::move(AllocNodes)) {
}

std::string StructNode::ToString(int Depth) {
    auto Res = Indent(Depth) + "struct \n";
    for (auto alloc: AllocNodes)
        Res += Indent(Depth + 1) + alloc->AllocTypeName + " " + alloc->Name + "\n";
    return Res;
}

StructNode::~StructNode() {

}

TypedefNode::TypedefNode(AllocNode *Alloc) : Alloc(Alloc) {

}

std::string TypedefNode::ToString(int Depth) {
    return Indent(Depth) + "typedef " + Alloc->ToString(Depth + 1);
}

TypedefNode::~TypedefNode() {

}

BlockNode::BlockNode() {
}

BlockNode::~BlockNode() {
    for (auto Node: Nodes)
        delete Node;
}

std::string BlockNode::ToString(int Depth) {
    std::string Res = Indent(Depth) + "block";
    for (auto Node: Nodes)
        Res += "\n" + Node->ToString(Depth + 1);
    return Res;
}

IfNode::IfNode(PNode *CondExpr, PNode *BodyExpr, PNode *ElseBrExpr) : CondExpr(CondExpr), BodyExpr(BodyExpr),
                                                                      ElseBrExpr(ElseBrExpr) {}

IfNode::~IfNode() {
    delete CondExpr;
    delete BodyExpr;
    delete ElseBrExpr;
}

std::string IfNode::ToString(int Depth) {
    std::string Res = Indent(Depth) + "if";
    Res += '\n' + CondExpr->ToString(Depth + 1);
    Res += '\n' + BodyExpr->ToString(Depth + 1);
    if (ElseBrExpr != nullptr)
        Res += '\n' + ElseBrExpr->ToString(Depth + 1);
    return Res;
}


RefNode::RefNode(PNode *Expr, bool IsDeref) : Expr(Expr), IsDeref(IsDeref) {}

RefNode::~RefNode() {
    delete Expr;
}

std::string RefNode::ToString(int Depth) {
    std::string Res = Indent(Depth);
    if (IsDeref)
        Res += "dereference";
    else
        Res += "reference";
    Res += "\n";
    Res += Expr->ToString(Depth + 1);
    return Res;
}

CallNode::CallNode(std::string CalleeName) : CalleeName(std::move(CalleeName)) {}

CallNode::~CallNode() {
    for (auto ArgExpr: ArgExprs)
        delete ArgExpr;
}

std::string CallNode::ToString(int Depth) {
    std::string Res = Indent(Depth) + "call";
    Res += '\n' + CalleeName;
    for (auto i: ArgExprs)
        Res += '\n' + i->ToString(Depth + 1);
    return Res;
}

PrototypeNode::PrototypeNode(AllocNode *Type, std::string Name, std::vector<AllocNode *> Params, bool IsVarArg,
                             PNode *BodyExpr) : ReturnAllocNode(Type), Name(std::move(Name)), Params(std::move(Params)),
                                                IsVarArg(IsVarArg), BodyExpr(BodyExpr) {}

PrototypeNode::~PrototypeNode() {}

std::string PrototypeNode::ToString(int Depth) {
    std::string Res = Indent(Depth) + ReturnAllocNode->ToString(Depth + 1) + " " + Name + " (";
    for (const auto &Pair: Params)
        Res += Pair->ToString(Depth + 1);
    Res += ")\n";
    if (BodyExpr)
        Res += BodyExpr->ToString(Depth + 1);
    return Res;
}

ReturnNode::ReturnNode(PNode *Expr) : Expr(Expr) {

}

ReturnNode::~ReturnNode() {

}

std::string ReturnNode::ToString(int Depth) {
    if (Expr != nullptr)
        return Indent(Depth) + "return\n" + Expr->ToString(Depth + 1);
    else
        return Indent(Depth) + "return void";
}

ForNode::ForNode(PNode *InitExpr, PNode *CondExpr, PNode *UpdateExpr, PNode *BodyExpr) : InitExpr(InitExpr),
                                                                                         CondExpr(CondExpr),
                                                                                         UpdateExpr(UpdateExpr),
                                                                                         BodyExpr(BodyExpr) {

}

ForNode::~ForNode() {
    delete InitExpr;
    delete CondExpr;
    delete UpdateExpr;
    delete BodyExpr;
}

std::string ForNode::ToString(int Depth) {
    std::string result = Indent(Depth) + "for ";
    result += InitExpr->ToString(0);
    result += CondExpr->ToString(0);
    result += UpdateExpr->ToString(0);
    result += "\n";
    result += BodyExpr->ToString(Depth + 1);
    return result;
}