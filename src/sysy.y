%{
#include "sysy.ast.h"
// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(const char *s);
%}

%union{
    char *symbol_;
    char op;
    struct ast* ast_;
    struct fun* fun_;
    struct compileUnit* com_;
    int val_;
}

%token ELSE IF INT CONST RETURN IDENT IVAL LE GE EQ NE AND OR

%type<symbol_> IDENT FuncType BType
%type<val_> IVAL
%type<ast_> Block BlocKItem Stmt Exps Decl VarDecl VarDef VarInit ConstDecl ConstDef ConstInit ConstExp Exp LorExp LandExp EqExp CmpExp AddExp MulExp UnaryExp PrimaryExp LVal Number
%type<fun_> FuncDef
%type<com_> CompUnit
%type<op> UnaryOp

%start CompUnit

// 用于解决 dangling else 的优先级设置
%precedence IFX
%precedence ELSE
%%

CompUnit: FuncDef {
        $$ = newCompileUnit($1);
    }
    ;

FuncDef: FuncType IDENT '(' ')' Block{
        $$ = newFun($1,$2,NULL,$5);
    }
    ;

FuncType: INT {
        char* newstr = (char*)malloc(strlen("int") + 1);
        strcpy(newstr, "int");
        $$ = newstr;
    }
    ;

Block:  '{' BlocKItem '}'   { $$ = newastBlk( $2 , NULL); }
    | '{' '}' { $$ = newastBlk( NULL, NULL); }
    ;

BlocKItem: Stmt { $$ = newastBItem($1, NULL); }
    | Decl { $$ = newastBItem($1, NULL); }
    | Decl BlocKItem { $$ = newastBItem($1, $2); }
    | Stmt BlocKItem { $$ = newastBItem($1, $2); }
    ;

Stmt: LVal '=' Exp ';' { $$ = newastAssign($1, $3); }
    | RETURN Exp ';'   { $$ = newastRet($2); }
    | Block { $$ = $1; }
    | Exps { $$ = $1; }
    | IF '(' Exp ')' Stmt %prec IFX { $$ = newastIf($3, $5, NULL); }
    | IF '(' Exp ')' Stmt ELSE Stmt { $$ = newastIf($3, $5, $7); }
    ;

Exps: ';' { $$ = newastExps(NULL,NULL); }
    | Exp ';' { $$ = newastExps($1, NULL); }
    | Exp ';' Exps { $$ = newastExps($1, $3); }

Decl: ConstDecl { $$ =$1; }
    | VarDecl { $$ = $1; }
    ;

VarDecl: BType VarDef ';' { $$ = $2; }
    ;

VarDef: IDENT { $$ = newastVarDef( NULL , NULL, $1 ); }
    | IDENT '=' VarInit  { $$ = newastVarDef( $3 , NULL, $1 ); }
    | IDENT '=' VarInit ',' VarDef { $$ = newastVarDef( $3 , $5, $1 ); }
    | IDENT ',' VarDef { $$ = newastVarDef( NULL , $3, $1 ); }
    ;

VarInit: Exp { $$ = $1; }
    ;

ConstDecl: CONST BType ConstDef ';' {  $$ = $3; }
    ;

ConstDef: IDENT '=' ConstInit  { $$ = newastConstDef($3 ,NULL,$1); }
    | IDENT '=' ConstInit ',' ConstDef { $$ = newastConstDef($3, $5 , $1); }
    ;

BType: INT  {
    char* newstr = (char*)malloc(strlen("int") + 1);
    strcpy(newstr, "int");
    $$ = newstr;
    }
    ;

ConstInit: ConstExp { $$ = $1; }
    ;

ConstExp: Exp { $$ = $1; }
    ;

Exp: LorExp { $$ = $1; }
    ;

LorExp: LandExp { $$ = $1; }
    | LorExp OR LandExp { $$ = newastBExp('O', $1, $3); }
    ;

LandExp:EqExp { $$ = $1; }
    | LandExp AND EqExp { $$ = newastBExp('A', $1, $3); }
    ; 

EqExp: CmpExp { $$ = $1; }
    | EqExp EQ CmpExp { $$ = newastBExp('E', $1, $3); }
    | EqExp NE CmpExp { $$ = newastBExp('N', $1, $3); }
    ;

CmpExp: AddExp { $$ = $1; }
    | CmpExp '<' AddExp { $$ = newastBExp('<', $1, $3); }
    | CmpExp '>' AddExp { $$ = newastBExp('>', $1, $3); }
    | CmpExp LE AddExp { $$ = newastBExp('L', $1, $3); }
    | CmpExp GE AddExp { $$ = newastBExp('G', $1, $3); }
    
AddExp: MulExp { $$ = $1; }
    | AddExp '+' MulExp { $$ = newastBExp('+', $1, $3); }
    | AddExp '-' MulExp { $$ = newastBExp('-', $1, $3); }
    ;

MulExp: UnaryExp { $$ = $1; }
    | MulExp '*' UnaryExp { $$ = newastBExp('*', $1, $3); }
    | MulExp '/' UnaryExp { $$ = newastBExp('/', $1, $3); }
    | MulExp '%' UnaryExp { $$ = newastBExp('%', $1, $3); }
    ;

UnaryExp: PrimaryExp { $$ = $1; }
    | UnaryOp UnaryExp { $$ = newastUExp($1,$2); }
    ;

PrimaryExp: Number { $$ = $1; }
    | '(' Exp ')' { $$ = $2; }
    | LVal { $$ = $1; }
    ;

UnaryOp: '-' { $$ = '-'; }
    | '+' { $$ = '+'; }
    | '!' { $$ = '!'; }
    ;

LVal: IDENT { $$ = newastLVal($1); }
    ;

Number: IVAL { 
        $$ = newastNum($1); 
    }
    ;
%%

void yyerror(const char *s){
    extern int yylineno;
    extern char* yytext;
    int len = strlen(yytext);
    char buffer[512] = {0};
    for(int i = 0; i < len; i++){
        sprintf(buffer,"%s%d",buffer,yytext[i]);
        }
    fprintf(stderr, "Error:%s , line:%d , %s\n" , s, yylineno , buffer);
}