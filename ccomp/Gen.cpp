#include "Gen.h"

GScope::GScope(GScope *Parent) {
    this->Parent = Parent;
}

Value *Gen::ThrowError(PNode *RelatedNode, std::string Text) {
    std::cerr << "error at " << RelatedNode->Row + 1 << ":" << RelatedNode->Column + 1 << ": " << Text << std::endl;
    exit(1);
}

GStruct::GStruct(std::vector<std::string> VarNames, std::vector<Type *> VarTypes, StructType *StrType) : VarNames(std::move(VarNames)),
                                                                                                         VarTypes(std::move(VarTypes)),
                                                                                                         StrType(StrType) {
}

Gen::Gen() {
    Context = new LLVMContext();
    MainModule = new Module("main", *Context);
    Builder = new IRBuilder<>(*Context);

    CurScope = nullptr;

    TVoid = Type::getVoidTy(*Context);
    TInt8 = Type::getInt8Ty(*Context);
    TInt16 = Type::getInt16Ty(*Context);
    TInt32 = Type::getInt32Ty(*Context);
    TInt64 = Type::getInt64Ty(*Context);
    TFloat32 = Type::getFloatTy(*Context);
    TFloat64 = Type::getDoubleTy(*Context);
    TPtr = PointerType::getUnqual(*Context);

    Types.emplace("void", TVoid);
    Types.emplace("char", TInt8);
    Types.emplace("short", TInt16);
    Types.emplace("int", TInt32);
    Types.emplace("long", TInt64);
    Types.emplace("float", TFloat32);
    Types.emplace("double", TFloat64);
}

void Gen::Generate(PNode *Node) {
    // Pushing Global scope
    PushScope();
    Node->Emit(this);
    PopScope();
}

void Gen::Save(const std::string &Path) const {
    MainModule->dump();
    std::string Str;
    raw_string_ostream OS(Str);
    OS << *MainModule;

    OS.flush();
    std::ofstream Out(Path);
    Out << Str;
    Out.close();
}

void Gen::PushScope() {
    CurScope = new GScope(CurScope);
}

void Gen::PopScope() {
    auto OldScope = CurScope;
    CurScope = CurScope->Parent;
    delete OldScope;
}

bool Gen::TryPutStruct(const std::string Name, GStruct *Str) {
    return CurScope->Structs.try_emplace(Name, Str).second;
}

bool Gen::TryGetStruct(const std::string Name, GStruct **StrPtr) {
    GScope *Scope = CurScope;
    while (Scope) {
        auto Result = Scope->Structs.find(Name);

        if (Result != Scope->Structs.end()) {
            *StrPtr = Result->second;
            return true;
        } else
            Scope = Scope->Parent;
    }
    return false;
}

bool Gen::TryPutValue(const std::string Name, AllocaInst *Alloca) {
    return CurScope->Allocas.try_emplace(Name, Alloca).second;
}

bool Gen::TryGetValue(const std::string Name, AllocaInst **AllocaPtr) {
    GScope *Scope = CurScope;
    while (Scope) {
        auto Result = Scope->Allocas.find(Name);

        if (Result != Scope->Allocas.end()) {
            *AllocaPtr = Result->second;
            return true;
        } else
            Scope = Scope->Parent;
    }
    return false;
}

bool Gen::TryPutPointer(Value *PtrVal, Type *Pointee, size_t Depth) {
    GPointer Value{};

    Value.PointeeType = Pointee;
    Value.Depth = Depth;

    return CurScope->Pointers.try_emplace(PtrVal, Value).second;
}

bool Gen::TryGetPointer(Value *PtrVal, Type **Pointee, size_t *Depth) {
    GScope *Scope = CurScope;
    while (Scope) {
        auto Result = Scope->Pointers.find(PtrVal);

        if (Result != Scope->Pointers.end()) {
            *Pointee = Result->second.PointeeType;
            *Depth = Result->second.Depth;
            return true;
        } else
            Scope = Scope->Parent;
    }
    return false;
}

bool Gen::TryPutType(std::string Name, Type *Type) {
    return Types.try_emplace(Name, Type).second;
}

bool Gen::TryGetType(std::string Name, Type **TypePtr) {
    {
        auto Result = Types.find(Name);

        if (Result != Types.end()) {

            *TypePtr = Result->second;
            return true;
        }
    }

    GScope *Scope = CurScope;
    while (Scope) {
        auto result = Scope->Structs.find(Name);

        if (result != Scope->Structs.end()) {
            *TypePtr = result->second->StrType;
            return true;
        } else
            Scope = Scope->Parent;
    }
    return false;
}

Value *IdentifierNode::Emit(Gen *G) {
    AllocaInst *Alloca;
    if (!G->TryGetValue(Name, &Alloca))
        return G->ThrowError(this, "unknown variable name `" + Name + "`");

    if (Alloca->isArrayAllocation()) {
        auto ElType = Alloca->getAllocatedType();

        if (IndexExpr) {
            auto Index = IndexExpr->Emit(G);
            auto El = G->Builder->CreateGEP(ElType, Alloca, Index);
            return G->Builder->CreateLoad(ElType, El, Name);
        } else {
            return G->Builder->CreateConstGEP1_32(ElType, Alloca, 0);
        }
    } else {
        return G->Builder->CreateLoad(Alloca->getAllocatedType(), Alloca, Name);
    }
}

Value *IntegerNode::Emit(Gen *G) {
    return ConstantInt::get(*G->Context, APInt(NumBits, Value, true));
}

Value *FloatNode::Emit(Gen *G) {
    return nullptr;
}

Value *StringNode::Emit(Gen *G) {
    return G->Builder->CreateGlobalStringPtr(Text);
}

Value *BinOpNode::Emit(Gen *G) {
    switch (OpType) {
        case TType::PLUS:
            return G->Builder->CreateAdd(LHS->Emit(G), RHS->Emit(G));
        case TType::MINUS:
            return G->Builder->CreateSub(LHS->Emit(G), RHS->Emit(G));
        case TType::STAR:
            return G->Builder->CreateMul(LHS->Emit(G), RHS->Emit(G));
        case TType::SLASH:
            return G->Builder->CreateSDiv(LHS->Emit(G), RHS->Emit(G));
        case TType::BIN_OR:
            return G->Builder->CreateOr(LHS->Emit(G), RHS->Emit(G));
        case TType::BIN_AND:
            return G->Builder->CreateAnd(LHS->Emit(G), RHS->Emit(G));
        case TType::GREAT_EQ:
            return G->Builder->CreateICmpSGE(LHS->Emit(G), RHS->Emit(G));
        case TType::GREAT:
            return G->Builder->CreateICmpSGT(LHS->Emit(G), RHS->Emit(G));
        case TType::D_EQUAL:
            return G->Builder->CreateICmpEQ(LHS->Emit(G), RHS->Emit(G));
        case TType::BANG_EQ:
            return G->Builder->CreateICmpNE(LHS->Emit(G), RHS->Emit(G));
        case TType::LESS:
            return G->Builder->CreateICmpSLT(LHS->Emit(G), RHS->Emit(G));
        case TType::LESS_EQ:
            return G->Builder->CreateICmpSLE(LHS->Emit(G), RHS->Emit(G));
        default:
            return nullptr;
    }
}

Value *UnOpNode::Emit(Gen *gen) {
    return nullptr;
}

Value *AssignNode::Emit(Gen *G) {
    AllocaInst *Alloca;
    std::string AllocaName;

    if (Alloc) {
        Alloc->Emit(G);
        AllocaName = Alloc->Name;
    }

    if (Ident) {
        AllocaName = Ident->Name;
    }

    if (!G->TryGetValue(AllocaName, &Alloca))
        return G->ThrowError(this, "unknown variable name");

    auto ExprValue = Expr->Emit(G);

    if (Ident && Ident->IndexExpr) {

        auto PtrType = dyn_cast<PointerType>(Alloca->getAllocatedType());

        if (!PtrType && !Alloca->isArrayAllocation())
            G->ThrowError(this, "indexee must be array or a pointer");

        auto Index = Ident->IndexExpr->Emit(G);

        Value *El = nullptr;

        if (PtrType) {
            Type *Pointee;
            size_t Depth;
            if (!G->TryGetPointer(Alloca, &Pointee, &Depth))
                G->ThrowError(this, "cant find pointer");
            auto Type = Depth > 1 ? G->TPtr : Pointee;
            El = G->Builder->CreateGEP(Type, Alloca, Index);
        } else {
            El = G->Builder->CreateGEP(Alloca->getAllocatedType(), Alloca, Index);
        }

        G->Builder->CreateStore(ExprValue, El);
    } else {
        G->Builder->CreateStore(ExprValue, Alloca);
    }

    return ExprValue;
}

Value *RefNode::Emit(Gen *G) {
    auto Ident = dynamic_cast<IdentifierNode *>(Expr);
    if (IsDeref) {
        auto EmitVal = Expr->Emit(G);
        auto PtrVal = getPointerOperand(EmitVal);

        Type *Pointee;
        size_t Depth;
        if (!G->TryGetPointer(PtrVal, &Pointee, &Depth))
            return G->ThrowError(Expr, "cant find pointer");

        auto Type = Depth > 1 ? G->TPtr : Pointee;

        auto LoadVal = EmitVal;
        for (int i = 0; i < Depth; i++)
            LoadVal = G->Builder->CreateLoad(Type, LoadVal);
        return LoadVal;
    } else {
        if (!Ident)
            return G->ThrowError(this, "cannot reference not identifier");

        AllocaInst *Alloca;
        if (!G->TryGetValue(Ident->Name, &Alloca))
            return G->ThrowError(this, "unknown variable name `" + Ident->Name + "`");
        return Alloca;
    }
}

Value *AllocNode::Emit(Gen *G) {
    Type *VarType;
    if (!G->TryGetType(AllocTypeName, &VarType))
        return G->ThrowError(this, "unknown type");

    auto AllocaType = PtrDepth ? G->TPtr : VarType;

    auto ArraySizeVal = ArraySizeExpr ? ArraySizeExpr->Emit(G) : nullptr;
    auto Alloca = G->Builder->CreateAlloca(AllocaType, ArraySizeVal, Name);

    if (PtrDepth && !G->TryPutPointer(Alloca, VarType, PtrDepth))
        return G->ThrowError(this, "pointer already exists");

    if (!G->TryPutValue(Name, Alloca))
        return G->ThrowError(this, "name already exists");
    return Alloca;
}

Value *StructNode::Emit(Gen *G) {
    auto StrType = StructType::create(*G->Context, Name);

    std::vector<std::string> VarNames;
    std::vector<Type *> VarTypes;
    for (auto Alloc : AllocNodes) {
        Type *AllocType;
        if (!G->TryGetType(Alloc->AllocTypeName, &AllocType))
            return G->ThrowError(this, "unknown type");

        auto AllocaType = Alloc->PtrDepth ? G->TPtr : AllocType;

        VarNames.push_back(Alloc->Name);
        VarTypes.push_back(AllocaType);
    }

    StrType->setBody(VarTypes);

    auto Str = new GStruct(VarNames, VarTypes, StrType);

    if (!G->TryPutStruct(Name, Str))
        return G->ThrowError(this, "struct name already exists");
}

Value *TypedefNode::Emit(Gen *G) {
    Type *DefType;

    if (!G->TryGetType(Alloc->AllocTypeName, &DefType))
        return G->ThrowError(this, "unknown type");

    if (!G->TryPutType(Alloc->Name, DefType))
        return G->ThrowError(this, "type exists");
}

Value *BlockNode::Emit(Gen *G) {
    G->PushScope();
    for (auto Node: Nodes) {
        Node->Emit(G);
    }
    G->PopScope();
    return nullptr;
}

Value *IfNode::Emit(Gen *G) {
    Value *CondVal = CondExpr->Emit(G);
    if (!CondVal)
        return nullptr;
    Value *CondValAsBit = G->Builder->CreateIntCast(CondVal, Type::getInt1Ty(*G->Context), false);

    Function *Func = G->Builder->GetInsertBlock()->getParent();

    BasicBlock *ThenBlock = BasicBlock::Create(*G->Context, "then", Func);
    BasicBlock *ElseBlock = BasicBlock::Create(*G->Context, "else");
    BasicBlock *MergeBlock = BasicBlock::Create(*G->Context, "finally");

    G->Builder->CreateCondBr(CondValAsBit, ThenBlock, ElseBlock);

    G->Builder->SetInsertPoint(ThenBlock);

    BodyExpr->Emit(G);

    G->Builder->CreateBr(MergeBlock);

    Func->getBasicBlockList().push_back(ElseBlock);
    G->Builder->SetInsertPoint(ElseBlock);

    if (ElseBrExpr)
        ElseBrExpr->Emit(G);

    G->Builder->CreateBr(MergeBlock);

    Func->getBasicBlockList().push_back(MergeBlock);
    G->Builder->SetInsertPoint(MergeBlock);

    return nullptr;
}

Value *ForNode::Emit(Gen *G) {
    G->PushScope();

    if (InitExpr)
        InitExpr->Emit(G);

    Function *Func = G->Builder->GetInsertBlock()->getParent();

    BasicBlock *LoopCondBlock = BasicBlock::Create(*G->Context, "condition", Func);
    BasicBlock *LoopBeginBlock = BasicBlock::Create(*G->Context, "entry");
    BasicBlock *LoopEndBlock = BasicBlock::Create(*G->Context, "finally");

    G->Builder->CreateBr(LoopCondBlock);
    G->Builder->SetInsertPoint(LoopCondBlock);

    Value *CondVal = CondExpr ? CondExpr->Emit(G) : ConstantInt::getTrue(*G->Context);

    Value *CondValAsBit = G->Builder->CreateIntCast(CondVal, Type::getInt1Ty(*G->Context), false);

    G->Builder->CreateCondBr(CondValAsBit, LoopBeginBlock, LoopEndBlock);

    Func->getBasicBlockList().push_back(LoopBeginBlock);
    G->Builder->SetInsertPoint(LoopBeginBlock);

    BodyExpr->Emit(G);
    if (UpdateExpr)
        UpdateExpr->Emit(G);

    G->Builder->CreateBr(LoopCondBlock);

    Func->getBasicBlockList().push_back(LoopEndBlock);
    G->Builder->SetInsertPoint(LoopEndBlock);

    G->PopScope();
}

Value *CallNode::Emit(Gen *G) {
    Function *CalleeFunc = G->MainModule->getFunction(CalleeName);
    if (!CalleeFunc)
        return G->ThrowError(this, "unknown function referenced");

    if (!CalleeFunc->isVarArg() && CalleeFunc->arg_size() != ArgExprs.size())
        return G->ThrowError(this, "incorrect # arguments passed");

    std::vector<Value *> ArgsVals;
    for (auto &ArgExpr: ArgExprs) {
        ArgsVals.push_back(ArgExpr->Emit(G));
        if (!ArgsVals.back())
            return nullptr;
    }

    return G->Builder->CreateCall(CalleeFunc, ArgsVals);
}

Value *PrototypeNode::Emit(Gen *G) {
    G->PushScope();

    std::vector<Type *> Types;
    for (const auto &Param: Params) {
        Type *ParamType;
        if (!G->TryGetType(Param->AllocTypeName, &ParamType))
            return G->ThrowError(this, "unknown type");
        auto AllocaType = Param->PtrDepth ? G->TPtr : ParamType;

        Types.push_back(AllocaType);
    }

    Type *ReturnType = nullptr;

    if (ReturnAllocNode->PtrDepth)
        ReturnType = G->TPtr;
    else if (!G->TryGetType(ReturnAllocNode->AllocTypeName, &ReturnType))
            return G->ThrowError(this, "unknown type");

    FunctionType *FuncType = FunctionType::get(ReturnType, Types, IsVarArg);
    Value *Val = nullptr;
    if (BodyExpr) {
        auto Func = G->MainModule->getFunction(Name);

        if (!Func)
            Func = static_cast<llvm::Function *>(G->MainModule->getOrInsertFunction(Name, FuncType).getCallee()); //Function::Create(FuncType, Function::ExternalLinkage, Name, G->MainModule);

        Val = Func;

        unsigned Index = 0;
        for (auto &Arg: Func->args())
            Arg.setName(Params[Index++]->Name);

        BasicBlock *BodyBlock = BasicBlock::Create(*G->Context, "entry", Func);
        G->Builder->SetInsertPoint(BodyBlock);

        Index = 0;
        for (auto &Arg: Func->args()) {
            auto Param = Params[Index];
            auto Alloca = dyn_cast<AllocaInst>(Param->Emit(G));
            G->Builder->CreateStore(&Arg, Alloca);
            Index++;
        }

        BodyExpr->Emit(G);

        if (ReturnType->isVoidTy())
            G->Builder->CreateRetVoid();
    } else {
        Val = G->MainModule->getOrInsertFunction(Name, FuncType).getCallee();
    }

    G->PopScope();

    return Val;
}

llvm::Value *ReturnNode::Emit(Gen *G) {
    if (Expr)
        G->Builder->CreateRet(Expr->Emit(G));
    else
        G->Builder->CreateRetVoid();
}