#include "sysy.ast.h"
struct compileUnit * compilelist = NULL;
static inline struct ast* newast(int nodetype, struct ast * astl, struct ast * astr, char* val)
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
static inline char* Reg2Str(int reg)
{
    int len = snprintf(NULL, 0, "%%%d", reg) + 1; // +1 给终止符'\0'
    char *str = malloc(len);
    snprintf(str, len, "%%%d", reg);
    return str;
}
static inline char* itoa(int num)
{
    int len = snprintf(NULL, 0, "%d", num) + 1; // +1 给终止符'\0'
    char *str = malloc(len);
    snprintf(str, len, "%d", num);
    return str;
}
struct ast* newastNum(int value)
{
    char* str = itoa(value);
    return newast(NT_INUMBER, NULL, NULL, str);
}

struct ast* newastRet(struct ast * astl)
{
    return newast(NT_STMT_RETURN, astl, NULL, NULL);
}
struct ast* newastUExp(char optype, struct ast * astl)
{
    switch (optype)
    {
        case '+':
            return newast(NT_EXP_PLUS, astl, NULL, NULL);
            break;
        case '-':
            return newast(NT_EXP_MINUS, astl, NULL, NULL);
            break;
        case '!':
            return newast(NT_EXP_EXCLAM, astl, NULL, NULL);
            break;
        default:
            perror("optype error");
            return NULL;
            break;
    }
    return NULL;
}
struct ast* newastBExp(char optype, struct ast * astl, struct ast * astr)
{
    switch(optype)
    {
        case '*':
        case '/':
        case '%':
            return newast(optype == '*' ? NT_EXP_MULTIPLE : optype == '/' ? NT_EXP_DIVIDE : NT_EXP_MODIFY, astl, astr, NULL);
        
        case '+':
        case '-':
            return newast(optype == '+' ? NT_EXP_PLUS : NT_EXP_MINUS, astl, astr, NULL);

        case '>':
        case '<':
        case 'L':
        case 'G':
            return newast(optype == '>' ? NT_EXP_GREATER : optype == '<' ? NT_EXP_LESS : optype == 'L' ? NT_EXP_LESS_EQUAL : NT_EXP_GREATER_EQUAL, astl, astr, NULL);
        case 'E':
        case 'N':
            return newast(optype == 'E' ? NT_EXP_EQUAL : NT_EXP_NEQUAL, astl, astr, NULL);

        case 'A':
            return newast(NT_EXP_AND, astl, astr, NULL);
        case 'O':
            return newast(NT_EXP_OR, astl, astr, NULL);

        default:
            perror("optype error");
            return NULL;
            break;
    }
    return NULL;
}

struct ast* newastblk(struct ast * astl, struct ast * astr)
{
    return newast(NT_BLOCK, astl, astr, NULL);
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
static int Reg = 0;
int parse(struct ast* ast , char* buffer)
{
    if(!ast) return 0;
    int len = 0;
    switch (ast->nodetype_)
    {
        case NT_BLOCK:
            len += parse(ast->astl_,buffer+len);
            len += parse(ast->astr_,buffer+len);
            break;
        case NT_STMT_RETURN:
            len += parse(ast->astl_,buffer+len);
            len += sprintf(buffer+len,"\tret %s\n}",ast->astl_->val_);
            break;
        case NT_EXP_PLUS:
            len += parse(ast->astl_,buffer+len);
            len += parse(ast->astr_,buffer+len);
            ast->val_ = ast->astl_->val_;
            if(ast->astr_)
            {
                ast->val_ = Reg2Str(Reg++);
                len += sprintf(buffer+len, "\t%s = add %s, %s\n", ast->val_, ast->astl_->val_, ast->astr_->val_);
            }
            break;
        case NT_EXP_MINUS:
            len += parse(ast->astl_,buffer+len);
            len += parse(ast->astr_,buffer+len);
            ast->val_ = Reg2Str(Reg++);
            if(ast->astr_)
            {
                len += sprintf(buffer+len, "\t%s = sub %s, %s\n", ast->val_, ast->astl_->val_, ast->astr_->val_);
            }
            else
            {
                len += sprintf(buffer+len, "\t%s = sub 0, %s\n", ast->val_, ast->astl_->val_);
            }
            break;
        case NT_EXP_EXCLAM:
            len += parse(ast->astl_,buffer+len);
            ast->val_ = Reg2Str(Reg++);
            len += sprintf(buffer+len,"\t%s = eq %s, 0\n",ast->val_ , ast->astl_->val_);
            break;
        case NT_EXP_MULTIPLE:
            len += parse(ast->astl_,buffer+len);
            len += parse(ast->astr_,buffer+len);
            ast->val_ = Reg2Str(Reg++);
            len += sprintf(buffer+len, "\t%s = mul %s, %s\n", ast->val_, ast->astl_->val_, ast->astr_->val_);
            break;
        case NT_EXP_DIVIDE:
            len += parse(ast->astl_,buffer+len);
            len += parse(ast->astr_,buffer+len);
            ast->val_ = Reg2Str(Reg++);
            len += sprintf(buffer+len, "\t%s = div %s, %s\n", ast->val_, ast->astl_->val_, ast->astr_->val_);
            break;
        case NT_EXP_MODIFY:
            len += parse(ast->astl_,buffer+len);
            len += parse(ast->astr_,buffer+len);
            ast->val_ = Reg2Str(Reg++);
            len += sprintf(buffer+len, "\t%s = mod %s, %s\n", ast->val_, ast->astl_->val_, ast->astr_->val_);
            break;

        case NT_EXP_LESS:
            len += parse(ast->astl_,buffer+len);
            len += parse(ast->astr_,buffer+len);
            ast->val_ = Reg2Str(Reg++);
            len += sprintf(buffer+len, "\t%s = lt %s, %s\n", ast->val_, ast->astl_->val_, ast->astr_->val_);
            break;
        case NT_EXP_GREATER:
            len += parse(ast->astl_,buffer+len);
            len += parse(ast->astr_,buffer+len);
            ast->val_ = Reg2Str(Reg++);
            len += sprintf(buffer+len, "\t%s = gt %s, %s\n", ast->val_, ast->astl_->val_, ast->astr_->val_);
            break;
        
        case NT_EXP_LESS_EQUAL:
            len += parse(ast->astl_,buffer+len);
            len += parse(ast->astr_,buffer+len);
            ast->val_ = Reg2Str(Reg++);
            len += sprintf(buffer+len, "\t%s = le %s, %s\n", ast->val_, ast->astl_->val_, ast->astr_->val_);
            break;
        case NT_EXP_GREATER_EQUAL:
            len += parse(ast->astl_,buffer+len);
            len += parse(ast->astr_,buffer+len);
            ast->val_ = Reg2Str(Reg++);
            len += sprintf(buffer+len, "\t%s = ge %s, %s\n", ast->val_, ast->astl_->val_, ast->astr_->val_);
            break;
        
        case NT_EXP_EQUAL:
            len += parse(ast->astl_,buffer+len);
            len += parse(ast->astr_,buffer+len);
            ast->val_ = Reg2Str(Reg++);
            len += sprintf(buffer+len, "\t%s = eq %s, %s\n", ast->val_, ast->astl_->val_, ast->astr_->val_);
            break;
        case NT_EXP_NEQUAL:
            len += parse(ast->astl_,buffer+len);
            len += parse(ast->astr_,buffer+len);
            ast->val_ = Reg2Str(Reg++);
            len += sprintf(buffer+len, "\t%s = ne %s, %s\n", ast->val_, ast->astl_->val_, ast->astr_->val_);
            break;
    
        // Koopa IR 只支持按位与或, 而不支持逻辑与或, 但你可以用其他运算拼凑出这些运算
        case NT_EXP_AND:
            len += parse(ast->astl_,buffer+len);
            len += parse(ast->astr_,buffer+len);
            // 先分别将左右操作数与0比较以获取布尔值
            char* left_bool = Reg2Str(Reg++);
            len += sprintf(buffer+len, "\t%s = ne %s, 0\n", left_bool, ast->astl_->val_);
            char* right_bool = Reg2Str(Reg++);
            len += sprintf(buffer+len, "\t%s = ne %s, 0\n", right_bool, ast->astr_->val_);
            // 再进行按位与操作
            ast->val_ = Reg2Str(Reg++);
            len += sprintf(buffer+len, "\t%s = and %s, %s\n", ast->val_, left_bool, right_bool);
            break;

        case NT_EXP_OR:
            len += parse(ast->astl_,buffer+len);
            len += parse(ast->astr_,buffer+len);
            // 先分别将左右操作数与0比较以获取布尔值
            char* left_bool_or = Reg2Str(Reg++);
            len += sprintf(buffer+len, "\t%s = ne %s, 0\n", left_bool_or, ast->astl_->val_);
            char* right_bool_or = Reg2Str(Reg++);
            len += sprintf(buffer+len, "\t%s = ne %s, 0\n", right_bool_or, ast->astr_->val_);
            // 再进行按位或操作
            ast->val_ = Reg2Str(Reg++);
            len += sprintf(buffer+len, "\t%s = or %s, %s\n", ast->val_, left_bool_or, right_bool_or);
            break;

        case NT_INUMBER:
            break;
    }
    return len;
}
void dumpCompileUnit(struct compileUnit* compileUnit, char* buffer)
{
    assert(compileUnit);
    assert(buffer);
    char* pos = buffer; // 定义指针追踪当前位置
    while(compileUnit)
    {
        /*
            eg:
            fun @main(): i32 {  // main 函数的定义
            %entry:             // 入口基本块
                ret 0           // return 0
            }
        */
        pos += sprintf(pos, "fun @%s(): %s {\n", compileUnit->entry_->name_, !strcmp(compileUnit->entry_->type_ , "int") ? "i32" : "Unknown");
        pos += sprintf(pos, "%%entry:\n");
        Reg = 0;
        pos += parse(compileUnit->entry_->body_, pos);
        compileUnit = compileUnit->next_;
    }
}

void freeCompileUnit(struct compileUnit* compileUnit)
{
    while(compileUnit)
    {
        // to do
    }
}