#ifndef VALUE_H
#define VALUE_H

#include <string>

enum class VarType {
    NONE,
    PTR,
    INT8,
    INT16,
    INT32,
    INT64,
    FLOAT16,
    FLOAT32,
};

struct TVar {
public:
    VarType Type;
    union {
        const char *CharPtr;
        int8_t Char;
        int16_t Short;
        int32_t Int;
        int64_t Long;
        float Float;
        double Double;
    } As;

    TVar();

    TVar(const char *CharPtr);

    TVar(int8_t Char);

    TVar(int16_t Short);

    TVar(int32_t Int);

    TVar(int64_t Long);

    TVar(float Float);

    TVar(double Double);

    std::string ToString();
};

#endif