#include "../include/cpu.h"
#include <stdint.h>
extern riscv32_CPU_state cpu;
extern uint32_t memory[];
uint32_t monitor_value_passed;
uint32_t monitor_value_failed;
#define PASSED_MONITOR_ADDR 0x80000000
#define FAILED_MONITOR_ADDR 0x80000004

#define MEM_LOAD_CASE_ENTRY(op, calc) case op: load_result = (calc); break
#define MEM_STORE_CASE_ENTRY(op, calc) case op: store_val = (calc); is_store = 1; break
uint32_t mem_load(uint32_t addr, uint32_t sz, uint32_t is_signed) {
    Log("Load");
    Log("Mem addr = %8.8x", addr);
    uint32_t index = addr >> 2;
    uint32_t byte_offset = addr & 3;
    uint32_t half_offset = byte_offset >> 1;
    uint32_t bitmask = 0xffffffff;
    Log("Mem Size = %8.8x", sz);
    switch (sz) {
        case 1: bitmask = 0xffu << (byte_offset * 8); break;
        case 2: bitmask = 0xffffu << (half_offset * 16); break;
        default: bitmask = 0xffffffff; break;
    }
    uint32_t mem_line;
    if(index == PASSED_MONITOR_ADDR) {
        mem_line = monitor_value_passed;
    } else if(index == FAILED_MONITOR_ADDR) {
        mem_line = monitor_value_failed;
    } else {
        mem_line = memory[index];
    }
    Log("Mem Line = %8.8x", mem_line);
    uint32_t ret = (mem_line & bitmask);
    Log("bitmask = %8.8x", bitmask);
    switch (sz) {
        case 1: ret = ret >> (byte_offset * 8); break;
        case 2: ret = ret >> (half_offset * 16); break;
        default: ret = ret; break;
    }
    if(is_signed) {
        switch (sz) {
            case 1: ret = (int32_t)((int8_t)ret); break;
            case 2: ret = (int32_t)((int16_t)ret); break;
            default: ret = ret; break;
        }
    }
    return ret;
}

void mem_store(uint32_t addr, uint32_t sz, uint32_t value) {
    uint32_t index = addr >> 2;
    uint32_t byte_offset = addr & 3;
    uint32_t half_offset = byte_offset >> 1;
    uint32_t bitmask = 0xffffffff;
    Log("Store");
    Log("Mem addr = %8.8x", addr);
    Log("Mem Size = %8.8x", sz);
    Log("register = %8.8x", value);
    switch (sz) {
        case 1: bitmask = 0xff << (byte_offset * 8); value = value & 0xff; break;
        case 2: bitmask = 0xffff << (half_offset * 16); value = value & 0xffff; break;
        default: bitmask = 0xffffffff; break;
    }
    uint32_t mem_line;
    if(index == PASSED_MONITOR_ADDR) {
        mem_line = monitor_value_passed;
    } else if(index == FAILED_MONITOR_ADDR) {
        mem_line = monitor_value_failed;
    } else {
        mem_line = memory[index];
    }
    Log("Mem Line = %8.8x", mem_line);
    uint32_t store_val = (mem_line & (~bitmask));  // clear the corresponding positions
    switch (sz) {
        case 1: store_val = (value << (byte_offset * 8)) | store_val; break;
        case 2: store_val = (value << (half_offset * 16)) | store_val; break;
        default: store_val = value; break;
    }
    if(index == PASSED_MONITOR_ADDR) {
        monitor_value_passed = store_val;
    } else if(index == FAILED_MONITOR_ADDR) {
        monitor_value_failed = store_val;
    } else {
        memory[index] = store_val;
    }
    Log("new Mem Line = %8.8x", store_val);
}

MEM2WB MEM(EX2MEM ex_info) {
    MEM2WB ret;
    // passthrough
    ret.alu_out = ex_info.alu_out;
    ret.wb_sel = ex_info.wb_sel;
    ret.wb_en = ex_info.wb_en;
    ret.dst = ex_info.dst;
    ret.branch_taken = ex_info.branch_taken;
    ret.target_pc = ex_info.target_pc;
    ret.inst = ex_info.inst;
    ret.pc = ex_info.pc;
    Log("PC = %8.8x", ret.pc);
    uint32_t load_result = 0;
    uint32_t is_store = 0;
    uint32_t mem_sz;
    uint32_t is_signed = 0;
    if(ex_info.is_mem) {
        switch (ex_info.mem_op) {
            case MEM_SB: mem_sz = 1; is_store = 1;  break; case MEM_SH: mem_sz = 2; is_store = 1; break; case MEM_SW: mem_sz = 4; is_store = 1; break;
            case MEM_LB: mem_sz = 1; is_signed = 1; break; case MEM_LH: mem_sz = 2;  is_signed = 1;break; case MEM_LW: mem_sz = 4;  is_signed = 1;break;
            case MEM_LBU: mem_sz = 1; break; case MEM_LHU: mem_sz = 2; break; 
            default: is_store = 0; is_signed = 0;
        }
        if(is_store) {
            mem_store(ret.alu_out, mem_sz, ex_info.store_val);
        } else {
            ret.load_out = mem_load(ret.alu_out, mem_sz, is_signed);
        }
    }
    return ret;
}

