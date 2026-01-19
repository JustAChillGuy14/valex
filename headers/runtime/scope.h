#ifndef SCOPE_H
#define SCOPE_H
#include <stddef.h> //Not sure why,but the preivously needed for size_t is not needed here acc. to vs code idk why???...
#include "runtime/values.h"
struct Scope
{
    char **keys;
    RuntimeVal *values;
    char **constants;
    size_t len;
    size_t cap;
    size_t constantslen;
    size_t constantscap;
    struct Scope *parent;
};
typedef struct Scope Scope;

int resolve(Scope *scope, char *varname, Scope **out_scope, size_t *out_idx);

RuntimeVal declarevar(Scope *scope, char *varname, RuntimeVal value, int isconst);
RuntimeVal getvar(Scope *scope, char *varname);
RuntimeVal setvar(Scope *scope, char *varname, RuntimeVal value);

void init_scope(Scope *scope);
void init_global_scope(Scope *scope);

Scope new_scope(Scope *parent);
void free_scope(Scope *scope);
#endif