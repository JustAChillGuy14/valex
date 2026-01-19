#ifndef VALUES_H
#define VALUES_H
typedef enum
{
    VAL_Number,
    VAL_Null,
} ValueType;

typedef struct
{
    double value;
} NumberVal;

typedef struct {
    ValueType type;
    union
    {
        NumberVal n;
    } data;
} RuntimeVal;

RuntimeVal runtimeval_number(double val);
RuntimeVal runtimeval_null();

void dump_value(RuntimeVal val);
#endif