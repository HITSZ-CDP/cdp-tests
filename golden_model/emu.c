#include <cpu.h>
#include <debug.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
static const char *reg_name[33] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6",
  "this_pc"
};

riscv32_CPU_state cpu;
uint32_t memory[4096];
extern IF2ID IF(uint32_t);
extern ID2EX ID(IF2ID);
extern EX2MEM EX(ID2EX);
extern MEM2WB MEM(EX2MEM);
extern WB_info WB(MEM2WB);
WB_info cpu_run_once() {
    IF2ID inst = IF(cpu.npc);    
    ID2EX decode_info = ID(inst);
    EX2MEM ex_info = EX(decode_info);
    MEM2WB mem_info = MEM(ex_info);
    return WB(mem_info);
}

void print_reg_state(){
    color_print("======= REG VALUE =======\n");
    color_print("x[ 0] = 0x00000000\t");
    for(int i = 1; i < 32; i++) {
        if( (i % 4) == 0 ) printf("\n");
        color_print("x[%2d] = 0x%8.8x\t", i, cpu.gpr[i]);
    }
    printf("\n");
}

void init_memory(const char *fname) {
    assert(fname != NULL);
    FILE *fp = fopen(fname, "rb");
    Assert(fp != NULL, "Cannot open file \"%s\"!\n", fname);
    Log("Using file \"%s\" as memory image.\n", fname);
    fseek(fp, 0, SEEK_END);
    int img_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    int ret = fread(memory, img_size, 1, fp);
    assert(ret == 1);
    fclose(fp);
}

void init_cpu(const char *fname) {
    cpu.npc = 0;
    init_memory(fname);
}
