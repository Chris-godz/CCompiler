#include "sysy.riscv.h"
static FILE* fp_out_global = NULL;
/*
  .text
  .globl main
main:
  li a0, 0
  ret
*/
void visit_slice(const koopa_raw_slice_t slice)
{ 
    for(size_t i =0; i< slice.len; i++)
    {
        const void* ptr = slice.buffer[i];
        switch (slice.kind)
        {
        case KOOPA_RSIK_FUNCTION:
            visit_function((koopa_raw_function_t)ptr);
            break;
        case KOOPA_RSIK_BASIC_BLOCK:
            visit_block((koopa_raw_basic_block_t)ptr);
            break;
        case KOOPA_RSIK_VALUE:
            visit_value((koopa_raw_value_t)ptr);
            break;
        default:
            assert(false);
        }
    }
}
void visit_program(const koopa_raw_program_t program)
{
    visit_slice(program.values);
    visit_slice(program.funcs);
}
void visit_function(const koopa_raw_function_t function)
{
    fprintf(fp_out_global, "  .text\n");
    fprintf(fp_out_global, "  .global %s\n",function->name+1);
    fprintf(fp_out_global, "%s:\n", function->name+1); // delete the first char '@'
    visit_slice(function->bbs);
}
void visit_block(const koopa_raw_basic_block_t block)
{
    visit_slice(block->insts);
}
void visit_ret(const koopa_raw_return_t exp)
{
    visit_value(exp.value);
    fprintf(fp_out_global, "  ret");
}
void visit_int(const koopa_raw_integer_t exp)
{
    fprintf(fp_out_global, "  li a0, %d\n", exp.value);
}
void visit_value(const koopa_raw_value_t raw)
{
    switch (raw->kind.tag)
    {
        case KOOPA_RVT_RETURN:
            visit_ret(raw->kind.data.ret);
            break;
        case KOOPA_RVT_INTEGER:
            visit_int(raw->kind.data.integer);
            break;
        default:
            assert(false);
    }
}

void riscvCompile(char* ir,FILE* fp_out)
{
    assert(fp_out); assert(ir);
    // 解析字符串 str, 得到 Koopa IR 程序
    koopa_program_t program;
    koopa_error_code_t ret = koopa_parse_from_string(ir, &program);
    assert(ret == KOOPA_EC_SUCCESS); // 确保解析时没有出错
    // 创建一个 raw program builder, 用来构建 raw program
    koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
    // 将 Koopa IR 程序转换为 raw program
    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
    // 释放 Koopa IR 程序占用的内存
    koopa_delete_program(program);
    fp_out_global = fp_out;
    // 处理 raw program
    visit_program(raw);

    // 处理完成, 释放 raw program builder 占用的内存
    // 注意, raw program 中所有的指针指向的内存均为 raw program builder 的内存
    // 所以不要在 raw program 处理完毕之前释放 builder
    koopa_delete_raw_program_builder(builder);


}