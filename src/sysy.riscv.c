#include "sysy.riscv.h"
#include "sysy.map.h"
#include <stdint.h>  // 包含 uintptr_t 类型
static FILE* fp_out_global = NULL;
static HashMap* value_offset_map = NULL;
/*
    fun @main(): i32 {
        %entry:
        // int x = 10;
        @x = alloc i32
        store 10, @x
        
        // x = x + 1;
        %0 = load @x
        %1 = add %0, 1
        store %1, @x
        
        // return x;
        %2 = load @x
        ret %2
        }
*/
static const char* reg_names[15] = {"t0", "t1", "t2", "t3", "t4", "t5", "t6",
    "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};
static int stack_frame_size_used = 0;
static int stack_frame_size =  0;
static bool ComPareFunc(void *key1, void *key2)
{
    return key1 == key2;
}
static size_t HashCalc(void *key)
{
    return (size_t)key;
}

int get_offset(void* key)
{
    if(!value_offset_map)
    {
        perror("map not init");
        exit(EXIT_FAILURE);
    }
    return (int)(intptr_t)map_get(value_offset_map, key);
}
void set_offset(void* key, int value)
{
    if(!value_offset_map)
    {
        value_offset_map = map_create(ComPareFunc,HashCalc);
    }
    map_put(value_offset_map, key, (void*)(intptr_t)value);
}

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
    // 访问所有全局变量
    visit_slice(program.values);
    // 访问所有函数
    visit_slice(program.funcs);
}
void visit_function(const koopa_raw_function_t function)
{
    fprintf(fp_out_global, "\t.text\n");
    fprintf(fp_out_global, "\t.global %s\n",function->name+1);
    fprintf(fp_out_global, "%s:\n",function->name+1); // delete the first char '@'
    stack_frame_size_used = 0;
    stack_frame_size = 0;
    calc_stack_frame_size(function->bbs);
    if(stack_frame_size > 0)
    {
        fprintf(fp_out_global, "\taddi sp, sp, -%d\n", stack_frame_size);
    }
    // 访问函数参数
    visit_slice(function->bbs);
}
void visit_block(const koopa_raw_basic_block_t block)
{
    fprintf(fp_out_global, "%s:\n",block->name+1); 
    visit_slice(block->insts);
}


void visit_ret(const koopa_raw_return_t ret)
{
    load_value_to_reg(ret.value, 7);
    if (stack_frame_size != 0)
    {
        fprintf(fp_out_global, "\taddi sp, sp, %d\n", stack_frame_size);
    }
    fprintf(fp_out_global, "\tret\n");
}
void visit_int(const koopa_raw_integer_t integer)
{
    fprintf(fp_out_global, "\tli a0, %d\n", integer.value);
}
void load_value_to_reg(const koopa_raw_value_t value,int reg)
{
    if(value->kind.tag == KOOPA_RVT_INTEGER)
    {
        fprintf(fp_out_global, "\tli %s, %d\n", reg_names[reg], value->kind.data.integer.value);
    }
    else
    {
        fprintf(fp_out_global, "\tlw %s, %d(sp)\n", reg_names[reg], get_offset((void*)value));
    }
}
void visit_load(const koopa_raw_load_t load, const koopa_raw_value_t value)
{
    load_value_to_reg(load.src, 0);
    fprintf(fp_out_global, "\tsw %s, %d(sp)\n", reg_names[0], stack_frame_size_used);
    set_offset((void*)value,stack_frame_size_used);
    stack_frame_size_used += 4;
}
void visit_store(const koopa_raw_store_t store)
{
    load_value_to_reg(store.value , 0);
    fprintf(fp_out_global, "\tsw %s, %d(sp)\n", reg_names[0], get_offset((void*)store.dest));
}
void visit_binary(const koopa_raw_binary_t binary, const koopa_raw_value_t value)
{
    int reg_left = 0, reg_right = 1;
    load_value_to_reg(binary.lhs, reg_left);
    load_value_to_reg(binary.rhs, reg_right);
    switch(binary.op)
    {
        case KOOPA_RBO_NOT_EQ:
            fprintf(fp_out_global, "\txor %s, %s, %s\n", reg_names[reg_left], reg_names[reg_left], reg_names[reg_right]);
            fprintf(fp_out_global, "\tsnez %s, %s\n", reg_names[reg_left], reg_names[reg_left]);
            break;
        case KOOPA_RBO_EQ:
            fprintf(fp_out_global, "\txor %s, %s, %s\n", reg_names[reg_left], reg_names[reg_left], reg_names[reg_right]);
            fprintf(fp_out_global, "\tseqz %s, %s\n", reg_names[reg_left], reg_names[reg_left]);
            break;
        case KOOPA_RBO_GT:
            fprintf(fp_out_global, "\tsgt %s, %s, %s\n", reg_names[reg_left], reg_names[reg_left], reg_names[reg_right]);
            break;
        case KOOPA_RBO_LT:
            fprintf(fp_out_global, "\tslt %s, %s, %s\n", reg_names[reg_left], reg_names[reg_left], reg_names[reg_right]);
            break;
        case KOOPA_RBO_GE:
            fprintf(fp_out_global, "\tslt %s, %s, %s\n", reg_names[reg_left], reg_names[reg_left], reg_names[reg_right]);
            fprintf(fp_out_global, "\txori %s, %s, 1\n", reg_names[reg_left], reg_names[reg_left]);
            break;
        case KOOPA_RBO_LE:
            fprintf(fp_out_global, "\tsgt %s, %s, %s\n", reg_names[reg_left], reg_names[reg_left], reg_names[reg_right]);
            fprintf(fp_out_global, "\txori %s, %s, 1\n", reg_names[reg_left], reg_names[reg_left]);
            break;
        case KOOPA_RBO_ADD:
            fprintf(fp_out_global, "\tadd %s, %s, %s\n", reg_names[reg_left], reg_names[reg_left], reg_names[reg_right]);
            break;
        case KOOPA_RBO_SUB:
            fprintf(fp_out_global, "\tsub %s, %s, %s\n", reg_names[reg_left], reg_names[reg_left], reg_names[reg_right]);
            break;
        case KOOPA_RBO_MUL:
            fprintf(fp_out_global, "\tmul %s, %s, %s\n", reg_names[reg_left], reg_names[reg_left], reg_names[reg_right]);
            break;
        case KOOPA_RBO_DIV:
            fprintf(fp_out_global, "\tdiv %s, %s, %s\n", reg_names[reg_left], reg_names[reg_left], reg_names[reg_right]);
            break;
        case KOOPA_RBO_MOD:
            fprintf(fp_out_global, "\trem %s, %s, %s\n", reg_names[reg_left], reg_names[reg_left], reg_names[reg_right]);
            break;
        case KOOPA_RBO_AND:
            fprintf(fp_out_global, "\tand %s, %s, %s\n", reg_names[reg_left], reg_names[reg_left], reg_names[reg_right]);
            break;
        case KOOPA_RBO_OR:
            fprintf(fp_out_global, "\tor %s, %s, %s\n", reg_names[reg_left], reg_names[reg_left], reg_names[reg_right]);
            break;
        case KOOPA_RBO_XOR:
            fprintf(fp_out_global, "\txor %s, %s, %s\n", reg_names[reg_left], reg_names[reg_left], reg_names[reg_right]);
            break;
        case KOOPA_RBO_SHL:
            fprintf(fp_out_global, "\tsll %s, %s, %s\n", reg_names[reg_left], reg_names[reg_left], reg_names[reg_right]);
            break;
        case KOOPA_RBO_SHR:
            fprintf(fp_out_global, "\tsrl %s, %s, %s\n", reg_names[reg_left], reg_names[reg_left], reg_names[reg_right]);
            break;
        case KOOPA_RBO_SAR:
            fprintf(fp_out_global, "\tsra %s, %s, %s\n", reg_names[reg_left], reg_names[reg_left], reg_names[reg_right]);
            break;
    }
    set_offset((void*)value, stack_frame_size_used);
    fprintf(fp_out_global, "\tsw %s, %d(sp)\n", reg_names[reg_left], stack_frame_size_used);
    stack_frame_size_used += 4;
}
void visit_branch(const koopa_raw_branch_t branch)
{
    load_value_to_reg(branch.cond, 0);
    fprintf(fp_out_global, "\tbnez %s, %s\n", reg_names[0], branch.true_bb->name+1);
    fprintf(fp_out_global, "\tj %s\n", branch.false_bb->name+1);
    // visit_block(branch.true_bb);
    // visit_block(branch.false_bb);
}
void visit_jump(const koopa_raw_jump_t jump)
{
    fprintf(fp_out_global, "\tj %s\n", jump.target->name+1);
}
void visit_value(const koopa_raw_value_t value)
{
    
    switch (value->kind.tag)
    {
        case KOOPA_RVT_RETURN:
            visit_ret(value->kind.data.ret);
            break;
        case KOOPA_RVT_INTEGER:
            visit_int(value->kind.data.integer);
            break;
        case KOOPA_RVT_ALLOC:
            set_offset((void*)value, stack_frame_size_used);
            stack_frame_size_used += 4;
            break;
        case KOOPA_RVT_LOAD:
            //  eg: %0 = load @x
            //  lw t0, 0(sp)
            //  sw t0, 4(sp)
            // typedef struct {
            // koopa_raw_value_t src;
            // } koopa_raw_load_t;
            visit_load(value->kind.data.load, value);
            break;
        case KOOPA_RVT_STORE:
            //  eg: store 10, @x
            //  li t0, 10
            //  sw t0, 0(sp)
            // typedef struct {
            // koopa_raw_value_t value;
            // koopa_raw_value_t dest;
            // } koopa_raw_store_t;
            visit_store(value->kind.data.store);
            break;
        case KOOPA_RVT_BINARY:
            //  eg: %1 = add %0, 1
            //  lw t0, 4(sp)
            //  li t1, 1
            //  add t0, t0, t1
            //  sw t0, 8(sp)

            // typedef struct {
            //     /// Operator.
            //     koopa_raw_binary_op_t op;
            //     /// Left-hand side value.
            //     koopa_raw_value_t lhs;
            //     /// Right-hand side value.
            //     koopa_raw_value_t rhs;
            //   } koopa_raw_binary_t;
            visit_binary(value->kind.data.binary , value);
            break;
        case KOOPA_RVT_BRANCH:
            // typedef struct {
            //     /// Condition.
            //     koopa_raw_value_t cond;
            //     /// Target if condition is `true`.
            //     koopa_raw_basic_block_t true_bb;
            //     /// Target if condition is `false`.
            //     koopa_raw_basic_block_t false_bb;
            //     /// Arguments of `true` target..
            //     koopa_raw_slice_t true_args;
            //     /// Arguments of `false` target..
            //     koopa_raw_slice_t false_args;
            //   } koopa_raw_branch_t;
            visit_branch(value->kind.data.branch);
            break;
        case KOOPA_RVT_JUMP:
            // typedef struct {
            //     /// Target.
            //     koopa_raw_basic_block_t target;
            //     /// Arguments of target..
            //     koopa_raw_slice_t args;
            //   } koopa_raw_jump_t;
            visit_jump(value->kind.data.jump);
            break;
        default:
            assert(false);
    }
}
void calc_stack_frame_size(const koopa_raw_slice_t block)
{
    // 计算栈帧大小
    assert(block.kind == KOOPA_RSIK_BASIC_BLOCK);
    for(size_t i = 0; i < block.len; i++)
    {
        koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t)block.buffer[i];
        koopa_raw_slice_t insts = bb->insts;
        assert(insts.kind == KOOPA_RSIK_VALUE);
        for(size_t j = 0; j < insts.len; j++)
        {
            koopa_raw_value_t value = (koopa_raw_value_t)insts.buffer[j];
            // 如果指令的类型为 unit (类似 C/C++ 中的 void), 则这条指令不存在返回值.
            if(value->kind.tag == KOOPA_RVT_ALLOC || value->ty->tag != KOOPA_RTT_UNIT)
            {
                stack_frame_size += 4;
            }
        }   
    }
    //sp 寄存器用来保存栈指针, 它的值必须是 16 字节对齐的 (RISC-V 规范第 107 页)
    stack_frame_size = (stack_frame_size + 15) >> 4 << 4;
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