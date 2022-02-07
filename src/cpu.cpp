#include "gbemu.hpp"

/* 参考
    https://izik1.github.io/gbops/
    http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf
*/

void GBEmu::cpu_step(){
    uint16_t addr;
    int8_t i8a, i8b;
    uint8_t u8a, u8b, u8c;

    if (is_break(reg.pc)) {
        mtx_stop.lock();
        stop = true;
        mtx_stop.unlock();
    }
    
    while (stop && !exit_emu);

    printf("0x%04x: 0x%02x\n", reg.pc, read_mem(reg.pc));
    switch(read_mem(reg.pc)){
        case 0x00: //nop
            reg.pc += 1;
            last_instr_clock = 4;
        break;
        case 0x01: //ld bc, u16
            reg.bc = read_mem_u16(reg.pc + 1);
            reg.pc += 3;
            last_instr_clock = 12;
        break;
        case 0x03: //inc bc
            reg.bc++;
            reg.pc += 1;
            last_instr_clock = 8;
        break;
        case 0x18: //jr u8
            i8a = read_mem(reg.pc + 1);
            /*
            単純にpcに次のu8の値を足すだけかと思ったが、次の命令のアドレスを基準に足すらしい？
            例えば以下の場合、次のu8が格納されているアドレスを基準に加算するのではなく、
            次の命令(0x03のnop)に加算し、0x07になる。
            0x01: 0x18 ; jr 0x04 -> (0x03 + 4)=0x07
            0x02: 0x04 
            0x03: 0x00 ; nop
            */
            reg.pc += 2;
            reg.pc += i8a;
            last_instr_clock = 12;
        break;
        case 0x20: //jr if not zero i8
            i8a = read_mem(reg.pc + 1);
            reg.pc += 2;
            if(reg.flags.z != 1){
                reg.pc += i8a;
                last_instr_clock = 12;
            }
            else{
                last_instr_clock = 8;
            }
        break;
        case 0x21: //ld hl, u16
            reg.hl = read_mem_u16(reg.pc + 1);
            reg.pc += 3;
            last_instr_clock = 12;
        break;
        case 0x23: //inc hl
            reg.hl++;
            reg.pc += 1;
            last_instr_clock = 8;
        break;
        case 0x28: //jr z, i8
            //printf("a: 0x%02x  b: %02x  c: 0x%02x  z: %d\n", reg.a, reg.b, reg.c, reg.flags.z);
            i8a = read_mem(reg.pc + 1);
            reg.pc += 2;
            if(reg.flags.z == 1){
                reg.pc += i8a;
                last_instr_clock = 12;
            }
            else{
                last_instr_clock = 8;
            }
        break;
        case 0x2a: //ld, a (hl)
            reg.a = read_mem(reg.hl);
            reg.pc += 1;
            last_instr_clock = 8;
        break;
        case 0x31: //ld sp, u16
            reg.sp = read_mem_u16(reg.pc + 1);
            reg.pc += 3;
            last_instr_clock = 12;
        break;
        case 0x3e: //ld A, u8
            reg.a = read_mem(reg.pc + 1);
            reg.pc += 2;
            last_instr_clock = 8;
        break;
        case 0x78: //ld a, b
            reg.a = reg.b;
            reg.pc += 1;
            last_instr_clock = 4;
        break;
        case 0x7c: //ld a, h
            reg.a = reg.h;
            reg.pc += 1;
            last_instr_clock = 4;
        break;
        case 0x7d: //ld a, l
            reg.a = reg.l;
            reg.pc += 1;
            last_instr_clock = 4;
        break;
        case 0xb1: //or c
            reg.a |= reg.c;
            reg.f = 0;
            if(reg.a == 0x00){
                reg.flags.z = 1;
            }
            reg.pc += 1;
            last_instr_clock = 4;
        break;
        case 0xc1: //pop bc
            reg.bc = pop();
            reg.pc += 1;
            last_instr_clock = 12;
            break;
        case 0xc3: //jmp
            reg.pc = read_mem_u16(reg.pc + 1);
            last_instr_clock = 16;
        break;
        case 0xc5: //push bc
            push(reg.bc);
            reg.pc += 1;
            last_instr_clock = 16;
        break;
        case 0xc9: //ret
            reg.pc = pop();
            last_instr_clock = 16;
        break;
        case 0xcd: //call u16
            push(reg.pc + 3); //store next instruction address
            reg.pc = read_mem_u16(reg.pc + 1);
            last_instr_clock = 24;
        break;
        case 0xe0: //ld (0xff00 + u8), A
            write_mem(0xff00 + read_mem(reg.pc + 1), reg.a);
            reg.pc += 2;
            last_instr_clock = 12;
        break;
        case 0xe1: //pop hl
            reg.hl = pop();
            reg.pc += 1;
            last_instr_clock = 12;
        break;
        case 0xe5: //push hl
            push(reg.hl);
            reg.pc += 1;
            last_instr_clock = 16;
        break;
        case 0xea: //ld (u16), a
            addr = read_mem_u16(reg.pc + 1);
            write_mem(addr, reg.a);
            reg.pc += 3;
            last_instr_clock = 16;
        break;
        case 0xf0: //ld a, (0xff00 + u8)
            u8a = read_mem(reg.pc + 1);
            reg.a = read_mem(0xff00 + u8a);
            reg.pc += 2;
            last_instr_clock = 12;
            printf("a: 0x%04x, c: 0x%04x\n", reg.a, reg.c);
        break;
        case 0xf1: //pop af
            reg.af = pop();
            reg.pc += 1;
            reg.f = 0;

            //TODO: POPのときのフラグの仕様わからん
            if(reg.a == 0) reg.flags.z = 0;
            last_instr_clock = 12;
        break;
        case 0xf3: //di
            write_mem(0xffff, 0x00);
            reg.pc += 1;
            last_instr_clock = 4;
        break;
        case 0xf5: //push af
            push(reg.af);
            reg.pc += 1;
            last_instr_clock = 16;
        break;
        case 0xfa: //ld a, (u16)
            addr = read_mem_u16(reg.pc + 2);
            reg.a = read_mem(addr);
            reg.pc += 3;
            last_instr_clock = 16;
        break;
        case 0xfe: //cp a, u8
            u8a = read_mem(reg.pc + 1);
            u8b = (reg.a & 0x8) >> 3; // bit 4 before subtract
            u8c = ((reg.a - u8a) & 0x8) >> 3; // bit 4 after subtract

            //減産前後でbit 4の符号が変わってればhalf carry立てていい？

            reg.f = 0;
            reg.flags.n = 1;
            if(reg.a == u8a) reg.flags.z = 1;
            if(reg.a < u8a) reg.flags.c = 1;
            if(u8b != u8c) reg.flags.h = 0; //TODO: わからん

            reg.pc += 2;
            last_instr_clock = 8;
        break;
        default:
            stop = true;
        break;
    }
    if (debug_step_exec) {
        mtx_stop.lock();
        stop = true;
        mtx_stop.unlock();
    }
}

bool GBEmu::is_break(uint16_t addr) {
    if(debug_break){
        for (int i = 0; i < BREAK_POINT_MAX; i++) {
            if (debug_break_addr[i] != 0 && debug_break_addr[i] == addr) {
                return true;
            }
        }
    }
    return false;
}