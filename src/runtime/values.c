#include <stdio.h>
#include <stdlib.h>
#include "runtime/values.h"

#include "frontend/lexer.h"

RuntimeVal runtimeval_null() 
{
    RuntimeVal ret;
    ret.type = VAL_Null;
    return ret;
}

RuntimeVal runtimeval_number(double val)
{
    RuntimeVal ret;
    ret.type = VAL_Number;
    ret.data.n.value = val;
    return ret;
}

RuntimeVal runtimeval_bool(bool b)
{
    RuntimeVal ret;
    ret.type = VAL_Bool;
    ret.data.b.value = b;
    return ret;
}

RuntimeVal runtimeval_string(char *s)
{
    RuntimeVal ret;
    ret.type = VAL_String;
    ret.data.s.value = my_str_dup(s);
    if (!ret.data.s.value)
    {
        fprintf(stderr,"Memory allocation error. Happened while allocating memory for StringVal.\n");
        exit(EXIT_FAILURE);
    }
    return ret;
}

void dump_value(RuntimeVal val)
{
    switch (val.type)
    {
    case VAL_Number:
        if ((int)(val.data.n.value) == val.data.n.value) {
            printf("%.0f\n",val.data.n.value);
            break;
        }
        else if ((int)(val.data.n.value * 10) == (val.data.n.value * 10)) {
            printf("%.1f\n",val.data.n.value);
            break;
        }
        else if ((int)(val.data.n.value * 100) == (val.data.n.value * 100)) {
            printf("%.2f\n",val.data.n.value);
            break;
        }
        else if ((int)(val.data.n.value * 1000) == (val.data.n.value * 1000)) {
            printf("%.3f\n",val.data.n.value);
            break;
        }
        else if ((int)(val.data.n.value * 10000) == (val.data.n.value * 10000)) {
            printf("%.4f\n",val.data.n.value);
            break;
        }
        else if ((int)(val.data.n.value * 100000) == (val.data.n.value * 100000)) {
            printf("%.5f\n",val.data.n.value);
            break;
        }
        printf("%f\n",val.data.n.value);
        break;
    case VAL_Bool:
        printf(val.data.b.value ? "true\n" : "false\n");
        break;
    case VAL_String:
        printf("%s\n",val.data.s.value);
        break;
    case VAL_Null:
        printf("null\n");
        break;
    default:
        fprintf(stderr,"Exhaustive handling of ValueType in dump_value.\n");
        exit(EXIT_FAILURE);
    }
}

RuntimeVal copy_value(RuntimeVal value)
{
    RuntimeVal ret;
    switch (value.type)
    {
    case VAL_Null:
    case VAL_Number:
    case VAL_Bool:
        ret = value;
        break;
    case VAL_String:
        ret.type = VAL_String;
        ret.data.s.value = my_str_dup(value.data.s.value);
        break;
    default:
        fprintf(stderr,"Exhaustive handling of ValueType in copy_value.\n");
        exit(EXIT_FAILURE);
    }
    return ret;
}

void free_value(RuntimeVal *value)
{
    switch (value->type)
    {
    case VAL_Null:
    case VAL_Number:
    case VAL_Bool:
        break;
    case VAL_String:
        free(value->data.s.value);
        break;
    default:
        fprintf(stderr,"Exhaustive handling of ValueType in free_value.\n");
        exit(EXIT_FAILURE);
    }
}