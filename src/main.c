#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "frontend/lexer.h"
#include "frontend/ast.h"
#include "frontend/parser.h"

#include "runtime/values.h"
#include "runtime/scope.h"
#include "runtime/interpreter.h"

int main()
{
    int c;
    Parser p;
    Scope s;
    init_global_scope(&s);
    char *buf = NULL;
    size_t len;
    size_t cap;
    printf(">>> ");
    while ((c = getchar()) != EOF)
    {
        if (!buf)
        {
            len = 0;
            cap = 1024;
            buf = malloc(cap + 1);
            if (!buf)
            {
                fprintf(stderr, "Memory allocation error. Happened while reading from stdin.\n");
                exit(EXIT_FAILURE);
            }
        }
        if (c != '\n')
        {
            if (len == cap)
            {
                cap *= 2;
                buf = realloc(buf, cap + 1);
                if (!buf)
                {
                    fprintf(stderr, "Memory reallocation error. Happened while reading from stdin.\n");
                    exit(EXIT_FAILURE);
                }
            }
            buf[len++] = c;
        }
        else
        {
            buf[len++] = '\0';
            
            if (!strcmp(buf,"exit"))
            {
                free_scope(&s);
                exit(0); //We cannot just "break" because it would print out a '\n'(since EOF was triggered.)
            }

            Program program = parse_src(buf, &p);
            RuntimeVal evaled = eval_program(program, &s);
            dump_value(evaled);
            free_program(&program);
            free_value(&evaled);
            free(buf);
            buf = NULL;
            printf(">>> ");
        }
    }
    printf("\n");
    free_scope(&s);
    return 0;
}
