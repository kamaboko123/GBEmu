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
    if (read_mem(reg.pc) == 0xcb) {
        printf("0x%04x: [0xCB] 0x%02x\n", reg.pc, read_mem(reg.pc + 1));
    }
    else {
        printf("0x%04x: 0x%02x\n", reg.pc, read_mem(reg.pc));
    }
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
        case 0x02: //ld (bc), a
            write_mem(reg.bc, a);
            reg.pc += 1;
            last_instr_clock = 8;
            break;
        case 0x03: //inc bc
            reg.bc++;
            reg.pc += 1;
            last_instr_clock = 8;
            break;
        case 0x04: //inc b
            u8a = reg.b;
            reg.b++;
            reg.flags.n = 0;
            reg.flags.z = (reg.b == 0) ? 1 : 0;
            reg.flags.h = half_carry_sub(u8a, 1) ? 1 : 0;
            reg.pc += 1;
            last_instr_clock = 4;
            break;
        case 0x05: //dec b
            u8a = reg.b;
            reg.b--;

            reg.flags.n = 1;
            reg.flags.z = (reg.b == 0) ? 1 : 0;
            reg.flags.h = half_carry_sub(u8a, 1) ? 1 : 0;

            reg.pc += 1;
            last_instr_clock = 4;
            break;
        case 0x06: //ld b, u8
            reg.b = read_mem(reg.pc + 1);
            reg.pc += 2;
            last_instr_clock = 8;
            break;
        case 0x0e: //ld c, u8
            reg.c = read_mem(reg.pc + 1);
            reg.pc += 2;
            last_instr_clock = 8;
            break;
        case 0x11: //ld de, u16
            reg.de = read_mem_u16(reg.pc + 1);
            reg.pc += 3;
            last_instr_clock = 12;
            break;
        case 0x13: //inc de
            reg.de++;
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
        case 0x1a: //ld a, (de)
            reg.a = read_mem(reg.de);
            reg.pc += 1;
            last_instr_clock = 8;
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
        case 0x22: //ldi (hl), a
            write_mem(reg.hl, reg.a);
            reg.hl++;
            reg.pc += 1;
            last_instr_clock = 8;
            break;
        case 0x23: //inc hl
            reg.hl++;
            reg.pc += 1;
            last_instr_clock = 8;
        break;
        case 0x24: //inc h
            u8a = reg.h;
            reg.h++;
            reg.pc += 1;

            reg.flags.n = 0;
            reg.flags.z = (reg.h == 0) ? 1 : 0;
            reg.flags.h = half_carry_add(u8a, 1) ? 1 : 0;

            last_instr_clock = 4;
            break;
        case 0x26: //ld h, u8
            reg.h = read_mem(reg.pc + 1);
            reg.pc += 2;

            last_instr_clock = 8;
            break;
        case 0x28: //jr z, i8
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
        case 0x2a: //ldi, a (hl)
            reg.a = read_mem(reg.hl);
            reg.hl++;
            reg.pc += 1;
            last_instr_clock = 8;
        break;
        case 0x2c: //inc l
            u8a = reg.l;
            reg.l++;
            reg.pc += 1;

            reg.flags.n = 0;
            reg.flags.z = (reg.l == 0) ? 1 : 0;
            reg.flags.h = half_carry_add(u8a, 1) ? 1 : 0;

            last_instr_clock = 4;
            break;
        case 0x2d: //dec l
            u8a = reg.l;
            reg.l--;
            reg.pc += 1;

            reg.flags.n = 1;
            reg.flags.z = (reg.l == 0) ? 1 : 0;
            reg.flags.h = half_carry_sub(u8a, 1) ? 1 : 0;

            last_instr_clock = 4;
            break;
        case 0x31: //ld sp, u16
            reg.sp = read_mem_u16(reg.pc + 1);
            reg.pc += 3;
            last_instr_clock = 12;
        break;
        case 0x32: //ldd (hl), a
            write_mem(reg.hl, reg.a);
            reg.hl--;
            reg.pc += 1;
            last_instr_clock = 8;
            break;
        case 0x3e: //ld A, u8
            reg.a = read_mem(reg.pc + 1);
            reg.pc += 2;
            last_instr_clock = 8;
        break;
        case 0x46: //ld b, (hl)
            reg.b = read_mem(reg.hl);
            reg.pc += 1;
            last_instr_clock = 8;
            break;
        case 0x4e: //ld c, (hl)
            reg.c = read_mem(reg.hl);
            reg.pc += 1;
            last_instr_clock = 8;
            break;
        case 0x56: //ld d, (hl)
            reg.d = read_mem(reg.hl);
            reg.pc += 1;
            last_instr_clock = 8;
            break;
        case 0x77: //ld (hl), a
            write_mem(reg.hl, reg.a);
            reg.pc += 1;
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
        case 0xa9: //xor a, c
            reg.a ^= reg.c;
            reg.f = 0;
            reg.flags.z = (reg.a == 0) ? 1 : 0;
            reg.pc += 1;
            last_instr_clock = 4;
            break;
        case 0xae: //xor a, (hl)
            reg.a ^= read_mem(reg.hl);
            reg.f = 0;
            reg.flags.z = (reg.a == 0) ? 1 : 0;
            reg.pc += 1;
            last_instr_clock = 8;
            break;
        case 0xb1: //or c
            reg.a |= reg.c;
            reg.f = 0;
            reg.flags.z = (reg.a == 0) ? 1 : 0;
            reg.pc += 1;
            last_instr_clock = 4;
        break;
        case 0xb7: //or a, a
            reg.a |= reg.a;
            reg.f = 0;
            reg.flags.z = (reg.a == 0) ? 1 : 0;
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
        case 0xc4: //call nz, u16
            addr = read_mem_u16(reg.pc + 1);
            reg.pc += 3;
            if (reg.flags.z == 0) {
                push(reg.pc);
                reg.pc = addr;
                last_instr_clock = 24;
            }
            else {
                last_instr_clock = 16;
            }
            break;
        case 0xc5: //push bc
            push(reg.bc);
            reg.pc += 1;
            last_instr_clock = 16;
        break;
        case 0xc6: //add A, u8
            u8a = reg.a;
            u8b = read_mem(reg.pc + 1);
            reg.a += u8b;
            reg.pc += 2;

            reg.flags.z = (reg.a == 0) ? 1 : 0;
            reg.flags.n = 0;
            reg.flags.h = half_carry_add(u8a, u8b) ? 1 : 0;
            reg.flags.c = carry_add(u8a, u8b) ? 1 : 0;

            last_instr_clock = 8;
            break;
        case 0xc9: //ret
            reg.pc = pop();
            last_instr_clock = 16;
        break;
        case 0xcb: //ret
            instrs_pfx_0xcb();
            break;
        case 0xcd: //call u16
            push(reg.pc + 3); //store next instruction address
            reg.pc = read_mem_u16(reg.pc + 1);
            last_instr_clock = 24;
        break;
        case 0xd5: //push de
            push(reg.de);
            reg.pc += 1;
            last_instr_clock = 16;
            break;
        case 0xd6: //sub a, u8
            u8a = reg.a;
            u8b = read_mem(reg.pc + 1);
            reg.a -= u8b;
            reg.pc += 2;

            reg.flags.z = (reg.a == 0) ? 1 : 0;
            reg.flags.n = 1;
            reg.flags.h = half_carry_sub(u8a, u8b) ? 1 : 0;
            reg.flags.c = carry_sub(u8a, u8b) ? 1 : 0;

            last_instr_clock = 8;
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
        case 0xe6: //and a, u8
            reg.a &= read_mem(reg.pc + 1);
            reg.f = 0;
            reg.flags.h = 1;
            reg.flags.z = (reg.a == 0) ? 1 : 0;
            reg.pc += 2;
            last_instr_clock = 8;
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
        break;
        case 0xf1: //pop af
            reg.af = pop();
            reg.pc += 1;
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

            reg.flags.n = 1;
            reg.flags.z = (reg.a == u8a) ? 1 : 0;
            reg.flags.c = (reg.a < u8a) ? 1 : 0;
            reg.flags.h = half_carry_sub(reg.a, u8a) ? 1 : 0;

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

void GBEmu::instrs_pfx_0xcb(void) {
    reg.pc += 1;
    switch (read_mem(reg.pc)) {
    case 0x38://srl b
        reg.f = 0;
        reg.flags.c = clib::getBit(reg.b, 0);
        reg.b = reg.b >> 1;
        reg.flags.z = (reg.b == 0) ? 1 : 0;

        reg.pc += 1;
        last_instr_clock = 8;
        break;
    default:
        reg.pc--;
        stop = true;
        break;
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

inline bool GBEmu::half_carry_add(uint8_t a, uint8_t b) {
    //下位4bitを取り出したものを計算し、bit4を取り出して繰り上がったかを判断する
    return (((a & 0x0f) + (b & 0x0f)) & 0x10) == 0x10;
}

inline bool GBEmu::half_carry_sub(uint8_t a, uint8_t b) {
    return (((a & 0x0f) - (b & 0x0f)) & 0x10) == 0x10;
}

inline bool GBEmu::carry_add(uint8_t a, uint8_t b) {
    //REVIEW
    //下位8bitを取り出したものを計算し、bit8を取り出して繰り上がったかを判断する
    return ((((uint16_t)a & 0xff) + ((uint16_t)b & 0xff)) & 0x100) == 0x100;
}

inline bool GBEmu::carry_sub(uint8_t a, uint8_t b) {
    //REVIEW
    //下位8bitを取り出したものを計算し、bit8を取り出して繰り上がったかを判断する
    return ((((uint16_t)a & 0xff) - ((uint16_t)b & 0xff)) & 0x100) == 0x100;
}
