#include "sysy.ast.h"
#include "sysy.map.h"
struct compileUnit *compilelist = NULL;
static bool ComPareFunc(void *str1, void *str2)
{
    return !strcmp((char *)str1, (char *)str2);
}
static char *bufferCursor = NULL;
static int Reg = 0;
static size_t HashCalc(void *key)
{
    char *str = (char *)key;
    size_t hash = 0;
    while (*str)
    {
        hash = hash * 131 + *str++; // 单目运算法 右结合
    }
    return hash;
}
static HashMap *symbolTable = NULL;
bool SymbolTableContain(char *symbol)
{
    if (!symbolTable)
    {
        symbolTable = map_create(ComPareFunc, HashCalc);
    }
    return map_contains(symbolTable, symbol);
}
void SymbolTableRegister(char *symbol, struct valNode *valNode)
{
    if (!symbolTable)
    {
        symbolTable = map_create(ComPareFunc, HashCalc);
    }
    char *sym = malloc(sizeof(char) * (strlen(symbol) + 1));
    strcpy(sym, symbol);
    map_put(symbolTable, (void *)sym, (void *)valNode);
}
struct valNode *SymbolTableGet(char *symbol)
{
    if (!symbolTable)
    {
        perror("symbolTable is NULL");
        exit(EXIT_FAILURE);
    }
    return (struct valNode *)map_get(symbolTable, symbol);
}
static inline struct ast *newast(int nodetype, struct ast *astl, struct ast *astr, char *val)
{
    struct ast *astNode = (struct ast *)malloc(sizeof(struct ast));
    if (!astNode)
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
static inline char *Reg2Str(int reg)
{
    int len = snprintf(NULL, 0, "%%%d", reg) + 1; // +1 给终止符'\0'
    char *str = malloc(len);
    snprintf(str, len, "%%%d", reg);
    return str;
}
static inline char *itoa(int num)
{
    int len = snprintf(NULL, 0, "%d", num) + 1; // +1 给终止符'\0'
    char *str = malloc(len);
    snprintf(str, len, "%d", num);
    return str;
}
struct ast *newastNum(int value)
{
    char *str = itoa(value);
    return newast(NT_INUMBER, NULL, NULL, str);
}

struct ast *newastRet(struct ast *astl)
{
    return newast(NT_STMT_RETURN, astl, NULL, NULL);
}
struct ast *newastUExp(char optype, struct ast *astl)
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
struct ast *newastBExp(char optype, struct ast *astl, struct ast *astr)
{
    switch (optype)
    {
    case '*':
    case '/':
    case '%':
        return newast(optype == '*' ? NT_EXP_MULTIPLE : optype == '/' ? NT_EXP_DIVIDE
                                                                      : NT_EXP_MODIFY,
                      astl, astr, NULL);

    case '+':
    case '-':
        return newast(optype == '+' ? NT_EXP_PLUS : NT_EXP_MINUS, astl, astr, NULL);

    case '>':
    case '<':
    case 'L':
    case 'G':
        return newast(optype == '>' ? NT_EXP_GREATER : optype == '<' ? NT_EXP_LESS
                                                   : optype == 'L'   ? NT_EXP_LESS_EQUAL
                                                                     : NT_EXP_GREATER_EQUAL,
                      astl, astr, NULL);
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
void ConstSymbolRegister(struct ast *constExp)
{
    // 计算常量表达式的值
    if(SymbolTableContain(constExp->val_)) 
    {
        fprintf(stderr,"%s:got inited twice\n",constExp->val_);
        exit(EXIT_FAILURE);
    }
    int ret = Calc(constExp->astl_);
    // 注册符号
    struct valNode *valNode = (struct valNode *)malloc(sizeof(struct valNode));
    if (!valNode)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    valNode->type_ = 1;
    valNode->value_ = ret;
    SymbolTableRegister(constExp->val_, valNode);
}
struct ast *newastConstDef(struct ast *astl, struct ast *astr, char *constsymbol)
{
    return newast(NT_DECL_CONST, astl, astr, constsymbol);
}
struct ast *newastVarDef(struct ast *astl, struct ast *astr, char *varName)
{
    return newast(NT_DECL_VAR, astl, astr, varName);
}

struct ast *newastAssign(struct ast *astl, struct ast *astr)
{
    return newast(NT_STMT_ASSIGN, astl, astr, NULL);
}

struct ast *newastBItem(struct ast *astl, struct ast *astr)
{
    return newast(NT_BLOCK_ITEM, astl, astr, NULL);
}

struct ast *newastBlk(struct ast *astl, struct ast *astr)
{
    return newast(NT_BLOCK, astl, astr, NULL);
}

struct fun *newFun(char *type, char *name, struct ast *param, struct ast *body)
{
    struct fun *funNode = (struct fun *)malloc(sizeof(struct fun));
    if (!funNode)
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

struct compileUnit *newCompileUnit(struct fun *entry)
{
    struct compileUnit *compileUnitNode = (struct compileUnit *)malloc(sizeof(struct compileUnit));
    if (!compileUnitNode)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    compileUnitNode->entry_ = entry;
    compileUnitNode->next_ = compilelist;
    compilelist = compileUnitNode;
    return compileUnitNode;
}
int Calc(struct ast *ast)
{
    int ret = 0;
    struct valNode *val;

    switch (ast->nodetype_)
    {
    case NT_EXP_PLUS:
        ret += Calc(ast->astl_);
        if (ast->astr_)
            ret += Calc(ast->astr_);
        break;
    case NT_EXP_MINUS:
        ret += Calc(ast->astl_);
        if (ast->astr_)
            ret -= Calc(ast->astr_);
        break;
    case NT_EXP_EXCLAM:
        ret += !Calc(ast->astl_);
        break;
    case NT_EXP_MULTIPLE:
        ret = Calc(ast->astl_) * Calc(ast->astr_);
        break;
    case NT_EXP_DIVIDE:
        ret = Calc(ast->astl_) / Calc(ast->astr_);
        break;
    case NT_EXP_MODIFY:
        ret = Calc(ast->astl_) % Calc(ast->astr_);
        break;

    case NT_EXP_LESS:
        ret = Calc(ast->astl_) < Calc(ast->astr_);
        break;
    case NT_EXP_GREATER:
        ret = Calc(ast->astl_) > Calc(ast->astr_);
        break;

    case NT_EXP_LESS_EQUAL:
        ret = Calc(ast->astl_) <= Calc(ast->astr_);
        break;
    case NT_EXP_GREATER_EQUAL:
        ret = Calc(ast->astl_) >= Calc(ast->astr_);
        break;

    case NT_EXP_EQUAL:
        ret = Calc(ast->astl_) == Calc(ast->astr_);
        break;
    case NT_EXP_NEQUAL:
        ret = Calc(ast->astl_) != Calc(ast->astr_);
        break;
    case NT_EXP_AND:
        ret = Calc(ast->astl_) && Calc(ast->astr_);
        break;
    case NT_EXP_OR:
        ret = Calc(ast->astl_) || Calc(ast->astr_);
        break;
    case NT_INUMBER:
        ret = atoi(ast->val_);
        break;
    case NT_LVAL:
    {
        val = SymbolTableGet(ast->val_);
        assert(val != NULL);
        if (val->type_ == 0)
        {
            // 直接计算 变量的值
            // ast->astl_== NULL 成立说明此时仅声明
            if (val->ast_ == NULL)
            {
                perror("variable used to init const but not init!!!");
                exit(EXIT_FAILURE);
            };
            ret = Calc(val->ast_);
        }
        else
        {
            assert(val->type_ == 1);
            ret = val->value_;
        }
        break;
    }
    }
    return ret;
}
void parse(struct ast *ast)
{
    if (!ast)
        return;

    struct valNode *val;
    char *left_bool, *right_bool, *left_bool_or, *right_bool_or;
    char *symbol;

    switch (ast->nodetype_)
    {
    case NT_BLOCK:
        parse(ast->astl_);
        parse(ast->astr_);
        break;
    case NT_BLOCK_ITEM:
        parse(ast->astl_);
        if (ast->astr_)
            parse(ast->astr_);
        break;
    case NT_DECL_CONST:
        ConstSymbolRegister(ast); // 注册常量
        if (ast->astr_)
            parse(ast->astr_);
        break;
    case NT_DECL_VAR:
        bufferCursor += sprintf(bufferCursor, "\t@%s = alloc i32\n", ast->val_);
        struct valNode *valNode = (struct valNode *)malloc(sizeof(struct valNode));
        if (!valNode)
        {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        valNode->type_ = 0;
        valNode->value_ = 0;
        valNode->ast_ = ast->astl_;
        SymbolTableRegister(ast->val_, valNode);

        if (ast->astl_)
        {
            parse(ast->astl_);
            bufferCursor += sprintf(bufferCursor, "\tstore %s, @%s\n", ast->astl_->val_, ast->val_);
        }

        if (ast->astr_)
            parse(ast->astr_);
        break;
    case NT_STMT_RETURN:
        if (ast->astl_)
        {
            parse(ast->astl_);
            bufferCursor += sprintf(bufferCursor, "\tret %s\n}", ast->astl_->val_);
        }
        else
        {
            bufferCursor += sprintf(bufferCursor, "\tret 0\n}"); // 默认返回0
        }
        break;
    case NT_STMT_ASSIGN:
        assert(ast->astl_ != NULL);
        assert(ast->astr_ != NULL);
        parse(ast->astr_);
        val = SymbolTableGet(ast->astl_->val_);
        if(val== NULL){
            val = (struct valNode *)malloc(sizeof(struct valNode));
            if (!val)
            {
                perror("malloc");
                exit(EXIT_FAILURE);
            }
            val->type_ = 0;
            val->value_ = 0;
            SymbolTableRegister(ast->astl_->val_, val);
        }
        assert(val->type_ == 0);
        //变量就更新符号表中的值
        val->ast_ = ast->astr_;
        bufferCursor += sprintf(bufferCursor, "\tstore %s, @%s\n", ast->astr_->val_, ast->astl_->val_);
        break;
    case NT_EXP_PLUS:
        parse(ast->astl_);
        parse(ast->astr_);
        ast->val_ = ast->astl_->val_;
        if (ast->astr_)
        {
            ast->val_ = Reg2Str(Reg++);
            bufferCursor += sprintf(bufferCursor, "\t%s = add %s, %s\n", ast->val_, ast->astl_->val_, ast->astr_->val_);
        }
        break;
    case NT_EXP_MINUS:
        parse(ast->astl_);
        parse(ast->astr_);
        ast->val_ = Reg2Str(Reg++);
        if (ast->astr_)
        {
            bufferCursor += sprintf(bufferCursor, "\t%s = sub %s, %s\n", ast->val_, ast->astl_->val_, ast->astr_->val_);
        }
        else
        {
            bufferCursor += sprintf(bufferCursor, "\t%s = sub 0, %s\n", ast->val_, ast->astl_->val_);
        }
        break;
    case NT_EXP_EXCLAM:
        parse(ast->astl_);
        assert(ast->astr_ == NULL);
        ast->val_ = Reg2Str(Reg++);
        bufferCursor += sprintf(bufferCursor, "\t%s = eq %s, 0\n", ast->val_, ast->astl_->val_);
        break;
    case NT_EXP_MULTIPLE:
        parse(ast->astl_);
        parse(ast->astr_);
        ast->val_ = Reg2Str(Reg++);
        bufferCursor += sprintf(bufferCursor, "\t%s = mul %s, %s\n", ast->val_, ast->astl_->val_, ast->astr_->val_);
        break;
    case NT_EXP_DIVIDE:
        parse(ast->astl_);
        parse(ast->astr_);
        ast->val_ = Reg2Str(Reg++);
        bufferCursor += sprintf(bufferCursor, "\t%s = div %s, %s\n", ast->val_, ast->astl_->val_, ast->astr_->val_);
        break;
    case NT_EXP_MODIFY:
        parse(ast->astl_);
        parse(ast->astr_);
        ast->val_ = Reg2Str(Reg++);
        bufferCursor += sprintf(bufferCursor, "\t%s = mod %s, %s\n", ast->val_, ast->astl_->val_, ast->astr_->val_);
        break;

    case NT_EXP_LESS:
        parse(ast->astl_);
        parse(ast->astr_);
        ast->val_ = Reg2Str(Reg++);
        bufferCursor += sprintf(bufferCursor, "\t%s = lt %s, %s\n", ast->val_, ast->astl_->val_, ast->astr_->val_);
        break;
    case NT_EXP_GREATER:
        parse(ast->astl_);
        parse(ast->astr_);
        ast->val_ = Reg2Str(Reg++);
        bufferCursor += sprintf(bufferCursor, "\t%s = gt %s, %s\n", ast->val_, ast->astl_->val_, ast->astr_->val_);
        break;

    case NT_EXP_LESS_EQUAL:
        parse(ast->astl_);
        parse(ast->astr_);
        ast->val_ = Reg2Str(Reg++);
        bufferCursor += sprintf(bufferCursor, "\t%s = le %s, %s\n", ast->val_, ast->astl_->val_, ast->astr_->val_);
        break;
    case NT_EXP_GREATER_EQUAL:
        parse(ast->astl_);
        parse(ast->astr_);
        ast->val_ = Reg2Str(Reg++);
        bufferCursor += sprintf(bufferCursor, "\t%s = ge %s, %s\n", ast->val_, ast->astl_->val_, ast->astr_->val_);
        break;

    case NT_EXP_EQUAL:
        parse(ast->astl_);
        parse(ast->astr_);
        ast->val_ = Reg2Str(Reg++);
        bufferCursor += sprintf(bufferCursor, "\t%s = eq %s, %s\n", ast->val_, ast->astl_->val_, ast->astr_->val_);
        break;
    case NT_EXP_NEQUAL:
        parse(ast->astl_);
        parse(ast->astr_);
        ast->val_ = Reg2Str(Reg++);
        bufferCursor += sprintf(bufferCursor, "\t%s = ne %s, %s\n", ast->val_, ast->astl_->val_, ast->astr_->val_);
        break;

    // Koopa IR 只支持按位与或, 而不支持逻辑与或, 但你可以用其他运算拼凑出这些运算
    case NT_EXP_AND:
        parse(ast->astl_);
        parse(ast->astr_);
        // 先分别将左右操作数与0比较以获取布尔值
        left_bool = Reg2Str(Reg++);
        bufferCursor += sprintf(bufferCursor, "\t%s = ne %s, 0\n", left_bool, ast->astl_->val_);
        right_bool = Reg2Str(Reg++);
        bufferCursor += sprintf(bufferCursor, "\t%s = ne %s, 0\n", right_bool, ast->astr_->val_);
        // 再进行按位与操作
        ast->val_ = Reg2Str(Reg++);
        bufferCursor += sprintf(bufferCursor, "\t%s = and %s, %s\n", ast->val_, left_bool, right_bool);
        break;

    case NT_EXP_OR:
        parse(ast->astl_);
        parse(ast->astr_);
        // 先分别将左右操作数与0比较以获取布尔值
        left_bool_or = Reg2Str(Reg++);
        bufferCursor += sprintf(bufferCursor, "\t%s = ne %s, 0\n", left_bool_or, ast->astl_->val_);
        right_bool_or = Reg2Str(Reg++);
        bufferCursor += sprintf(bufferCursor, "\t%s = ne %s, 0\n", right_bool_or, ast->astr_->val_);
        // 再进行按位或操作
        ast->val_ = Reg2Str(Reg++);
        bufferCursor += sprintf(bufferCursor, "\t%s = or %s, %s\n", ast->val_, left_bool_or, right_bool_or);
        break;

    case NT_INUMBER:
        break;
    case NT_LVAL:
    {
        val = SymbolTableGet(ast->val_);
        assert(val != NULL);
        if (val->type_ == 0)
        {
            symbol = ast->val_;
            ast->val_ = Reg2Str(Reg++);
            bufferCursor += sprintf(bufferCursor, "\t%s = load @%s\n", ast->val_, symbol);
        }
        else
        {
            ast->val_ = itoa(val->value_);
        }
    }
    break;
    default:
        perror("nodetype error");
        break;
    }
}
struct ast *newastLVal(char *name)
{
    char *str = malloc(sizeof(char) * (strlen(name) + 1));
    if (!str)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    strcpy(str, name);
    return newast(NT_LVAL, NULL, NULL, str);
}
void dumpCompileUnit(struct compileUnit *compileUnit, char *buffer)
{
    assert(compileUnit != NULL);
    assert(buffer != NULL);
    bufferCursor = buffer; // 定义指针追踪当前位置
    while (compileUnit)
    {
        /*
            eg:
            fun @main(): i32 {  // main 函数的定义
            %entry:             // 入口基本块
                ret 0           // return 0
            }
        */
        bufferCursor += sprintf(bufferCursor, "fun @%s(): %s {\n", compileUnit->entry_->name_, !strcmp(compileUnit->entry_->type_, "int") ? "i32" : "Unknown");
        bufferCursor += sprintf(bufferCursor, "%%entry:\n");
        Reg = 0;
        parse(compileUnit->entry_->body_);
        compileUnit = compileUnit->next_;
    }
}

void freeCompileUnit(struct compileUnit *compileUnit)
{
    while (compileUnit)
    {
        // to do
    }
}