#ifndef SYS_AST_H
#define SYS_AST_H
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
extern struct compileUnit * compilelist;

struct ast{
    int nodetype_;
    struct ast* astl_;
    struct ast* astr_;
    char* val_;
};

struct valNode{
    int type_;
    int value_;
    struct ast* ast_;
};

struct fun{
    char* type_;
    char* name_;
    struct ast* param_;
    struct ast* body_;
    int ret_;
};

struct compileUnit{
    struct fun* entry_;
    struct compileUnit* next_;
};

enum {
    NT_INUMBER,
    NT_LVAL,
    NT_EXP_PLUS,
    NT_EXP_MINUS,
    NT_EXP_EXCLAM,
    NT_EXP_MULTIPLE,
    NT_EXP_DIVIDE,
    NT_EXP_MODIFY,
    NT_EXP_LESS,
    NT_EXP_GREATER,
    NT_EXP_LESS_EQUAL,
    NT_EXP_GREATER_EQUAL,
    NT_EXP_EQUAL,
    NT_EXP_NEQUAL,
    NT_EXP_AND,
    NT_EXP_OR,
    NT_STMT_RETURN,
    NT_STMT_ASSIGN,
    NT_DECL_CONST,
    NT_DECL_VAR,
    NT_BLOCK_ITEM,
    NT_BLOCK, 
    NT_FUNCTION,
};

struct ast* newastNum(int value);
struct ast* newastRet(struct ast * astl);
struct ast* newastUExp(char optype, struct ast * astl);
struct ast* newastBExp(char optype, struct ast * astl, struct ast * astr);
void ConstSymbolRegister(struct ast* constExp);
struct ast *newastConstDef(struct ast *astl, struct ast *astr, char *constsymbol);
struct ast* newastBlk(struct ast * astl, struct ast * astr);
struct fun* newFun(char* type,char* name,struct ast* param, struct ast* body);
struct compileUnit* newCompileUnit(struct fun* entry);
struct ast* newastLVal(char* name);
struct ast* newastVarDef(struct ast* astl, struct ast* astr, char* varName);
struct ast *newastAssign(struct ast *astl, struct ast *astr);
struct ast* newastBItem(struct ast* astl, struct ast* astr);
int Calc(struct ast *ast);
void dumpCompileUnit(struct compileUnit* compileUnit , char* buffer);
void freeCompileUnit(struct compileUnit* compileUnit);
#endif // SYS_AST_H