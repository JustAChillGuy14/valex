#ifndef INTERPRETER_H
#define INTERPRETER_H
#include "runtime/values.h"
#include "runtime/scope.h"
#include "frontend/ast.h"

RuntimeVal eval_program(Program program, Scope *scope);

RuntimeVal eval_stmt(Stmt *stmt, Scope *scope);
RuntimeVal eval_variable_declaration_stmt(VariableDeclarationStmt vds, Scope *scope);

RuntimeVal eval_expr(Expr *expr, Scope *scope);
RuntimeVal eval_binary_expr(BinaryExpr be, Scope *scope);

RuntimeVal eval_numeric_binary_expr(NumberVal left, NumberVal right, char *op);
RuntimeVal eval_string_binary_expr(StringVal left, StringVal right, char *op);
RuntimeVal eval_bool_binary_expr(BoolVal left, BoolVal right, char *op);

RuntimeVal eval_numeric_string_binary_expr(NumberVal left, StringVal right, char *op);
RuntimeVal eval_numeric_bool_expr(NumberVal left, BoolVal right, char *op);

RuntimeVal eval_assignment_expr(AssignmentExpr a, Scope *scope);
#endif