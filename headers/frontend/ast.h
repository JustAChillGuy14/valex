#ifndef AST_H
#define AST_H
#include <stddef.h>
typedef enum
{
    // Literals
    EXPR_NumericLiteral,
    EXPR_Identifier,
    // Expressions
    EXPR_BinaryExpr,
    EXPR_AssignmentExpr,
} ExprType;

struct Expr;

typedef struct
{
    int x;
} NumericLiteral;

typedef struct
{
    char *symbol;
} Identifier;

typedef struct
{
    struct Expr *left;
    struct Expr *right;
    char *op; // Dynamically allocated,big waste rn,but when != >= is added,greatness!
} BinaryExpr;

typedef struct
{
    struct Expr *assigne; // E.g: x.y = 3 => both assigne and value have to be expressions.
    struct Expr *value;
} AssignmentExpr;

struct Expr
{
    ExprType kind;
    union
    {
        NumericLiteral n;
        Identifier i;
        BinaryExpr be;
        AssignmentExpr a;
    } data;
};
typedef struct Expr Expr;

typedef enum
{
    NODE_ExprStmt,
    NODE_VariableDeclarationStmt,
} NodeType;

typedef struct
{
    char *ident;
    Expr *value;
    int isConst;
} VariableDeclarationStmt;

struct Stmt
{
    NodeType kind;
    union
    {
        Expr *e; // Expression statements.
        VariableDeclarationStmt vds;
    } data;
};
typedef struct Stmt Stmt; // One would think typedef struct{} Stmt; is valid,but Stmts can contain each other E.g: if_body,program_body,etc.

typedef struct
{
    Stmt **body;
    size_t len;
    size_t cap;
} Program;

Expr *make_expr_numeric(int n);
Expr *make_expr_ident(char *symbol);
Expr *make_expr_binary(Expr *left, Expr *right, char *op);
Expr *make_expr_assignment(Expr *assigne,Expr *value);

Stmt *make_stmt_expr_stmt(Expr *expr);
Stmt *make_stmt_var_decl_stmt(char *ident, Expr *value, int isConst);

void program_init(Program *prog);
void program_append(Program *prog, Stmt *stmt);

void free_program(Program *program);
void free_stmt(Stmt *stmt);
void free_expr(Expr *expr);

void dump_program(Program program);
void dump_stmt(Stmt *stmt, int depth);
void dump_expr(Expr *expr, int depth);
#endif