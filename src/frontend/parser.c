#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "frontend/parser.h"

void parser_init(Parser *p, Token *tokens)
{
    p->tokens = tokens;
    p->i = 0;
}

Token at(Parser *p)
{
    return p->tokens[p->i];
}

Token eat(Parser *p)
{
    return p->tokens[p->i++];
}

Token expect(Parser *p, TokenType type)
{
    Token tok = eat(p);
    if (tok.kind != type)
    {
        fprintf(stderr, "Expected tokentype %d but recieved %d instead.\n", type, tok.kind);
        exit(EXIT_FAILURE);
    }
    return tok;
}

Token expecterr(Parser *p, TokenType type, const char *err)
{
    Token tok = eat(p);

    if (tok.kind != type)
    {
        fprintf(stderr, "%s\n", err);
        exit(EXIT_FAILURE);
    }

    return tok;
}

Program parse_src(const char *src,Parser *p)
{
    Token *tokens = tokenize(src);
    parser_init(p,tokens);
    Program ret = parse_program(p);
    free_tokens(tokens);
    return ret;
}

Program parse_program(Parser *p)
{
    Program ret;
    program_init(&ret);
    while (at(p).kind != TOKENTYPE_Eof)
    {
        program_append(&ret, parse_stmt(p));
        
        while (at(p).kind == TOKENTYPE_SemiColon)
        {
            eat(p); //Allow ;;;;;;;;;;;;;;;;;;;;;
        }
    }
    return ret;
}

Stmt *parse_stmt(Parser *p)
{
    if (at(p).kind == TOKENTYPE_Let || at(p).kind == TOKENTYPE_Const)
    {
        return parse_var_decl_stmt(p);
    }
    return make_stmt_expr_stmt(parse_expr(p));
}

Stmt *parse_var_decl_stmt(Parser *p) //This function is only to be called when it actually is a variable declaration.
{
    int isConst = eat(p).kind == TOKENTYPE_Const;
    char *ident = expecterr(p,TOKENTYPE_Identifier,"Expected identifier name after let/const.").value;
    if (at(p).kind != TOKENTYPE_Equals)
    {
        if (isConst)
        {
            fprintf(stderr,"Cannot declare constant without value.\n");
            exit(EXIT_FAILURE);
        }
        return make_stmt_var_decl_stmt(ident,NULL,0); //Handled during interpretation.
    }
    eat(p);
    Expr *value = parse_expr(p);
    return make_stmt_var_decl_stmt(ident,value,isConst);
}

Expr *parse_expr(Parser *p)
{
    return parse_assignment_expr(p); // This is always gonna be a wrapper.
}

Expr *parse_assignment_expr(Parser *p)
{
    Expr *assigne = parse_additive_expr(p);
    if (at(p).kind == TOKENTYPE_Equals)
    {
        eat(p);
        Expr *value = parse_expr(p);
        return make_expr_assignment(assigne,value);
    }
    return assigne;
}

Expr *parse_additive_expr(Parser *p)
{
    Expr *left = parse_multiplicative_expr(p);
    while (at(p).value && (!strcmp(at(p).value,"+") || !strcmp(at(p).value,"-")))
    {
        char *op = my_str_dup(eat(p).value);
        if (!op)
        {
            fprintf(stderr,"Memory allocation error when copying operation.");
            exit(EXIT_FAILURE);
        }
        Expr *right = parse_multiplicative_expr(p);
        left = make_expr_binary(left,right,op);
        free(op);
    }
    return left;
}

Expr *parse_multiplicative_expr(Parser *p)
{
    Expr *left = parse_primary_expr(p);
    while (at(p).value && (!strcmp(at(p).value,"*") || !strcmp(at(p).value,"/")))
    {
        char *op = my_str_dup(eat(p).value);
        if (!op)
        {
            fprintf(stderr,"Memory allocation error when copying operation.");
            exit(EXIT_FAILURE);
        }
        Expr *right = parse_primary_expr(p);
        left = make_expr_binary(left,right,op);
        free(op);
    }
    return left;
}

Expr *parse_primary_expr(Parser *p)
{
    switch (at(p).kind)
    {
    case TOKENTYPE_Identifier:
        return make_expr_ident(eat(p).value);
    case TOKENTYPE_Number:
        return make_expr_numeric(atoi(eat(p).value));
    case TOKENTYPE_String:
        return make_expr_string(eat(p).value);
    case TOKENTYPE_OpenParen:
        eat(p);
        Expr *ret = parse_expr(p);
        expecterr(p,TOKENTYPE_CloseParen,"Expected ) to an (");
        return ret;
    default:
        if (!at(p).value)
        {
            fprintf(stderr,"EOF(End of file) reached while parsing, most likely cause: unended operations such as : `1 +` or `2 *` however,it can also be cause because of `x =` or `let x = `\n");
            exit(EXIT_FAILURE);
        }
        
        fprintf(stderr,"Exhaustive handling of TokenType in parse_primary_expr\n");
        exit(EXIT_FAILURE);
    }
}