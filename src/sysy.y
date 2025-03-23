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

%token INT RETURN IDENT IVAL LE GE EQ NE AND OR

%type<symbol_> IDENT FuncType
%type<val_> IVAL
%type<ast_> Block Stmt Exp LorExp LandExp EqExp CmpExp AddExp MulExp UnaryExp PrimaryExp Number
%type<fun_> FuncDef
%type<com_> CompUnit
%type<op> UnaryOp

%start CompUnit

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

Block: '{' Stmt '}' {
        $$ = newastblk($2,NULL);
    }
    ;

Stmt: RETURN Exp ';' {
        $$ = newastRet($2);
    }
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
    ;

UnaryOp: '-' { $$ = '-'; }
    | '+' { $$ = '+'; }
    | '!' { $$ = '!'; }
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