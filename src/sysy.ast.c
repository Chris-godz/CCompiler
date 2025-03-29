#include "sysy.ast.h"
#include "sysy.map.h"
struct compileUnit *compilelist = NULL;
static bool ComPareFunc(void *str1, void *str2)
{
    return !strcmp((char *)str1, (char *)str2);
}
static char *bufferCursor = NULL;
static int Reg = 0;
static struct valNode *blkList = NULL;
struct strNode
{
    char *str_;
    struct strNode *next_;
    struct strNode *row_;
}*strList = NULL;
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
bool SymbolTableRegistered(char *symbol)
{
    if (!symbolTable)
    {
        symbolTable = map_create(ComPareFunc, HashCalc);
    }
    return map_contains(symbolTable, symbol);
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

void SymbolTableRegister(char *symbol, struct valNode *valNode)
{
    assert(valNode != NULL);
    if (!symbolTable)
    {
        symbolTable = map_create(ComPareFunc, HashCalc);
    }
    if(!SymbolTableRegistered(symbol))
    {
        // 深拷贝字符串
        char *sym = malloc(sizeof(char) * (strlen(symbol) + 1));
        strcpy(sym, symbol);

        //创建头节点
        struct valNode *head = (struct valNode *)malloc(sizeof(struct valNode));
        if (!head)
        {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        head->next_ = valNode;
        head->row_ = NULL;
        head->type_ = SYM_HEADER;
        // head 和 valNode 的 编号由注册函数统一维护
        head->value_ = (valNode->type_ == SYM_VAR ? 1 : 0);
        if(valNode->type_ == SYM_VAR) valNode->value_ = 0;
        head->ast_ = NULL;
        //注册符号
        map_put(symbolTable, (void *)sym, (void *)head);
        return;
    }
    struct valNode* head = SymbolTableGet(symbol);
    assert(head != NULL);
    assert(head->type_ == SYM_HEADER);
    valNode->next_ = head->next_;
    if(valNode->type_ == SYM_VAR)  valNode->value_ = head->value_++;
    head->next_ = valNode;
    map_put(symbolTable, (void *)symbol, (void *)head);
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
    struct valNode * node = SymbolTableGet(constExp->val_);
    assert(node != NULL);
    struct valNode* nodeHeader = node;
    while(nodeHeader->next_)
    {
        nodeHeader = nodeHeader->next_;
        assert(nodeHeader->type_ != SYM_CONST || nodeHeader->type_ != SYM_HEADER);
    }
    int ret = Calc(constExp->astl_);
    // 注册符号
    struct valNode *valNode = (struct valNode *)malloc(sizeof(struct valNode));
    if (!valNode)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    valNode->type_ = SYM_CONST;
    valNode->value_ = ret;
    valNode->ast_ = NULL;
    valNode->next_ = NULL;
    valNode->row_ = NULL;
    SymbolTableRegister(constExp->val_, valNode);

    assert(blkList != NULL);
    assert(blkList->next_ != NULL);
    node = SymbolTableGet(constExp->val_);
    node = node->next_;
    assert(node != NULL);
    assert(node->type_ == SYM_CONST);
    //更新常量Node链表
    node->row_ = blkList->next_->row_;
    blkList->next_->row_ = node;
    //更新字符串Node链表
    struct strNode* strNode = (struct strNode *)malloc(sizeof(struct strNode));
    if (!strNode)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    char* sym = malloc(sizeof(char) * (strlen(constExp->val_) + 1));
    strcpy(sym, constExp->val_);
    strNode->str_ = sym;
    strNode->row_ = strList->next_->row_;
    strList->next_->row_ = strNode;
}
void variableSymbolRegister(struct ast *variableExp)
{
    struct valNode *valNode = (struct valNode *)malloc(sizeof(struct valNode));
    if (!valNode)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    //变量不处理value统一注册时处理
    valNode->type_ = SYM_VAR;
    valNode->ast_ = variableExp->astl_;
    valNode->next_ = NULL;
    valNode->row_ = NULL;
    SymbolTableRegister(variableExp->val_, valNode);
    struct valNode* node = SymbolTableGet(variableExp->val_);
    node = node->next_;   //更新变量Node链表
    assert(blkList != NULL);
    assert(blkList->next_ != NULL);
    node->row_ = blkList->next_->row_;
    blkList->next_->row_ = node;

    //更新字符串Node链表
    struct strNode* strNode = (struct strNode *)malloc(sizeof(struct strNode));
    if (!strNode)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    char* sym = malloc(sizeof(char) * (strlen(variableExp->val_) + 1));
    strcpy(sym, variableExp->val_);
    strNode->str_ = sym;
    strNode->row_ = strList->next_->row_;
    strList->next_->row_ = strNode;
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
        assert(val->next_ != NULL);
        val = val->next_;
        if (val->type_ == SYM_VAR)
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
            assert(val->type_ == SYM_CONST);
            ret = val->value_;
        }
        break;
    }
    }
    return ret;
}
struct ast* newastExps(struct ast* astl, struct ast* astr)
{
    return newast(NT_STMT_EXPS, astl, astr, NULL);
}

void blockNodeFree()
{
    if(blkList->next_->row_ == NULL)
    {
        assert(strList->next_->row_ == NULL);
        // 说明没有变量
        struct valNode* blkHead = blkList->next_;
        blkList->next_ = blkHead->next_;
        free(blkHead);
        struct strNode* strHead = strList->next_;
        strList->next_ = strHead->next_;
        free(strHead->str_);
        free(strHead);
        return;
    }
    assert(blkList != NULL);
    assert(strList != NULL);
    struct valNode* blkHead = blkList->next_;
    struct valNode* nodeHeader = blkHead;
    struct valNode* nodePrev = NULL;

    struct strNode* strHead = strList->next_;
    struct strNode* strHeader = strHead;
    struct strNode* strPrev = NULL;

    while(nodeHeader->row_)
    {
        nodeHeader = nodeHeader->row_;
        assert(nodeHeader->type_ == SYM_VAR || nodeHeader->type_ == SYM_CONST);
        assert(strHeader->row_ != NULL);
        strHeader = strHeader->row_;

        if(nodePrev)
        {
            assert(strPrev);
            struct valNode* header = SymbolTableGet(strPrev->str_);
            assert(header != NULL);
            assert(header->next_ != NULL);
            assert(header->next_ == nodePrev);
            header->next_ = nodePrev->next_;
            free(nodePrev);
            free(strPrev->str_);
            free(strPrev);
        }

        strPrev = strHeader;
        nodePrev = nodeHeader;
    }
    assert(strPrev);
    struct valNode* header = SymbolTableGet(strPrev->str_);
    assert(header != NULL);
    assert(header->next_ != NULL);
    assert(header->next_ == nodePrev);
    header->next_ = nodePrev->next_;
    free(nodePrev);
    free(strPrev->str_);
    free(strPrev);

    // 释放头节点
    blkHead->row_ = NULL;
    blkList->next_ = blkHead->next_;
    free(blkHead);

    strHead->row_ = NULL;
    strList->next_ = strHead->next_;
    free(strHead->str_);
    free(strHead);
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
        // 处理当前块的变量
        assert(blkList != NULL);
        assert(strList != NULL);
        struct valNode* blkHeader = (struct valNode *)malloc(sizeof(struct valNode));
        if (!blkHeader)
        {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        blkHeader->row_ = NULL;
        blkHeader->type_ = SYM_HEADER;
        blkHeader->ast_ = NULL;
        blkHeader->next_ = blkList->next_;
        blkList->next_ = blkHeader;
        struct strNode* strHeader = (struct strNode *)malloc(sizeof(struct strNode));
        if (!strHeader)
        {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        strHeader->str_ = NULL;
        strHeader->row_ = NULL;
        strHeader->next_ = strList->next_;
        strList->next_ = strHeader;
        
        parse(ast->astl_);
        parse(ast->astr_);
        // 释放当前块的变量
        blockNodeFree();
        break;
    case NT_BLOCK_ITEM:
        parse(ast->astl_);
        parse(ast->astr_);
        break;
    case NT_DECL_CONST:
        ConstSymbolRegister(ast); // 注册常量
        parse(ast->astr_);
        break;
    case NT_DECL_VAR:
        variableSymbolRegister(ast); // 注册变量
        struct valNode* head = SymbolTableGet(ast->val_);
        assert(head != NULL);
        assert(head->type_ == SYM_HEADER);
        assert(head->next_ != NULL);
        bufferCursor += sprintf(bufferCursor, "\t@%s_%d = alloc i32\n", ast->val_ , head->next_->value_);
        if (ast->astl_)
        {
            parse(ast->astl_);
            bufferCursor += sprintf(bufferCursor, "\tstore %s, @%s_%d\n", ast->astl_->val_, ast->val_, head->next_->value_);
        }
        parse(ast->astr_);
        break;
    case NT_STMT_EXPS:
        parse(ast->astl_);
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
        assert(val != NULL);
        // 为空说明之前仅声明了变量，未初始化
        if(val->next_== NULL){
            // 变量未初始化，创建一个新的节点
            struct valNode* node  = (struct valNode *)malloc(sizeof(struct valNode));
            if (!node)
            {
                perror("malloc");
                exit(EXIT_FAILURE);
            }
            node->type_ = SYM_VAR;
            node->ast_ = ast->astr_;
            node->next_ = NULL;
            node->row_ = NULL;
            // 注册符号
            SymbolTableRegister(ast->astl_->val_, node);
        }
        assert(val->next_->type_ == SYM_VAR);
        //变量就更新符号表中的值
        val->next_->ast_ = ast->astr_;
        bufferCursor += sprintf(bufferCursor, "\tstore %s, @%s_%d\n", ast->astr_->val_, ast->astl_->val_, val->next_->value_);
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
        // 再进行按位或操作blkList
        ast->val_ = Reg2Str(Reg++);
        bufferCursor += sprintf(bufferCursor, "\t%s = or %s, %s\n", ast->val_, left_bool_or, right_bool_or);
        break;

    case NT_INUMBER:
        break;
    case NT_LVAL:
    {
        val = SymbolTableGet(ast->val_);
        assert(val != NULL);
        assert(val->next_ != NULL);
        val = val->next_;
        assert(val->type_ == SYM_VAR || val->type_ == SYM_CONST);
        if (val->type_ == SYM_VAR)
        {
            symbol = ast->val_;
            ast->val_ = Reg2Str(Reg++);
            bufferCursor += sprintf(bufferCursor, "\t%s = load @%s_%d\n", ast->val_, symbol , val->value_);
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
    assert(blkList == NULL);
    blkList = (struct valNode *)malloc(sizeof(struct valNode));
    if (!blkList)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    blkList->next_ = NULL;
    blkList->row_ = NULL;
    blkList->type_ = SYM_HEADER;
    blkList->ast_ = NULL;
    assert(strList == NULL);
    strList = (struct strNode *)malloc(sizeof(struct strNode));
    if (!strList)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    strList->str_ = NULL;
    strList->next_ = NULL;
    strList->row_ = NULL;

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