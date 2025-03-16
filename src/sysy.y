%{
#include "sysy.ast.h"
// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(const char *s);
%}

%union{
    char *symbol_;
    struct ast* ast_;
    struct fun* fun_;
    struct compileUnit* com_;
    int val_;
}

%token INT RETURN IDENT IVAL

%type<symbol_> IDENT FuncType
%type<val_> IVAL
%type<ast_> Block Exp Number
%type<fun_> FuncDef
%type<com_> CompUnit

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

Block: '{' Exp '}' {
        $$ = newastblk($2,NULL);
    }
    ;

Exp: RETURN Number ';' {
        $$ = newastRet($2);
    }
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