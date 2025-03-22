#include "sysy.ast.h"
#include "sysy.riscv.h"
#define MAX_IR_LENTH 1000
extern FILE* yyin;
extern int yyparse();
int main(int argc , char **argv)
{
    assert(argc == 5);
    // eg : compiler -koopa input_file -o output_file
    char* mode = argv[1];
    char* input = argv[2];
    char* output = argv[4];
    FILE *fp_out = fopen(output, "w");
    if(!fp_out) { perror("fopen"); return EXIT_FAILURE; }
    yyin = fopen(input, "r");
    if(!yyin) { perror("fopen"); return EXIT_FAILURE; }
    yyparse();
    char ir_buffer[MAX_IR_LENTH];
    if(!strcmp(mode, "-koopa"))
    {
        dumpCompileUnit(compilelist, ir_buffer);
        fprintf(fp_out, "%s", ir_buffer);
    }
    else if(!strcmp(mode,"-riscv"))
    {
        dumpCompileUnit(compilelist, ir_buffer);
        riscvCompile(ir_buffer, fp_out);
    }
    fclose(fp_out);
    return 0;
}