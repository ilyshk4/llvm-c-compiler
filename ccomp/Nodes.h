//
// Created by Ilya on 08.04.2024.
//

#ifndef CCOMP_NODES_H
#define CCOMP_NODES_H

#include "llvm/IR/Value.h"
#include "llvm/ADT/APInt.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "Token.h"

class Gen;

class PNode {
public:
    size_t Row, Column;

    virtual llvm::Value *Emit(Gen *G) = 0;

    virtual std::string ToString(int Depth = 0);

    virtual ~PNode();
};

class IdentifierNode : public PNode {
public:
    std::string Name;
    PNode *IndexExpr;

    IdentifierNode(std::string Name, PNode *IndexExpr);

    llvm::Value *Emit(Gen *G);

    std::string ToString(int Depth = 0);
};

class IntegerNode : public PNode {
public:
    uint64_t Value;
    size_t NumBits;

    IntegerNode(uint64_t Value, size_t NumBits);

    llvm::Value *Emit(Gen *G);

    std::string ToString(int Depth = 0);
};

class FloatNode : public PNode {
public:
    double Value;

    FloatNode(double Value);

    llvm::Value *Emit(Gen *G);

    std::string ToString(int Depth = 0);
};

class StringNode : public PNode {
public:
    std::string Text;

    StringNode(std::string Text);

    llvm::Value *Emit(Gen *G);

    std::string ToString(int Depth = 0);
};

class BinOpNode : public PNode {
public:
    TType OpType;
    PNode *LHS;
    PNode *RHS;

    BinOpNode(TType OpType, PNode *LHS, PNode *RHS);

    ~BinOpNode();

    llvm::Value *Emit(Gen *G);

    std::string ToString(int Depth = 0);
};

class UnOpNode : public PNode {
public:
    TType OpType;
    PNode *Expr;

    UnOpNode(TType OpType, PNode *Expr);

    ~UnOpNode();

    llvm::Value *Emit(Gen *G);

    std::string ToString(int Depth = 0);
};

class AllocNode : public PNode {
public:
    std::string AllocTypeName;
    std::string Name;
    size_t PtrDepth;
    PNode *ArraySizeExpr;

    AllocNode(std::string AllocTypeName, std::string Name, size_t PtrDepth, PNode *ArraySizeExpr);

    ~AllocNode();

    llvm::Value *Emit(Gen *G);

    std::string ToString(int Depth = 0);
};


class AssignNode : public PNode {
public:
    IdentifierNode *Ident;
    AllocNode *Alloc;
    PNode *Expr;

    AssignNode(IdentifierNode *Ident, PNode *Expr);

    AssignNode(AllocNode *Alloc, PNode *Expr);

    ~AssignNode();

    llvm::Value *Emit(Gen *G);

    std::string ToString(int Depth = 0);
};

class BlockNode : public PNode {
public:
    std::vector<PNode *> Nodes;

    BlockNode();

    ~BlockNode();

    llvm::Value *Emit(Gen *G);

    std::string ToString(int Depth = 0);
};

class IfNode : public PNode {
public:
    PNode *CondExpr;
    PNode *BodyExpr;
    PNode *ElseBrExpr;

    IfNode(PNode *CondExpr, PNode *BodyExpr, PNode *ElseBrExpr);

    ~IfNode();

    llvm::Value *Emit(Gen *G);

    std::string ToString(int Depth = 0);
};

class RefNode : public PNode {
public:
    bool IsDeref;
    PNode *Expr;

    RefNode(PNode *Expr, bool IsDeref);

    ~RefNode();

    llvm::Value *Emit(Gen *G);

    std::string ToString(int Depth = 0);
};

class ForNode : public PNode {
public:
    PNode *InitExpr;
    PNode *CondExpr;
    PNode *UpdateExpr;
    PNode *BodyExpr;

    ForNode(PNode *InitExpr, PNode *CondExpr, PNode *UpdateExpr, PNode *BodyExpr);

    ~ForNode();

    llvm::Value *Emit(Gen *G);

    std::string ToString(int Depth = 0);
};

class CallNode : public PNode {
public:
    std::string CalleeName;
    std::vector<PNode *> ArgExprs;

    CallNode(std::string CalleeName);

    ~CallNode();

    llvm::Value *Emit(Gen *G);

    std::string ToString(int Depth = 0);
};

class PrototypeNode : public PNode {
public:
    AllocNode *ReturnAllocNode;
    std::string Name;
    std::vector<AllocNode *> Params;
    PNode *BodyExpr;
    bool IsVarArg;

    PrototypeNode(AllocNode *Type, std::string Name, std::vector<AllocNode *> Params, bool IsVarArg, PNode *BodyExpr);

    ~PrototypeNode();

    llvm::Value *Emit(Gen *G);

    std::string ToString(int Depth = 0);
};

class ReturnNode : public PNode {
public:
    PNode *Expr;

    ReturnNode(PNode *Expr);

    ~ReturnNode();

    llvm::Value *Emit(Gen *G);

    std::string ToString(int Depth = 0);
};

class StructNode : public PNode {
public:
    std::string Name;
    std::vector<AllocNode *> AllocNodes;

    StructNode(std::string Name, std::vector<AllocNode *> AllocNodes);

    ~StructNode();

    llvm::Value *Emit(Gen *G);

    std::string ToString(int Depth);
};

class TypedefNode : public PNode {
public:
    AllocNode *Alloc;

    TypedefNode(AllocNode *Alloc);

    ~TypedefNode();

    llvm::Value *Emit(Gen *G);

    std::string ToString(int Depth);
};

#endif //CCOMP_NODES_H
