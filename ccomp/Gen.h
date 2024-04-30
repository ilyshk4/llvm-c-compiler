#ifndef GEN_H
#define GEN_H

#include <fstream>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/ADT/APInt.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Casting.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "Parser.h"

using namespace llvm;

class GStruct {
public:
    std::vector<std::string> VarNames;
    std::vector<Type *> VarTypes;
    StructType *StrType;

    GStruct(std::vector<std::string> VarNames, std::vector<Type *> VarTypes, StructType *StrType);
};

class GPointer {
public:
    Type *PointeeType;
    size_t Depth;
};

class GScope {
public:
    GScope *Parent;
    std::map<std::string, AllocaInst *> Allocas;
    std::map<std::string, GStruct *> Structs;
    std::map<Value *, GPointer> Pointers;

    GScope(GScope *Parent);
};

class Gen {
public:
    Gen();

    LLVMContext *Context;
    IRBuilder<> *Builder;
    Module *MainModule;

    GScope *CurScope;

    std::map<std::string, Type *> Types;

    Type *TVoid;
    Type *TInt8;
    Type *TInt16;
    Type *TInt32;
    Type *TInt64;
    Type *TFloat32;
    Type *TFloat64;
    Type *TPtr;

    Value *ThrowError(PNode *RelatedNode, std::string Text);

    void Generate(PNode *Node);

    void Save(const std::string &Path) const;

    void PushScope();

    void PopScope();

    bool TryPutStruct(const std::string Name, GStruct *Str);

    bool TryGetStruct(const std::string Name, GStruct **StrPtr);

    bool TryPutValue(const std::string Name, AllocaInst *Alloca);

    bool TryGetValue(const std::string Name, AllocaInst **AllocaPtr);

    bool TryPutPointer(Value *PtrVal, Type *Pointee, size_t Depth);

    bool TryGetPointer(Value *PtrVal, Type **Pointee, size_t *Depth);

    bool TryPutType(std::string Name, Type *Type);

    bool TryGetType(std::string Name, Type **TypePtr);
};

#endif