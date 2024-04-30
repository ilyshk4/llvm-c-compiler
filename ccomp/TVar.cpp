#include "TVar.h"

TVar::TVar() : Type(VarType::NONE) { As.CharPtr = nullptr; }

TVar::TVar(const char *CharPtr) : Type(VarType::PTR) { As.CharPtr = CharPtr; }

TVar::TVar(int8_t Char) : Type(VarType::INT8) { As.Char = Char; }

TVar::TVar(int16_t Short) : Type(VarType::INT16) { As.Short = Short; }

TVar::TVar(int32_t Int) : Type(VarType::INT32) { As.Int = Int; }

TVar::TVar(int64_t Long) : Type(VarType::INT64) { As.Long = Long; }

TVar::TVar(float Float) : Type(VarType::FLOAT16) { As.Float = Float; }

TVar::TVar(double Double) : Type(VarType::FLOAT32) { As.Double = Double; }

std::string TVar::ToString() {
    switch (Type) {
        case VarType::NONE:
            return "";
        case VarType::PTR:
            return "'" + std::string((const char *) As.CharPtr) + "'";
        case VarType::INT8:
            return "int8 " + std::to_string(As.Char);
        case VarType::INT16:
            return "int16 " + std::to_string(As.Short);
        case VarType::INT32:
            return "int32 " + std::to_string(As.Int);
        case VarType::FLOAT16:
            return "float16 " + std::to_string(As.Float);
        case VarType::FLOAT32:
            return "float32 " + std::to_string(As.Double);
        default:
            return "unknown type";
    }
}
