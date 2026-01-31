#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "frontend/lexer.h"

char *my_str_dup(const char *str)
{
    size_t len = strlen(str) + 1;
    char *copy = malloc(len);
    if (!copy)
        return NULL;
    memcpy(copy, str, len);
    copy[len - 1] = '\0';
    return copy;
}

Token token(char *value, TokenType kind)
{
    Token ret;
    ret.kind = kind;
    ret.value = my_str_dup(value); // Just in case strdup is not defined in some safe env..
    if (!ret.value)
    {
        fprintf(stderr, "Memory allocation error. Happened during making of token(uses strdup)\n");
        exit(EXIT_FAILURE);
    }
    return ret;
}

void tk_arr_init(TokenArray *arr)
{
    arr->cap = 1024;
    arr->len = 0;
    arr->tokens = malloc(sizeof(Token) * arr->cap);
    if (!arr->tokens)
    {
        fprintf(stderr, "Memory allocation error. Happened during initialization of token array.\n");
        exit(EXIT_FAILURE);
    }
}

void tk_arr_append(TokenArray *arr, Token tok)
{
    if (arr->len == arr->cap)
    {
        arr->cap += 1024;
        Token *tmp = realloc(arr->tokens, sizeof(Token) * arr->cap);
        if (!tmp)
        {
            free(arr->tokens);
            fprintf(stderr, "Memory reallocation error. Happened during appending token to tokenarray\n");
            exit(EXIT_FAILURE);
        }
        arr->tokens = tmp;
    }
    arr->tokens[arr->len++] = tok;
}

int is_binop(char c)
{
    return c == '+' || c == '-' || c == '*' || c == '/';
}

char *binopstr(char c)
{
    switch (c)
    {
    case '+':
        return "+";
    case '-':
        return "-";
    case '*':
        return "*";
    case '/':
        return "/";
    default:
        fprintf(stderr, "Exhaustive handling of binary operators in binopstr\n");
        exit(EXIT_FAILURE);
    }
}

int is_alpha(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

int is_num(char c)
{
    return c >= '0' && c <= '9';
}

int is_skippable(char c)
{
    return c == '\n' || c == ' ' || c == '\t';
}

Token *tokenize(const char *src)
{
    TokenArray ret;
    tk_arr_init(&ret);
    while (*src)
    {
        if (*src == '_' || is_alpha(*src))
        {
            size_t len = 0;
            size_t cap = 1024;
            char *buf = malloc(cap + 1);
            if (!buf)
            {
                fprintf(stderr, "Memory allocation error. Happened during identifier building\n");
                exit(EXIT_FAILURE);
            }
            while (*src && (*src == '_' || is_alpha(*src) || is_num(*src)))
            {
                if (len == cap)
                {
                    cap += 1024;
                    char *tmp = realloc(buf, cap + 1);
                    if (!tmp)
                    {
                        free(buf);
                        fprintf(stderr, "Memory reallocation error. Happened during identifier building.\n");
                        exit(EXIT_FAILURE);
                    }
                    buf = tmp;
                }
                buf[len++] = *src++;
            }
            buf[len++] = '\0';
            if (!strcmp(buf, "let"))
            {
                tk_arr_append(&ret, token(buf, TOKENTYPE_Let));
            }
            else if (!strcmp(buf, "const"))
            {
                tk_arr_append(&ret, token(buf, TOKENTYPE_Const));
            }
            else
            {
                tk_arr_append(&ret, token(buf, TOKENTYPE_Identifier));
            }
            free(buf);
        }
        else if (is_num(*src))
        {
            size_t len = 0;
            size_t cap = 1024;
            char *buf = malloc(cap + 1);
            if (!buf)
            {
                fprintf(stderr, "Memory allocation error. Happened during number building\n");
                exit(EXIT_FAILURE);
            }
            while (*src && is_num(*src))
            {
                if (len == cap)
                {
                    cap += 1024;
                    char *tmp = realloc(buf, cap + 1);
                    if (!tmp)
                    {
                        free(buf);
                        fprintf(stderr, "Memory reallocation error. Happened during number building.\n");
                        exit(EXIT_FAILURE);
                    }
                    buf = tmp;
                }
                buf[len++] = *src++;
            }
            buf[len++] = '\0';
            tk_arr_append(&ret, token(buf, TOKENTYPE_Number));
            free(buf);
        }
        else if (*src == '"')
        {
            size_t len = 0;
            size_t cap = 1024;
            char *buf = malloc(cap + 1);
            if (!buf)
            {
                fprintf(stderr, "Memory allocation error. Happened during number building\n");
                exit(EXIT_FAILURE);
            }
            src++;
            while (*src && *src != '"')
            {
                if (len == cap)
                {
                    cap += 1024;
                    char *tmp = realloc(buf, cap);
                    if (!tmp)
                    {
                        free(buf);
                        fprintf(stderr, "Memory reallocation error. Happened during number building.\n");
                        exit(EXIT_FAILURE);
                    }
                    buf = tmp;
                }
                buf[len++] = *src++;
            }
            if (*src != '"') // We reached end of src,but didnt get '"'
            {
                fprintf(stderr, "String literal not ended.\n");
                exit(EXIT_FAILURE);
            }
            buf[len++] = '\0';
            tk_arr_append(&ret, token(buf, TOKENTYPE_String));
            free(buf);
            src++;
        }
        else if (*src == '(')
        {
            tk_arr_append(&ret, token("(", TOKENTYPE_OpenParen));
            src++;
        }
        else if (*src == ')')
        {
            tk_arr_append(&ret, token(")", TOKENTYPE_CloseParen));
            src++;
        }
        else if (*src == '>')
        {
            if (src[1] == '=')
            {
                tk_arr_append(&ret, token(">=", TOKENTYPE_BinaryOperator));
                src++;
                src++;
                continue;
            }

            tk_arr_append(&ret, token(">", TOKENTYPE_BinaryOperator));
            src++;
        }
        else if (*src == '<')
        {
            if (src[1] == '=')
            {
                tk_arr_append(&ret, token("<=", TOKENTYPE_BinaryOperator));
                src++;
                src++;
                continue;
            }

            tk_arr_append(&ret, token("<", TOKENTYPE_BinaryOperator));
            src++;
        }
        else if (*src == '=')
        {
            if (src[1] == '=')
            {
                tk_arr_append(&ret, token("==", TOKENTYPE_BinaryOperator));
                src++;
                src++;
                continue;
            }
            
            tk_arr_append(&ret, token("=", TOKENTYPE_Equals));
            src++;
        }
        else if (*src == ';')
        {
            tk_arr_append(&ret, token(";", TOKENTYPE_SemiColon));
            src++;
        }
        else if (*src == '/' && src[1] == '/') // Even if src[1] is '\0' then it's not like its illegal but if src[0] is '\0' then it shouldve already been stopped
        {
            while (*src && (*src != '\n'))
            {
                src++;
            }
        }
        else if (*src == '/' && src[1] == '*') // Even if src[1] is '\0' then it's not like its illegal but if src[0] is '\0' then it shouldve already been stopped
        {
            src++;
            src++;
            size_t depth = 1;
            while (depth)
            {
                while (*src && !(*src == '*' && src[1] == '/'))
                {
                    if (*src == '/' && src[1] == '*')
                    {
                        depth++;
                        src++;
                        src++;
                    }
                    else src++;
                }
                if (!*src)
                {
                    if (depth == 1)
                    {
                        fprintf(stderr, "Unclosed multi-line comment.\n");
                    }
                    else
                    {
                        fprintf(stderr, "Unclosed multi-line comment at depth %zu.\n", depth);
                    }
                    exit(EXIT_FAILURE);
                }
                src++;
                src++;
                depth--;
            }
            src++;
            src++;
        }
        else if (is_binop(*src))
        {
            tk_arr_append(&ret, token(binopstr(*src), TOKENTYPE_BinaryOperator));
            src++;
        }
        else if (is_skippable(*src))
        {
            src++;
        }
        else
        {
            fprintf(stderr, "Unrecognized character found in source: %c ASCII value: %d\n", *src, *src);
            exit(EXIT_FAILURE);
        }
    }
    Token eof;
    eof.kind = TOKENTYPE_Eof;
    eof.value = NULL;
    tk_arr_append(&ret, eof);
    return ret.tokens;
}

void free_tokens(Token *toks)
{
    Token *orig = toks;
    while (toks->kind != TOKENTYPE_Eof)
    {
        free(toks->value);
        toks++;
    }
    free(toks->value);
    free(orig); // Free the array itself.
}

void dump_tokens(Token *tokens)
{
    printf("[\n");
    while (tokens->kind != TOKENTYPE_Eof)
    {
        printf("\t{ value:\"%s\", kind:%d },\n", tokens->value, tokens->kind);
        tokens++;
    }
    printf("\t{ value:\"EndOfFile\", kind:%d }\n", tokens->kind);
    printf("]\n");
}