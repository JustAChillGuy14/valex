#ifndef VALUES_H
#define VALUES_H

#ifndef bool
#define bool _Bool
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

typedef enum
{
    VAL_Number,
    VAL_Bool,
    VAL_String,
    VAL_Null,
} ValueType;

typedef struct
{
    double value;
} NumberVal;

typedef struct
{
    bool value;
} BoolVal;

typedef struct
{
    char *value;
} StringVal;

typedef struct {
    ValueType type;
    union
    {
        NumberVal n;
        BoolVal b;
        StringVal s;
    } data;
} RuntimeVal;

RuntimeVal runtimeval_number(double val);
RuntimeVal runtimeval_bool(bool b);
RuntimeVal runtimeval_string(char *s);
RuntimeVal runtimeval_null();

void dump_value(RuntimeVal val);

RuntimeVal copy_value(RuntimeVal val);

void free_value(RuntimeVal *val);
#endif