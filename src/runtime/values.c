#include <stdio.h>
#include <stdlib.h>
#include "runtime/values.h"

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

void dump_value(RuntimeVal val)
{
    switch (val.type)
    {
    case VAL_Number:
        printf("%f\n",val.data.n.value);
        break;
    case VAL_Null:
        printf("null\n");
        break;
    default:
        fprintf(stderr,"Exhaustive handling of ValueType in dump_value.\n");
        exit(EXIT_FAILURE);
    }
}