#ifndef VALUES_H
#define VALUES_H
typedef enum
{
    VAL_Number,
    VAL_String,
    VAL_Null,
} ValueType;

typedef struct
{
    double value;
} NumberVal;

typedef struct
{
    char *value;
} StringVal;

typedef struct {
    ValueType type;
    union
    {
        NumberVal n;
        StringVal s;
    } data;
} RuntimeVal;

RuntimeVal runtimeval_number(double val);
RuntimeVal runtimeval_string(char *s);
RuntimeVal runtimeval_null();

void dump_value(RuntimeVal val);

RuntimeVal copy_value(RuntimeVal val);

void free_value(RuntimeVal *val);
#endif