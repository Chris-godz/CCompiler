#include "sysy.ast.h"

extern FILE* yyin;
extern int yyparse();
int main(int argc , char **argv)
{
    assert(argc == 5);
    // eg : compiler -koopa input_file -o output_file
    char* mode = argv[1];
    char* input = argv[2];
    char* output = argv[4];
    yyin = fopen(input, "r");
    FILE *fp = freopen(output, "w", stdout);
    if(!yyin) { perror("fopen"); return 1; }
    yyparse();
    if(!strcmp(mode, "-koopa"))
    {
        dumpCompileUnit(compilelist);
    }

    fclose(fp);
    return 0;
}