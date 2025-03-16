#ifndef SYS_AST_H
#define SYS_AST_H
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
extern struct compileUnit * compilelist;

struct ast{
    int nodetype_;
    struct ast * astl_;
    struct ast * astr_;
    int val_;
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
    NT_RETURN,
    NT_FUNCTION,
    NT_BLOCK, 
};

struct ast* newastNum(int value);
struct ast* newastRet(struct ast * astl);
struct ast* newastblk(struct ast * astl, struct ast * astr);
struct fun* newFun(char* type,char* name,struct ast* param, struct ast* body);
struct compileUnit* newCompileUnit(struct fun* entry);
void dumpCompileUnit(struct compileUnit* compileUnit);
void freeCompileUnit(struct compileUnit* compileUnit);
#endif // SYS_AST_H