#include <stdlib.h>
#include <stdio.h>
#include "frontend/ast.h"
#include "frontend/lexer.h"

Expr *make_expr_numeric(int n)
{
    Expr *ret = malloc(sizeof(Expr));
    if (!ret)
    {
        fprintf(stderr, "Memory allocation error. Happened during allocation of Expr on the heap.\n");
        exit(EXIT_FAILURE);
    }

    ret->data.n.x = n;
    ret->kind = EXPR_NumericLiteral;
    return ret;
}

Expr *make_expr_string(char *s)
{
    char *copied = my_str_dup(s);
    if (!copied)
    {
        fprintf(stderr, "Memory allocation error. Happened during making of a StringLiteral.\n");
        exit(EXIT_FAILURE);
    }
    Expr *ret = malloc(sizeof(Expr));
    if (!ret)
    {
        fprintf(stderr, "Memory allocation error. Happened during allocation of Expr on the heap.\n");
        exit(EXIT_FAILURE);
    }
    ret->data.s.s = copied;
    ret->kind = EXPR_StringLiteral;
    return ret;
}

Expr *make_expr_ident(char *c)
{
    char *copied = my_str_dup(c);
    if (!copied)
    {
        fprintf(stderr, "Memory allocation error. Happened during making of an Identifier.\n");
        exit(EXIT_FAILURE);
    }
    Expr *ret = malloc(sizeof(Expr));
    if (!ret)
    {
        fprintf(stderr, "Memory allocation error. Happened during allocation of Expr on the heap.\n");
        exit(EXIT_FAILURE);
    }
    ret->data.i.symbol = copied;
    ret->kind = EXPR_Identifier;
    return ret;
}

Expr *make_expr_unary(Expr *on, char op)
{
    Expr *ret = malloc(sizeof(Expr));
    if (!ret)
    {
        fprintf(stderr, "Memory allocation error. Happened during allocation of Expr on the heap.\n");
        exit(EXIT_FAILURE);
    }
    ret->data.ue.on = on;
    ret->data.ue.op = op;
    ret->kind = EXPR_UnaryExpr;
    return ret;
}

Expr *make_expr_binary(Expr *left, Expr *right, char *op)
{
    Expr *ret = malloc(sizeof(Expr));
    if (!ret)
    {
        fprintf(stderr, "Memory allocation error. Happened during allocation of Expr on the heap.\n");
        exit(EXIT_FAILURE);
    }
    ret->data.be.left = left;
    ret->data.be.right = right;
    ret->data.be.op = my_str_dup(op); // Ik ik,so much duping,but we own everything,no views.
    if (!ret->data.be.op)
    {
        fprintf(stderr, "Memory allocation error. Happened during duplication of operation in BinaryExpr.\n");
        exit(EXIT_FAILURE);
    }
    ret->kind = EXPR_BinaryExpr;
    return ret;
}

Expr *make_expr_assignment(Expr *assigne, Expr *value)
{
    Expr *ret = malloc(sizeof(Expr));
    if (!ret)
    {
        fprintf(stderr, "Memory allocation error. Happened during allocation of Expr on the heap.\n");
        exit(EXIT_FAILURE);
    }
    ret->data.a.assigne = assigne;
    ret->data.a.value = value;
    ret->kind = EXPR_AssignmentExpr;
    return ret;
}

Stmt *make_stmt_expr_stmt(Expr *expr)
{
    Stmt *ret = malloc(sizeof(Stmt));
    if (!ret)
    {
        fprintf(stderr, "Memory allocation error. Happened during allocating memory for a stmt on the heap.\n");
        exit(EXIT_FAILURE);
    }
    ret->data.e = expr;
    ret->kind = NODE_ExprStmt;
    return ret;
}

Stmt *make_stmt_var_decl_stmt(char *ident,Expr *value, int isConst){
    Stmt *ret = malloc(sizeof(Stmt));
    if (!ret)
    {
        fprintf(stderr, "Memory allocation error. Happened during allocating memory for a stmt on the heap.\n");
        exit(EXIT_FAILURE);
    }
    ret->data.vds.ident = my_str_dup(ident);
    if (!ret->data.vds.ident)
    {
        fprintf(stderr,"Memory allocation error. Happened during duplication of identifier in variable declaration.\n");
        exit(EXIT_FAILURE);
    }
    ret->data.vds.value = value;
    ret->data.vds.isConst = isConst;
    ret->kind = NODE_VariableDeclarationStmt;
    return ret;
}

void program_init(Program *prog)
{
    prog->cap = 1024;
    prog->len = 0;
    prog->body = malloc(sizeof(Stmt *) * prog->cap);
    if (!prog->body)
    {
        fprintf(stderr, "Memory allocation error. Happened during initialization of program.\n");
        exit(EXIT_FAILURE);
    }
}

void program_append(Program *prog, Stmt *stmt)
{
    if (prog->len == prog->cap)
    {
        prog->cap += 1024;
        Stmt **tmp = realloc(prog->body, sizeof(Stmt *) * prog->cap);
        if (!tmp)
        {
            fprintf(stderr, "Memory reallocation error. Happened during appending statement to program.\n");
            exit(EXIT_FAILURE);
        }
        prog->body = tmp;
    }
    prog->body[prog->len++] = stmt;
}

void free_expr(Expr *expr)
{
    if (!expr)
        return;

    switch (expr->kind)
    {
    case EXPR_NumericLiteral:
        break;
    case EXPR_Identifier:
        free(expr->data.i.symbol);
        break;
    case EXPR_StringLiteral:
        free(expr->data.s.s);
        break;
    case EXPR_UnaryExpr:
        free(expr->data.ue.on);
        break;
    case EXPR_BinaryExpr:
        free_expr(expr->data.be.left);
        free_expr(expr->data.be.right);
        free(expr->data.be.op);
        break;
    case EXPR_AssignmentExpr:
        free_expr(expr->data.a.assigne);
        free_expr(expr->data.a.value);
        break;
    default:
        fprintf(stderr, "Exhaustive handling of expression types in free_expr.\n");
        exit(EXIT_FAILURE);
    }
    free(expr); // Gotta free the pointer itself...
}

void free_stmt(Stmt *stmt)
{
    if (!stmt)
        return;

    switch (stmt->kind)
    {
    case NODE_ExprStmt:
        free_expr(stmt->data.e);
        break;
    case NODE_VariableDeclarationStmt:
        free(stmt->data.vds.ident);
        free_expr(stmt->data.vds.value);
        break;
    default:
        fprintf(stderr, "Exhaustive handling of NodeType in free_stmt.\n");
        exit(EXIT_FAILURE);
    }
    free(stmt);
}

void free_program(Program *prog)
{
    for (size_t i = 0; i < prog->len; i++)
    {
        free_stmt(prog->body[i]);
    }
    free(prog->body);
}

static void indent(int depth)
{
    for (int i = 0; i < depth; i++)
        printf("  ");
}

static const char *expr_kind_str(ExprType kind)
{
    switch (kind)
    {
    case EXPR_NumericLiteral:
        return "NumericLiteral";
    case EXPR_Identifier:
        return "Identifier";
    case EXPR_StringLiteral:
        return "StringLiteral";
    case EXPR_BinaryExpr:
        return "BinaryExpr";
    case EXPR_AssignmentExpr:
        return "AssignmentExpr";
    default:
        fprintf(stderr, "Unknown ExprType in expr_kind_str\n");
        exit(EXIT_FAILURE);
    }
}

static const char *stmt_kind_str(NodeType kind)
{
    switch (kind)
    {
    case NODE_ExprStmt:
        return "ExprStmt";
    case NODE_VariableDeclarationStmt:
        return "VariableDeclarationStmt";
    default:
        fprintf(stderr, "Unknown NodeType in stmt_kind_str\n");
        exit(EXIT_FAILURE);
    }
}

/* ---------- expression dump ---------- */

void dump_expr(Expr *expr, int depth)
{
    if (!expr)
    {
        indent(depth);
        printf("null");
        return;
    }

    indent(depth);
    printf("{\n");

    indent(depth + 1);
    printf("\"kind\": \"%s\"", expr_kind_str(expr->kind));

    switch (expr->kind)
    {
    case EXPR_NumericLiteral:
        printf(",\n");
        indent(depth + 1);
        printf("\"value\": %d\n", expr->data.n.x);
        break;

    case EXPR_Identifier:
        printf(",\n");
        indent(depth + 1);
        printf("\"symbol\": \"%s\"\n", expr->data.i.symbol);
        break;
    
    case EXPR_StringLiteral:
        printf(",\n");
        indent(depth + 1);
        printf("\"string\": \"%s\"\n", expr->data.s.s);
        break;

    case EXPR_UnaryExpr:
        printf(",\n");

        indent(depth + 1);
        printf("\"op\": \"%c\",\n", expr->data.ue.op);

        indent(depth + 1);
        printf("\"on\": ");
        dump_expr(expr->data.ue.on, depth + 1);
        printf("\n");
        break;
    
    case EXPR_BinaryExpr:
        printf(",\n");

        indent(depth + 1);
        printf("\"op\": \"%s\",\n", expr->data.be.op);

        indent(depth + 1);
        printf("\"left\": ");
        dump_expr(expr->data.be.left, depth + 1);
        printf(",\n");

        indent(depth + 1);
        printf("\"right\": ");
        dump_expr(expr->data.be.right, depth + 1);
        printf("\n");
        break;
    case EXPR_AssignmentExpr:
        printf(",\n");

        indent(depth + 1);
        printf("\"assigne\": ");
        dump_expr(expr->data.a.assigne, depth + 1);
        printf(",\n");

        indent(depth + 1);
        printf("\"value\": ");
        dump_expr(expr->data.a.value, depth + 1);
        printf("\n");
        break;
    default:
        fprintf(stderr, "Exhaustive handling of ExprType in dump_expr\n");
        exit(EXIT_FAILURE);
    }

    indent(depth);
    printf("}");
}

/* ---------- statement dump ---------- */

void dump_stmt(Stmt *stmt, int depth)
{
    if (!stmt)
    {
        indent(depth);
        printf("null");
        return;
    }

    indent(depth);
    printf("{\n");

    indent(depth + 1);
    printf("\"kind\": \"%s\"", stmt_kind_str(stmt->kind));

    switch (stmt->kind)
    {
    case NODE_ExprStmt:
        printf(",\n");
        indent(depth + 1);
        printf("\"expr\": ");
        dump_expr(stmt->data.e, depth + 1);
        printf("\n");
        break;
    case NODE_VariableDeclarationStmt:
        printf(",\n");
        indent(depth + 1);
        printf("\"ident\": %s",stmt->data.vds.ident);
        printf(",\n");
        printf("\"value\": ");
        dump_expr(stmt->data.vds.value, depth + 1);
        printf("\n");
        break;
    default:
        fprintf(stderr, "Exhaustive handling of NodeType in dump_stmt\n");
        exit(EXIT_FAILURE);
    }

    indent(depth);
    printf("}");
}

void dump_program(Program program)
{
    printf("\"body\": [\n");
    for (size_t i = 0; i < program.len; i++)
    {
        dump_stmt(program.body[i], 2);
        if (i + 1 < program.len)
            printf(",");
        printf("\n");
    }

    indent(1);
    printf("]\n");
}