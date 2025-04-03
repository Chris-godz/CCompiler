#ifndef SYS_RISCV_H
#define SYS_RISCV_H
#include "koopa.h"
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
void riscvCompile(char* ir,FILE* fp_out);
void calc_stack_frame_size(const koopa_raw_slice_t block);
void load_value_to_reg(const koopa_raw_value_t value,int reg);
void visit_load(const koopa_raw_load_t load, const koopa_raw_value_t value);
void visit_slice(const koopa_raw_slice_t slice);
void visit_program(const koopa_raw_program_t program);
void visit_function(const koopa_raw_function_t function);
void visit_block(const koopa_raw_basic_block_t block);
void visit_binary(const koopa_raw_binary_t binary, const koopa_raw_value_t value);
void visit_int(const koopa_raw_integer_t exp);
void visit_ret(const koopa_raw_return_t exp);
void visit_value(const koopa_raw_value_t raw);
#endif // SYS_RISCV_H