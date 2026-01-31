#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "runtime/values.h"
#include "runtime/scope.h"
#include "frontend/lexer.h"

int resolve(Scope *scope, char *varname, Scope **out_scope, size_t *out_idx)
{
    for (size_t i = 0; i < scope->len; i++)
    {
        if (!strcmp(varname, scope->keys[i]))
        {
            *out_scope = scope;
            *out_idx = i;
            return 1;
        }
    }
    if (!scope->parent)
        return 0;
    return resolve(scope->parent, varname, out_scope, out_idx);
}

RuntimeVal declarevar(Scope *scope, char *varname, RuntimeVal value, int isconst)
{
    for (size_t i = 0; i < scope->len; i++)
    {
        if (!strcmp(scope->keys[i], varname))
        {
            fprintf(stderr, "Cannot redeclare already declared variable: %s\n", varname);
            exit(EXIT_FAILURE);
        }
    }
    if (scope->len == scope->cap)
    {
        scope->cap += 1024;
        char **tmp = realloc(scope->keys, sizeof(char *) * scope->cap);
        if (!tmp)
        {
            fprintf(stderr, "Memory reallocation error. Happened during declaration of variable %s\n", varname);
            exit(EXIT_FAILURE);
        }
        RuntimeVal *tmp2 = realloc(scope->values, sizeof(RuntimeVal) * scope->cap);
        if (!tmp2)
        {
            fprintf(stderr, "Memory reallocation error. Happened during declaration of variable %s\n", varname);
            exit(EXIT_FAILURE);
        }
        scope->keys = tmp;
        scope->values = tmp2;
    }
    scope->keys[scope->len] = my_str_dup(varname);
    if (!scope->keys[scope->len])
    {
        fprintf(stderr, "Memory allocation error. Happened during declaration of variable %s\n", varname);
        exit(EXIT_FAILURE);
    }
    scope->values[scope->len++] = value;
    if (isconst)
    {
        if (scope->constantscap == scope->constantslen)
        {
            scope->constantscap += 1024;
            char **tmp = realloc(scope->constants, sizeof(char *) * scope->constantscap);
            if (!tmp)
            {
                fprintf(stderr, "Memory reallocation error. Happened during declaration of variable %s\n", varname);
                exit(EXIT_FAILURE);
            }
            scope->constants = tmp;
        }
        scope->constants[scope->constantslen++] = my_str_dup(varname);
        if (!scope->constants[scope->constantslen - 1])
        {
            fprintf(stderr, "Memory allocation error. Happened during declaration of varible %s\n", varname);
            exit(EXIT_FAILURE);
        }
    }

    return copy_value(value);
}

RuntimeVal getvar(Scope *scope, char *varname)
{
    size_t idx;
    Scope *s;
    if (!resolve(scope, varname, &s, &idx))
    {
        fprintf(stderr, "Cannot resolve variable %s\n", varname);
        exit(EXIT_FAILURE);
    }
    return copy_value(s->values[idx]);
}

RuntimeVal setvar(Scope *scope, char *varname, RuntimeVal value)
{
    size_t i;
    Scope *s;
    if (!resolve(scope, varname, &s, &i))
    {
        fprintf(stderr,"Cannot resolve variable %s\n",varname);
        exit(EXIT_FAILURE);
    }

    for (size_t idx = 0; idx < s->constantslen; idx++)
    {
        if (!strcmp(s->constants[idx], varname))
        {
            fprintf(stderr, "Reassignment to constant variable %s\n", varname);
            exit(EXIT_FAILURE);
        }
    }
    s->values[i] = value;
    return copy_value(value);
}

void init_scope(Scope *scope)
{
    scope->cap = 1024;
    scope->constantscap = 1024;
    scope->len = 0;
    scope->constantslen = 0;
    scope->keys = malloc(sizeof(char *) * scope->cap);
    if (!scope->keys)
    {
        fprintf(stderr, "Memory allocation error. Happened during initialization of scope\n");
        exit(EXIT_FAILURE);
    }
    scope->values = malloc(sizeof(RuntimeVal) * scope->cap);
    if (!scope->values)
    {
        fprintf(stderr, "Memory allocation error. Happened during initialization of scope\n");
        exit(EXIT_FAILURE);
    }
    scope->constants = malloc(sizeof(char *) * scope->constantscap);
    if (!scope->constants)
    {
        fprintf(stderr, "Memory allocation error. Happened during initialization of scope\n");
        exit(EXIT_FAILURE);
    }
}

Scope new_scope(Scope *parent)
{
    Scope ret;
    ret.parent = parent;
    init_scope(&ret);
    return ret;
}

void init_global_scope(Scope *scope)
{
    init_scope(scope);
    scope->parent = NULL;
    declarevar(scope, "null", runtimeval_null(), 1);
    declarevar(scope, "true", runtimeval_bool(true), 1);
    declarevar(scope, "false", runtimeval_bool(false), 1);
}

void free_scope(Scope *scope)
{
    for (size_t i = 0; i < scope->len; i++)
    {
        free(scope->keys[i]);
    }
    free(scope->keys);
    free(scope->values);
    for (size_t i = 0; i < scope->constantslen; i++)
    {
        free(scope->constants[i]);
    }
    free(scope->constants);
}