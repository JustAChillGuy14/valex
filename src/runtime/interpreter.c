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
    if (!vds.value) {
        return declarevar(scope, vds.ident, runtimeval_null(), 0); // must not be constant.
    }
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
    case EXPR_UnaryExpr:
        return eval_unary_expr(expr->data.ue, scope);
    case EXPR_BinaryExpr:
        return eval_binary_expr(expr->data.be, scope);
    case EXPR_AssignmentExpr:
        return eval_assignment_expr(expr->data.a, scope);
    default:
        fprintf(stderr, "Exhaustive handling of ExprType in eval_expr.\n");
        exit(EXIT_FAILURE);
    }
}

RuntimeVal eval_unary_expr(UnaryExpr ue, Scope *scope)
{
    RuntimeVal on = eval_expr(ue.on,scope);
    switch (ue.op)
    {
    case '!':
        switch (on.type)
        {
        case VAL_Number:
            return runtimeval_bool(!on.data.n.value);
        case VAL_Bool:
            return runtimeval_bool(!on.data.b.value);
        case VAL_Null:
            return runtimeval_bool(true);
        case VAL_String:
            return runtimeval_bool(*on.data.s.value);
        default:
            fprintf(stderr, "Exhaustive handling of ValueType in eval_unary_expr(`!`)");
            exit(EXIT_FAILURE);
        }
        break;
    case '~':
        switch (on.type)
        {
        case VAL_Number:
            if ((int)on.data.n.value != on.data.n.value)
            {
                fprintf(stderr, "Cannot perform ~ on non-integer value.");
                exit(EXIT_FAILURE);
            }
            return runtimeval_number(~(int)(on.data.n.value));
        case VAL_Bool:
            return runtimeval_bool(!on.data.b.value); // Bitwise not is just logical not for booleans.
        case VAL_Null:
            fprintf(stderr, "Cannot perform ~ on null value.");
            exit(EXIT_FAILURE);
        case VAL_String:
            fprintf(stderr, "Cannot perform ~ on string value.");
            exit(EXIT_FAILURE);
        default:
            fprintf(stderr, "Exhaustive handling of ValueType in eval_unary_expr(`~`)");
            exit(EXIT_FAILURE);
        }
        break;
    case '-':
        switch (on.type)
        {
        case VAL_Number:
            return runtimeval_number(-on.data.n.value);
        case VAL_Bool:
            return runtimeval_bool(-on.data.b.value); // Bitwise not is just logical not for booleans.
        case VAL_Null:
            fprintf(stderr, "Cannot perform - on null value.");
            exit(EXIT_FAILURE);
        case VAL_String:
            fprintf(stderr, "Cannot perform - on string value.");
            exit(EXIT_FAILURE);
        default:
            fprintf(stderr, "Exhaustive handling of ValueType in eval_unary_expr(`-`)");
            exit(EXIT_FAILURE);
        }
        break;
    default:
        fprintf(stderr, "Exhaustive handling of UnaryOperator in eval_unary_expr");
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

    if (left.type == VAL_Bool && right.type == VAL_Bool)
    {
        return eval_bool_binary_expr(left.data.b, right.data.b, be.op);
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

    if ((left.type == VAL_Bool && right.type == VAL_Number) || (left.type == VAL_Number && right.type == VAL_Bool))
    {
        return left.type == VAL_Number ? eval_numeric_bool_expr(left.data.n,right.data.b, be.op) : eval_numeric_bool_expr(right.data.n,left.data.b, be.op);
    }

    if (left.type != right.type && !strcmp(be.op, "=="))
    {
        if (left.type != right.type)
        {
            return runtimeval_bool(false);
        }
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
    else if (!strcmp(op, "=="))
    {
        return runtimeval_bool(left.value == right.value);
    }
    else if (!strcmp(op, ">="))
    {
        return runtimeval_bool(left.value >= right.value);
    }
    else if (!strcmp(op, "<="))
    {
        return runtimeval_bool(left.value <= right.value);
    }
    else if (!strcmp(op, ">"))
    {
        return runtimeval_bool(left.value > right.value);
    }
    else if (!strcmp(op, "<"))
    {
        return runtimeval_bool(left.value < right.value);
    }

    fprintf(stderr, "Invalid operation %s for operand types: \"number\" and \"number\"\n", op);
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
    else if (!strcmp(op, "=="))
    {
        return runtimeval_bool(!strcmp(left.value,right.value));
    }

    fprintf(stderr, "Invalid operand operation %s for operand types \"string\" and \"string\"\n", op);
    exit(EXIT_FAILURE);
}

RuntimeVal eval_bool_binary_expr(BoolVal left, BoolVal right, char *op)
{
    if (!strcmp(op, "=="))
    {
        return runtimeval_bool(left.value == right.value);
    }
    else if (!strcmp(op, "!="))
    {
        return runtimeval_bool(left.value != right.value);
    }

    fprintf(stderr, "Invalid operand operation %s for operand types \"bool\" and \"bool\"\n", op);
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

RuntimeVal eval_numeric_bool_expr(NumberVal left, BoolVal right, char *op)
{
    if (!strcmp(op, "=="))
    {
        return runtimeval_bool(right.value ? left.value : !left.value);
    }
    else if (!strcmp(op, "<"))
    {
        return runtimeval_bool(right.value < left.value);
    }
    else if (!strcmp(op, ">"))
    {
        return runtimeval_bool(right.value > left.value);
    }
    else if (!strcmp(op, "<="))
    {
        return runtimeval_bool(right.value <= left.value);
    }
    else if (!strcmp(op, ">="))
    {
        return runtimeval_bool(right.value >= left.value);
    }

    fprintf(stderr, "Invalid operation %s for operand types: \"number\" and \"bool\"\n", op);
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