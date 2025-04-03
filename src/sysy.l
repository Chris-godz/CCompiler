%option noyywrap
%option nounput
%option noinput
%{
#include "sysy.tab.h"
int fileno(FILE *stream);
%}

%%
[ \t\n]+ { /* ignore white space */ }
"//".* { /* ignore line comments */ }
"/*"([^*]*|(\*+[^/]))*"*/" { /* ignore block comments */ }

"const"             { return CONST; }
"int"               { return INT; }
"return"            { return RETURN; }
"if"                { return IF; }
"else"              { return ELSE; }
"<="                { return LE; }
">="                { return GE; }
"!="                { return NE; }
"=="                { return EQ; }  
"&&"                { return AND; }
"||"                { return OR; }

[a-zA-Z_][a-zA-Z0-9_]* { char* newstr = (char*)malloc(strlen(yytext) + 1); strcpy(newstr, yytext); yylval.symbol_ = newstr; return IDENT; }
[1-9][0-9]*         { yylval.val_ = strtol(yytext, NULL, 10); return IVAL; }
0[0-7]*             { yylval.val_ = strtol(yytext, NULL, 8); return IVAL; }
0[xX][0-9a-fA-F]+   { yylval.val_ = strtol(yytext, NULL, 16); return IVAL; }
.                   { return yytext[0]; }
%%