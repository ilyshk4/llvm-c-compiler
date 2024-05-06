#include "Main.h"
#include "Lexer.h"
#include "Parser.h"
#include "Gen.h"

int main(int argc, char** argv)
{
	std::string SourcePath = "program.c";
	std::ifstream ifs(SourcePath);

	if (!ifs.is_open()) {
		perror("ifstream error");
		return 1;
	}

	std::string Contents = std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());

	Lexer Lexer(Contents);
	Lexer.Tokenize();

	string LexerErrorMsg;
	if (Lexer.GetError(LexerErrorMsg))
		std::cerr << "error: " << LexerErrorMsg << std::endl;
	else {
		cout << "tokens:" << endl;
		int Depth = 0;
		for (auto Token : Lexer.GetTokens())
		{
			if (Token.Var.Type != VarType::NONE)
				std::cout << "[" << Token::GetName(Token.Type) << " " << Token.Var.ToString();
			else
				std::cout << "[" << Token::GetName(Token.Type);

			// std::cout << " " << Token.Row << ", " << Token.Column << "] ";
            std::cout << "] ";

			if (Token.Type == TType::L_BRACE)
			{
				Depth++;
				std::cout << std::endl;
				for (int i = 0; i < Depth; i++)
					std::cout << "\t";
			}

			if (Token.Type == TType::R_BRACE)
			{
				Depth--;
				std::cout << std::endl;
			}

			if (Token.Type == TType::SEMICOLON)
			{
				std::cout << std::endl;
				for (int i = 0; i < Depth; i++)
					std::cout << "\t";
			}
		}

		cout << endl;
		cout << endl;

		Parser Parser(Lexer.GetTokens());
		PNode* Result = Parser.Parse();
		cout << "ast:" << endl;
		cout << Result->ToString() << endl;

		cout << endl << "ir code:" << endl;
		Gen Generator;
		Generator.Generate(Result);
		Generator.Save("out.ll");

        cout << endl << "clang:" << endl;
        system("clang -O0 out.ll");

        //system("clang -S -emit-llvm -O0 -o out_O0.ll out.ll");
        //system("clang -S -emit-llvm -O1 -o out_O1.ll out.ll");

        cout << endl << "run:" << endl;
        system("./a.out");
	}

	return 0;
}