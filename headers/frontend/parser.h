#ifndef PARSER_H
#define PARSER_H
#include "frontend/lexer.h"
#include "frontend/ast.h"
typedef struct
{
    Token *tokens;
    size_t i;
} Parser;

void parser_init(Parser *p, Token *tokens);

Token at(Parser *p);
Token eat(Parser *p);
Token expect(Parser *p, TokenType type);
Token expecterr(Parser *p, TokenType type, const char *err);

Program parse_src(const char *src, Parser *p); // For simplicity.

Program parse_program(Parser *p);

Stmt *parse_stmt(Parser *p);
Stmt *parse_var_decl_stmt(Parser *p);

Expr *parse_expr(Parser *p);
Expr *parse_assignment_expr(Parser *p);
Expr *parse_comparision_expr(Parser *p);
Expr *parse_additive_expr(Parser *p);
Expr *parse_multiplicative_expr(Parser *p);
Expr *parse_unary_expr(Parser *p);
Expr *parse_primary_expr(Parser *p);
#endif