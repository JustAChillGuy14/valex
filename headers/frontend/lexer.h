#ifndef LEXER_H
#include <stddef.h>
#define LEXER_H
typedef enum
{
    /*
    Literals
    */
    TOKENTYPE_Number,
    TOKENTYPE_String,
    TOKENTYPE_Identifier,

    /*
    Operators
    */
    TOKENTYPE_BinaryOperator,
    TOKENTYPE_UnaryOperator,

    /*
    Parenthesis(/Brackets/Braces in the future)
    */
    TOKENTYPE_OpenParen,
    TOKENTYPE_CloseParen,
    
    /*
    Variables
    */
    TOKENTYPE_Let,
    TOKENTYPE_Const,
    TOKENTYPE_Equals,
    
    /*
    Enders
    */
    TOKENTYPE_SemiColon,
    TOKENTYPE_Eof,
} TokenType;

typedef struct
{
    TokenType kind;
    char *value;
} Token;

typedef struct
{
    size_t cap;
    size_t len;
    Token *tokens;
} TokenArray;

//TODO: move my_str_dup into strutils or something.
char *my_str_dup(const char *str);

int is_unop(char c);
int is_binop(char c);
int is_alpha(char c);
int is_num(char c);
int is_skippable(char c);

char *binopstr(char c);
char *unopstr(char c);

Token token(char *value,TokenType kind);

void tk_arr_init(TokenArray *arr);
void tk_arr_append(TokenArray *arr, Token tok);

Token *tokenize(const char *src);

void free_tokens(Token *tokens);

void dump_tokens(Token *tokens);
#endif