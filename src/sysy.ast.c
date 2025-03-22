#include "sysy.ast.h"
struct compileUnit * compilelist = NULL;
static inline struct ast* newast(int nodetype, struct ast * astl, struct ast * astr, int val)
{
    struct ast* astNode = (struct ast*)malloc(sizeof(struct ast));
    if(!astNode)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    astNode->nodetype_ = nodetype;
    astNode->astl_ = astl;
    astNode->astr_ = astr;
    astNode->val_ = val;
    return astNode;
}

struct ast* newastNum(int value)
{
    return newast(NT_INUMBER, NULL, NULL, value);
}

struct ast* newastRet(struct ast * astl)
{
    return newast(NT_RETURN, astl, NULL, 0);
}

struct ast* newastblk(struct ast * astl, struct ast * astr)
{
    return newast(NT_BLOCK, astl, astr, 0);
}

struct fun* newFun(char* type,char* name,struct ast* param, struct ast* body)
{
    struct fun* funNode = (struct fun*)malloc(sizeof(struct fun));
    if(!funNode)  
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    funNode->type_ = type;
    funNode->name_ = name;
    funNode->param_ = param;
    funNode->body_ = body;
    funNode->ret_ = 0;
    return funNode;
}

struct compileUnit* newCompileUnit(struct fun* entry)
{
    struct compileUnit* compileUnitNode = (struct compileUnit*)malloc(sizeof(struct compileUnit));
    if(!compileUnitNode)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    compileUnitNode->entry_ = entry;
    compileUnitNode->next_ = compilelist;
    compilelist = compileUnitNode;
    return compileUnitNode;
}
static int eval_ret = 0;
int eval(struct ast* ast)
{
    int ret = 0;
    if(!ast) return 0;
    switch (ast->nodetype_)
    {
        case NT_BLOCK:
            eval(ast->astl_);
            eval(ast->astr_);
            break;
        case NT_RETURN:
            eval_ret = eval(ast->astl_);
            break;
        case NT_INUMBER:
            return ast->val_;            
            break;
    }
    return ret;
}
void dumpCompileUnit(struct compileUnit* compileUnit, char* buffer)
{
    assert(compileUnit);
    assert(buffer);
    while(compileUnit)
    {
        /*
            eg:
            fun @main(): i32 {  // main 函数的定义
            %entry:             // 入口基本块
                ret 0           // return 0
            }
        */
        char* pos = buffer; // 定义指针追踪当前位置
        pos += sprintf(pos, "fun @%s(): %s {\n", compileUnit->entry_->name_, !strcmp(compileUnit->entry_->type_ , "int") ? "i32" : "Unknown");
        pos += sprintf(pos, "%%entry:\n");
        eval_ret = 0;
        eval(compileUnit->entry_->body_);
        compileUnit->entry_->ret_ = eval_ret;
        pos += sprintf(pos, "\tret %d\n}", compileUnit->entry_->ret_);
        compileUnit = compileUnit->next_;
    }
}