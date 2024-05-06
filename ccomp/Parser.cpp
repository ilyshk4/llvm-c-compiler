#include "Parser.h"

#include <utility>


Parser::Parser(std::vector<Token> Tokens) : Tokens(std::move(Tokens)), Current(0) {
    Types = {
        "void",
        "char",
        "short",
        "int",
        "long",
        "float",
        "double",
    };
}

PNode *Parser::Parse() {
    return ParseBlock();
}

bool Parser::Check(TType Type) {
    if (End())
        return false;
    return Peek().Type == Type;
}

Token Parser::Advance() {
    if (!End())
        Current++;
    return Previous();
}

Token Parser::Consume(TType Type, const char *ErrorMsg) {
    if (Check(Type)) {
        return Advance();
    }
    std::cerr << "error " << Peek().Row + 1 << ":"
              << Peek().Column + 1 << ": expected " << Token::GetName(Type)
              << " got " << Token::GetName(Peek().Type) << ": " << ErrorMsg << std::endl;
    exit(1);
}

bool Parser::End() {
    return Peek().Type == TType::END_OF_FILE;
}

Token Parser::Peek(int Offset) {
    return Tokens.at(Current + Offset);
}

Token Parser::Previous() {
    return Tokens.at(Current - 1);
}

PNode *Parser::LocateNode(PNode *Node, Token Token) {
    Node->Row = Token.Row;
    Node->Column = Token.Column;
    return Node;
}

PNode *Parser::ParseBlock() {
    auto Block = new BlockNode();
    LocateNode(Block, Peek());

    while (!Check(TType::R_BRACE) && !End()) {
        Block->Nodes.push_back(ParseStatement());
    }

    return Block;
}

PNode *Parser::ParseStatement() {
    if (Check(TType::IF)) {
        Advance();
        return ParseIfStatement();
    }

    if (Check(TType::FOR)) {
        Advance();
        return ParseForStatement();
    }

    if (Check(TType::RETURN)) {
        Advance();
        return ParseReturnStatement();
    }

    auto Expr = ParseExpression();
    if (IsSemicolonRequired(Expr))
        Consume(TType::SEMICOLON, "expected semicolon after expression statement.");
    return Expr;
}

bool Parser::IsSemicolonRequired(PNode *Expr) const {
    return !dynamic_cast<PrototypeNode *>(Expr)
               && !dynamic_cast<BlockNode *>(Expr);
}

PNode *Parser::ParseIfStatement() {
    Consume(TType::L_PAREN, "expected left parenthesis before condition.");
    PNode *CondExpr = ParseOr();
    Consume(TType::R_PAREN, "expected right parenthesis after condition.");
    PNode *BodyExpr = ParseExpression();

    if (!dynamic_cast<BlockNode *>(BodyExpr))
        Consume(TType::SEMICOLON, "expected semicolon after expression statement.");

    PNode *ElseBrExpr = nullptr;
    if (Check(TType::ELSE)) {
        Advance();
        ElseBrExpr = ParseExpression();
    }

    return new IfNode(CondExpr, BodyExpr, ElseBrExpr);
}

PNode *Parser::ParseForStatement() {
    Consume(TType::L_PAREN, "expected left parenthesis before condition.");
    PNode *InitExpr = ParseExpression();
    Consume(TType::SEMICOLON, "expected semicolon after initializer.");
    PNode *CondExpr = ParseExpression();
    Consume(TType::SEMICOLON, "expected semicolon after condition.");
    PNode *UpdateExpr = ParseExpression();
    Consume(TType::R_PAREN, "expected right parenthesis after updation.");
    PNode *BodyExpr = ParseExpression();

    if (!dynamic_cast<BlockNode *>(BodyExpr))
        Consume(TType::SEMICOLON, "expected semicolon after expression statement.");

    return new ForNode(InitExpr, CondExpr, UpdateExpr, BodyExpr);
}

PNode *Parser::ParseReturnStatement() {
    ReturnNode *Node = new ReturnNode(nullptr);
    LocateNode(Node, Previous());

    if (Check(TType::SEMICOLON))
        Advance();
    else {
        Node->Expr = ParseOr();
        Consume(TType::SEMICOLON, "expected semicolon after return statement.");
    }
    return Node;
}

PNode *Parser::ParseExpression() {
    return ParseAssignment();
}

PNode *Parser::ParseAssignment() {
    PNode *Node = ParseOr();

    while (Check(TType::EQUAL)) {
        Token Token = Peek();
        Advance();

        PNode *Expr = ParseOr();
        auto Ident = dynamic_cast<IdentifierNode *>(Node);

        if (Ident) {
            return LocateNode(new AssignNode(Ident, Expr), Token);
        }

        auto Alloc = dynamic_cast<AllocNode *>(Node);
        if (Alloc) {
            return LocateNode(new AssignNode(Alloc, Expr), Token);
        }
    }

    return Node;
}

PNode *Parser::ParseOr() {
    PNode *Node = ParseAnd();

    while (Check(TType::OR)) {
        Advance();
        Token Op = Previous();
        PNode *RHS = ParseAnd();
        Node = new BinOpNode(Op.Type, Node, RHS);
        LocateNode(Node, Op);
    }

    return Node;
}

PNode *Parser::ParseAnd() {
    PNode *Node = ParseEquality();

    while (Check(TType::AND)) {
        Advance();
        Token Op = Previous();
        PNode *RHS = ParseEquality();
        Node = new BinOpNode(Op.Type, Node, RHS);
        LocateNode(Node, Op);
    }

    return Node;
}

PNode *Parser::ParseEquality() {
    PNode *Node = ParseComparison();

    while (Check(TType::BANG_EQ) || Check(TType::D_EQUAL)) {
        Advance();
        Token Op = Previous();
        PNode *RHS = ParseComparison();
        Node = new BinOpNode(Op.Type, Node, RHS);
        LocateNode(Node, Op);
    }

    return Node;
}

PNode *Parser::ParseComparison() {
    PNode *Node = ParseTerm();

    while (Check(TType::GREAT) || Check(TType::GREAT_EQ) || Check(TType::D_EQUAL)
           || Check(TType::BANG_EQ) || Check(TType::LESS) || Check(TType::LESS_EQ)) {
        Advance();
        Token Op = Previous();
        PNode *RHS = ParseTerm();
        Node = new BinOpNode(Op.Type, Node, RHS);
        LocateNode(Node, Op);
    }

    return Node;
}

PNode *Parser::ParseTerm() {
    PNode *Node = ParseFactor();

    while (Check(TType::PLUS) || Check(TType::MINUS)) {
        Advance();
        Token Op = Previous();
        PNode *RHS = ParseFactor();
        Node = new BinOpNode(Op.Type, Node, RHS);
        LocateNode(Node, Op);
    }

    return Node;
}

PNode *Parser::ParseFactor() {
    PNode *Node = ParseUnary();

    while (Check(TType::STAR) || Check(TType::SLASH) || Check(TType::PERCENT)) {
        Advance();
        Token Op = Previous();
        PNode *RHS = ParseUnary();
        Node = new BinOpNode(Op.Type, Node, RHS);
        LocateNode(Node, Op);
    }

    return Node;
}

PNode *Parser::ParseUnary() {
    if (Check(TType::BANG) || Check(TType::MINUS)) {
        Advance();
        Token Op = Previous();
        PNode *Expr = ParseUnary();
        PNode *Node = new UnOpNode(Op.Type, Expr);
        LocateNode(Node, Op);
        return Node;
    }

    if (Check(TType::STAR)) {
        Advance();
        int Depth = 1;
        while (Check(TType::STAR)) {
            Advance();
            Depth++;
        }
        Token Op = Previous();
        PNode *Expr = ParseUnary();
        PNode *Node = new RefNode(Expr, true, Depth);
        LocateNode(Node, Op);
        return Node;
    }

    if (Check(TType::BIN_AND)) {
        Advance();
        Token Op = Previous();
        PNode *Expr = ParseUnary();
        PNode *Node = new RefNode(Expr, false, 0);
        LocateNode(Node, Op);
        return Node;
    }

    return ParseCall();
}

PNode *Parser::ParseCall() {
    PNode *Node = ParsePrimary();

    while (true) {
        if (Check(TType::L_PAREN)) {
            Advance();
            auto IdentNode = dynamic_cast<IdentifierNode *>(Node);
            auto Call = new CallNode(IdentNode->Name);
            LocateNode(Call, Previous());

            if (!Check(TType::R_PAREN)) {
                bool Comma;
                do {
                    Call->ArgExprs.push_back(ParseOr());
                    Comma = Check(TType::COMMA);
                    if (Comma)
                        Advance();
                } while (Comma);
            }

            Consume(TType::R_PAREN, "expected right parenthesis after call identifier.");

            Node = Call;
        } else break;
    }

    return Node;
}

PNode *Parser::ParsePrimary() {
    if (Check(TType::IDENTIFIER)) {

        auto Text = Peek().Var.As.CharPtr;
        Advance();

        PNode *IndexExpr = nullptr;

        if (Check(TType::IDENTIFIER)
            || IsTypeDeclared(Text) && Check(TType::STAR)) {
            return ParseAlloc(Text);
        }

        if (Check(TType::L_SCR)) {
            Advance();
            IndexExpr = ParseOr();
            Consume(TType::R_SCR, "expected right square bracket after index expression");
        }

        auto Node = new IdentifierNode(Text, IndexExpr);
        LocateNode(Node, Peek());
        return Node;
    }

    if (Check(TType::STRUCT)) {
        Advance();
        return ParseStruct();
    }

    if (Check(TType::TYPEDEF)) {
        Advance();
        return ParseTypedef();
    }

    if (Check(TType::INTEGER)) {
        Advance();
        return LocateNode(new IntegerNode(Previous().Var.As.Int, 32), Previous());
    }

    if (Check(TType::CHAR)) {
        Advance();
        return LocateNode(new IntegerNode(Previous().Var.As.Int, 8), Previous());
    }

    if (Check(TType::FLOAT)) {
        Advance();
        return LocateNode(new FloatNode(Previous().Var.As.Double), Previous());
    }

    if (Check(TType::STRING)) {
        Advance();
        return LocateNode(new StringNode(Previous().Var.As.CharPtr), Previous());
    }

    if (Check(TType::L_BRACE)) {
        Advance();
        PNode *Node = ParseBlock();
        Consume(TType::R_BRACE, "expected closing right brace.");
        return Node;
    }

    return nullptr;
}

bool Parser::IsTypeDeclared(const char *Text) { return std::find(Types.begin(), Types.end(), Text) != Types.end(); }

PNode *Parser::ParseTypedef() {
    auto TypedefToken = Peek(-1);

    if (Check(TType::STRUCT))
        Advance();

    auto TypeName = Peek().Var.As.CharPtr;
    Advance();

    auto Alloc = dynamic_cast<AllocNode *>(ParseAlloc(TypeName));
    DefineType(Alloc);
    return LocateNode(new TypedefNode(Alloc), TypedefToken);
}

void Parser::DefineType(const AllocNode *Alloc) { Types.push_back(Alloc->Name); }

PNode *Parser::ParseStruct() {
    auto Name = Peek().Var.As.CharPtr;
    Consume(TType::IDENTIFIER, "expected struct identifier");

    if (Check(TType::IDENTIFIER)) {
        return ParseAlloc(Name);
    } else {
        Consume(TType::L_BRACE, "expected left brace");

        std::vector<AllocNode *> Allocs;

        while (true) {
            auto VarTypeName = Peek().Var.As.CharPtr;
            Consume(TType::IDENTIFIER, "expected field type");

            auto Node = ParseAlloc(VarTypeName);
            auto Alloc = dynamic_cast<AllocNode *>(Node);

            Allocs.push_back(Alloc);

            Consume(TType::SEMICOLON, "expected semicolon after field");

            if (Check(TType::R_BRACE))
                break;
        }

        Consume(TType::R_BRACE, "expected right brace");

        return new StructNode(Name, Allocs);
    }
}

PNode *Parser::ParseAlloc(std::string type) {
    auto NameToken = Peek();

    std::string Name;

    int PtrDepth = 0;
    PNode *ArraySizeExpr = nullptr;

    if (Check(TType::IDENTIFIER)) {
        Advance();
        Name = NameToken.Var.As.CharPtr;
    }

    if (Check(TType::STAR)) {
        while (Check(TType::STAR)) {
            Advance();
            PtrDepth++;
        }
        NameToken = Peek();
        Name = NameToken.Var.As.CharPtr;
        Advance();
    }

    if (Check(TType::L_SCR)) {
        Advance();
        ArraySizeExpr = ParseOr();

        Consume(TType::R_SCR, "expected right square bracket after array index expression");
    }

    auto Node = new AllocNode(type, Name, PtrDepth, ArraySizeExpr);
    LocateNode(Node, NameToken);

    if (Check(TType::L_PAREN)) {
        Advance();
        return ParsePrototype(Node, Name, NameToken);
    }

    return Node;
}

PNode *Parser:: ParsePrototype(AllocNode *ReturnAlloc, std::string Name, Token ProtToken) {
    std::vector<AllocNode *> Params;
    bool IsVarArg = false;
    bool Comma = false;
    do {
        if (Check(TType::IDENTIFIER)) {
            auto ArgTypeName = Peek().Var.As.CharPtr;
            Advance();
            auto Node = ParseAlloc(ArgTypeName);
            auto Alloc = dynamic_cast<AllocNode *>(Node);
            Params.push_back(Alloc);
            Comma = Check(TType::COMMA);
            if (Comma)
                Advance();
        } else if (Check(TType::VARARG)) {
            IsVarArg = true;
            Advance();
            break;
        }
    } while (Comma);

    Consume(TType::R_PAREN, "expected right parenthesis after function definition.");

    PNode *Node = nullptr;
    if (Check(TType::SEMICOLON))
        Advance();
    else {
        Consume(TType::L_BRACE, "expected left brace.");
        Node = ParseBlock();
        Consume(TType::R_BRACE, "expected right brace.");
    }

    return LocateNode(new PrototypeNode(ReturnAlloc, Name, Params, IsVarArg, Node), ProtToken);
}

