#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "frontend/ast.h"
#include "runtime/values.h"
#include "runtime/scope.h"
#include "runtime/interpreter.h"

RuntimeVal eval_program(Program prog, Scope *scope)
{
    RuntimeVal lastEvaled = runtimeval_null();
    for (size_t i = 0; i < prog.len; i++)
    {
        lastEvaled = eval_stmt(prog.body[i], scope);
    }
    return lastEvaled;
}

RuntimeVal eval_stmt(Stmt *stmt, Scope *scope)
{
    switch (stmt->kind)
    {
    case NODE_ExprStmt:
        return eval_expr(stmt->data.e, scope);
    case NODE_VariableDeclarationStmt:
        return eval_variable_declaration_stmt(stmt->data.vds, scope);
    default:
        fprintf(stderr, "Exhaustive handling of NodeType in eval_stmt.\n");
        exit(EXIT_FAILURE);
    }
}

RuntimeVal eval_variable_declaration_stmt(VariableDeclarationStmt vds, Scope *scope)
{
    return declarevar(scope, vds.ident, eval_expr(vds.value, scope), vds.isConst);
}

RuntimeVal eval_expr(Expr *expr, Scope *scope)
{
    switch (expr->kind)
    {
    case EXPR_NumericLiteral:
        return runtimeval_number(expr->data.n.x);
    case EXPR_StringLiteral:
        return runtimeval_string(expr->data.s.s);
    case EXPR_Identifier:
        return getvar(scope, expr->data.i.symbol);
    case EXPR_BinaryExpr:
        return eval_binary_expr(expr->data.be, scope);
    case EXPR_AssignmentExpr:
        return eval_assignment_expr(expr->data.a, scope);
    default:
        fprintf(stderr, "Exhaustive handling of ExprType in eval_expr.\n");
        exit(EXIT_FAILURE);
    }
}

RuntimeVal eval_binary_expr(BinaryExpr be, Scope *scope)
{
    RuntimeVal left = eval_expr(be.left, scope);
    RuntimeVal right = eval_expr(be.right, scope);

    if (left.type == VAL_Null || right.type == VAL_Null)
    {
        return runtimeval_null();
    }

    if (left.type == VAL_Number && right.type == VAL_Number)
    {
        return eval_numeric_binary_expr(left.data.n, right.data.n, be.op);
    }

    if (left.type == VAL_String && right.type == VAL_String)
    {
        RuntimeVal ret = eval_string_binary_expr(left.data.s, right.data.s, be.op);
        free_value(&left);
        free_value(&right);
        return ret;
    }

    if ((left.type == VAL_Number && right.type == VAL_String) || (left.type == VAL_String && right.type == VAL_Number))
    {
        RuntimeVal ret;
        if (left.type == VAL_Number)
        {
            ret = eval_numeric_string_binary_expr(left.data.n, right.data.s, be.op);
            free_value(&right);
        }
        else
        {
            ret = eval_numeric_string_binary_expr(right.data.n, left.data.s, be.op);
            free_value(&left);        
        }
        return ret;
    }

    fprintf(stderr, "Exhaustive handling of operand types in eval_binary_expr\n");
    exit(EXIT_FAILURE);
}

RuntimeVal eval_string_binary_expr(StringVal left, StringVal right, char *op)
{
    if (!strcmp(op, "+"))
    {
        size_t left_size = strlen(left.value);
        size_t right_size = strlen(right.value);
        char *buf = malloc(left_size + right_size + 1);
        if (!buf)
        {
            fprintf(stderr, "Memory allocation error happened during addition of string %s and string %s", right.value, left.value);
            exit(EXIT_FAILURE);
        }
        memcpy(buf, left.value, strlen(left.value));
        memcpy(buf + strlen(left.value), right.value, strlen(right.value) + 1);
        RuntimeVal ret = runtimeval_string(buf);
        free(buf);
        return ret;
    }

    fprintf(stderr, "Invalid operand operation %s for operand types \"string\" and \"string\"\n", op);
    exit(EXIT_FAILURE);
}

RuntimeVal eval_numeric_binary_expr(NumberVal left, NumberVal right, char *op)
{
    if (!strcmp(op, "+"))
    {
        return runtimeval_number(left.value + right.value);
    }
    else if (!strcmp(op, "-"))
    {
        return runtimeval_number(left.value - right.value);
    }
    else if (!strcmp(op, "*"))
    {
        return runtimeval_number(left.value * right.value);
    }
    else if (!strcmp(op, "/"))
    {
        return runtimeval_number(left.value / right.value);
    }

    fprintf(stderr, "Invalid operation %s for operand types: \"number\" and \"number\"\n", op);
    exit(EXIT_FAILURE);
}

RuntimeVal eval_numeric_string_binary_expr(NumberVal left, StringVal right, char *op)
{
    if (!strcmp(op, "*"))
    {
        if (left.value != (int)left.value)
        {
            fprintf(stderr, "Cannot multiply string with non-integer.");
            exit(EXIT_FAILURE);
        }

        // size_t i = 0;
        size_t len = strlen(right.value);
        size_t total = left.value * len;
        char *buf = malloc(total + 1);

        if (!buf)
        {
            fprintf(stderr, "Memory allocation error happened during multiplication of string %s and int %.0f", right.value, left.value);
            exit(EXIT_FAILURE);
        }

        // Copy first instance
        memcpy(buf, right.value, len);
        size_t copied = len;

        while (copied < total)
        {
            size_t to_copy = (copied > total - copied) ? total - copied : copied;
            memcpy(buf + copied, buf, to_copy);
            copied += to_copy;
        }

        buf[total] = '\0';
        RuntimeVal ret = runtimeval_string(buf);
        free(buf);
        return ret;
    }

    fprintf(stderr, "Invalid operation %s for operand types: \"number\" and \"string\"\n", op);
    exit(EXIT_FAILURE);
}

RuntimeVal eval_assignment_expr(AssignmentExpr a, Scope *scope)
{
    if (a.assigne->kind != EXPR_Identifier)
    {
        fprintf(stderr, "Cannot assign value to non-identifier.\n");
        exit(EXIT_FAILURE);
    }
    return setvar(scope, a.assigne->data.i.symbol, eval_expr(a.value, scope));
}