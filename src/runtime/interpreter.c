#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    fprintf(stderr, "Exhaustive handling of operand types in eval_binary_expr\n");
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

RuntimeVal eval_assignment_expr(AssignmentExpr a, Scope *scope)
{
    if (a.assigne->kind != EXPR_Identifier)
    {
        fprintf(stderr,"Cannot assign value to non-identifier.\n");
        exit(EXIT_FAILURE);
    }
    return setvar(scope,a.assigne->data.i.symbol,eval_expr(a.value,scope));
}