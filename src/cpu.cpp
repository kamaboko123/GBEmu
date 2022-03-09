#include "gbemu.hpp"

/* 参考
    https://izik1.github.io/gbops/
    http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf
*/

void GBEmu::cpu_step(){
    uint16_t addr;
    int8_t i8a;
    uint8_t u8a, u8b;

    bool is_branch = false;
    uint8_t opcode = 0x00;
    bool is_cb_prefix = false;

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
    opcode = read_mem(reg.pc);
    switch(opcode){
        case 0x00: //nop
            reg.pc += 1;
            break;
        case 0x01: //ld bc, u16
            _cpu_ld_r16_imm16(&reg.bc);
            break;
        case 0x02: //ld (bc), a
            _cpu_ld_memr16_r8(reg.bc, reg.a);
            break;
        case 0x03: //inc bc
            _cpu_inc_r16(&reg.bc);
            break;
        case 0x04: //inc b
            _cpu_inc_r8(&reg.b);
            break;
        case 0x05: //dec b
            _cpu_dec_r8(&reg.b);
            break;
        case 0x06: //ld b, u8
            _cpu_ld_r8_imm8(&reg.b);
            break;
        case 0x07: //rlca
            _cpu_rotate_left_carry_r8(&reg.a);
            break;
        case 0x08: //ld (u16), sp
            _cpu_ld_memimm16_r16(reg.sp);
            break;
        case 0x09: //add hl, bc
            _cpu_add_r16_r16(&reg.hl, &reg.bc);
            break;
        case 0x0a: //ld a, (bc)
            _cpu_ld_r8_memr16(&reg.a, reg.bc);
            break;
        case 0x0b: //dec bc
            _cpu_dec_r16(&reg.bc);
            break;
        case 0x0c: //inc c
            _cpu_inc_r8(&reg.c);
            break;
        case 0x0d: //dec c
            _cpu_dec_r8(&reg.c);
            break;
        case 0x0e: //ld c, u8
            _cpu_ld_r8_imm8(&reg.c);
            break;
        case 0x11: //ld de, u16
            _cpu_ld_r16_imm16(&reg.de);
            break;
        case 0x12: //ld (de), a
            _cpu_ld_memr16_r8(reg.de, reg.a);
            break;
        case 0x13: //inc de
            _cpu_inc_r16(&reg.de);
            break;
        case 0x14: //inc d
            _cpu_inc_r8(&reg.d);
            break;
        case 0x15: //dec b
            _cpu_dec_r8(&reg.d);
            break;
        case 0x16: //ld b, u8
            _cpu_ld_r8_imm8(&reg.d);
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
        break;
        case 0x19: //add hl, de
            _cpu_add_r16_r16(&reg.hl, &reg.de);
            break;
        case 0x1a: //ld a, (de)
            _cpu_ld_r8_memr16(&reg.a, reg.de);
            break;
        case 0x1b: //dec de
            _cpu_dec_r16(&reg.de);
            break;
        case 0x1c: //inc e
            _cpu_inc_r8(&reg.e);
            break;
        case 0x1d: //dec e
            _cpu_dec_r8(&reg.e);
            break;
        case 0x1e: //ld e, u8
            _cpu_ld_r8_imm8(&reg.e);
            break;
        case 0x20: //jr if not zero i8
            i8a = read_mem(reg.pc + 1);
            reg.pc += 2;
            if(!reg.flags.z){
                reg.pc += i8a;
                is_branch = true;
            }
        break;
        case 0x21: //ld hl, u16
            _cpu_ld_r16_imm16(&reg.hl);
        break;
        case 0x22: //ldi (hl), a
            write_mem(reg.hl, reg.a);
            reg.hl++;
            reg.pc += 1;
            break;
        case 0x23: //inc hl
            _cpu_inc_r16(&reg.hl);
        break;
        case 0x24: //inc h
            _cpu_inc_r8(&reg.h);
            break;
        case 0x25: //dec h
            _cpu_dec_r8(&reg.h);
            break;
        case 0x26: //ld h, u8
            _cpu_ld_r8_imm8(&reg.b);
            break;
        case 0x28: //jr z, i8
            i8a = read_mem(reg.pc + 1);
            reg.pc += 2;
            if(reg.flags.z){
                reg.pc += i8a;
                is_branch = true;
            }
        break;
        case 0x29: //add hl, hl
            _cpu_add_r16_r16(&reg.hl, &reg.hl);
            break;
        case 0x2a: //ldi, a (hl)
            reg.a = read_mem(reg.hl);
            reg.hl++;
            reg.pc += 1;
        break;
        case 0x2b: //dec hl
            _cpu_dec_r16(&reg.hl);
            break;
        case 0x2c: //inc l
            _cpu_inc_r8(&reg.l);
            break;
        case 0x2d: //dec l
            _cpu_dec_r8(&reg.l);
            break;
        case 0x2e: //ld l, u8
            _cpu_ld_r8_imm8(&reg.l);
            break;
        case 0x31: //ld sp, u16
            _cpu_ld_r16_imm16(&reg.sp);
        break;
        case 0x32: //ldd (hl), a
            write_mem(reg.hl, reg.a);
            reg.hl--;
            reg.pc += 1;
            break;
        case 0x33: //inc sp
            _cpu_inc_r16(&reg.sp);
            break;
        case 0x39: //add hl, sp
            _cpu_add_r16_r16(&reg.hl, &reg.sp);
            break;
        case 0x3b: //dec sp
            _cpu_dec_r16(&reg.sp);
            break;
        case 0x3c: //inc a
            _cpu_inc_r8(&reg.a);
            break;
        case 0x3d: //dec a
            _cpu_dec_r8(&reg.a);
            break;
        case 0x3e: //ld a, u8
            _cpu_ld_r8_imm8(&reg.a);
            break;
        case 0x46: //ld b, (hl)
            reg.b = read_mem(reg.hl);
            reg.pc += 1;
            break;
        case 0x4e: //ld c, (hl)
            reg.c = read_mem(reg.hl);
            reg.pc += 1;
            break;
        case 0x56: //ld d, (hl)
            reg.d = read_mem(reg.hl);
            reg.pc += 1;
            break;
        case 0x77: //ld (hl), a
            write_mem(reg.hl, reg.a);
            reg.pc += 1;
            break;
        case 0x78: //ld a, b
            reg.a = reg.b;
            reg.pc += 1;
        break;
        case 0x7c: //ld a, h
            reg.a = reg.h;
            reg.pc += 1;
        break;
        case 0x7d: //ld a, l
            reg.a = reg.l;
            reg.pc += 1;
        break;
        case 0xa9: //xor a, c
            reg.a ^= reg.c;
            reg.f = 0;
            reg.flags.z = (reg.a == 0);
            reg.pc += 1;
            break;
        case 0xae: //xor a, (hl)
            reg.a ^= read_mem(reg.hl);
            reg.f = 0;
            reg.flags.z = (reg.a == 0);
            reg.pc += 1;
            break;
        case 0xb1: //or c
            reg.a |= reg.c;
            reg.f = 0;
            reg.flags.z = (reg.a == 0);
            reg.pc += 1;
        break;
        case 0xb7: //or a, a
            reg.a |= reg.a;
            reg.f = 0;
            reg.flags.z = (reg.a == 0);
            reg.pc += 1;
            break;
        case 0xc1: //pop bc
            reg.bc = pop();
            reg.pc += 1;
            break;
        case 0xc3: //jmp
            reg.pc = read_mem_u16(reg.pc + 1);
        break;
        case 0xc4: //call nz, u16
            addr = read_mem_u16(reg.pc + 1);
            reg.pc += 3;
            if (reg.flags.z == 0) {
                push(reg.pc);
                reg.pc = addr;
                is_branch = true;
            }
            break;
        case 0xc5: //push bc
            push(reg.bc);
            reg.pc += 1;
        break;
        case 0xc6: //add A, u8
            u8a = reg.a;
            u8b = read_mem(reg.pc + 1);
            reg.a += u8b;
            reg.pc += 2;

            reg.flags.z = (reg.a == 0);
            reg.flags.n = 0;
            reg.flags.h = half_carry_add(u8a, u8b);
            reg.flags.c = carry_add(u8a, u8b);

            break;
        case 0xc9: //ret
            reg.pc = pop();
        break;
        case 0xcb: //ret
            is_cb_prefix = true;
            instrs_pfx_0xcb(&opcode);
            break;
        case 0xcd: //call u16
            push(reg.pc + 3); //store next instruction address
            reg.pc = read_mem_u16(reg.pc + 1);
        break;
        case 0xd5: //push de
            push(reg.de);
            reg.pc += 1;
            break;
        case 0xd6: //sub a, u8
            u8a = reg.a;
            u8b = read_mem(reg.pc + 1);
            reg.a -= u8b;
            reg.pc += 2;

            reg.flags.z = (reg.a == 0);
            reg.flags.n = 1;
            reg.flags.h = half_carry_sub(u8a, u8b);
            reg.flags.c = carry_sub(u8a, u8b);

            break;
        case 0xe0: //ld (0xff00 + u8), A
            write_mem(0xff00 + read_mem(reg.pc + 1), reg.a);
            reg.pc += 2;
        break;
        case 0xe1: //pop hl
            reg.hl = pop();
            reg.pc += 1;
        break;
        case 0xe5: //push hl
            push(reg.hl);
            reg.pc += 1;
        break;
        case 0xe6: //and a, u8
            reg.a &= read_mem(reg.pc + 1);
            reg.f = 0;
            reg.flags.h = 1;
            reg.flags.z = (reg.a == 0);
            reg.pc += 2;
        break;
        case 0xea: //ld (u16), a
            addr = read_mem_u16(reg.pc + 1);
            write_mem(addr, reg.a);
            reg.pc += 3;
        break;
        case 0xf0: //ld a, (0xff00 + u8)
            u8a = read_mem(reg.pc + 1);
            reg.a = read_mem(0xff00 + u8a);
            reg.pc += 2;
        break;
        case 0xf1: //pop af
            reg.af = pop();
            reg.pc += 1;
        break;
        case 0xf3: //di
            write_mem(0xffff, 0x00);
            reg.pc += 1;
        break;
        case 0xf5: //push af
            push(reg.af);
            reg.pc += 1;
        break;
        case 0xfa: //ld a, (u16)
            addr = read_mem_u16(reg.pc + 2);
            reg.a = read_mem(addr);
            reg.pc += 3;
        break;
        case 0xfe: //cp a, u8
            u8a = read_mem(reg.pc + 1);

            reg.flags.n = 1;
            reg.flags.z = (reg.a == u8a);
            reg.flags.c = (reg.a < u8a);
            reg.flags.h = half_carry_sub(reg.a, u8a);

            reg.pc += 2;
        break;
        default:
            stop = true;
        break;
    }
    if (is_cb_prefix) {
        last_instr_clock = cycle_pfx_cb[opcode];
    }
    else {
        last_instr_clock = cycle_nopfx[is_branch][opcode];
        printf("!!%d\n", cycle_nopfx[0][0]);
    }
    printf("cycle: %d\n", last_instr_clock);

    if (debug_step_exec) {
        mtx_stop.lock();
        stop = true;
        mtx_stop.unlock();
    }
}

void GBEmu::instrs_pfx_0xcb(uint8_t* opcode) {
    reg.pc += 1;
    *opcode = read_mem(reg.pc);
    switch (*opcode) {
    case 0x38://srl b
        reg.f = 0;
        reg.flags.c = clib::getBit(reg.b, 0);
        reg.b = reg.b >> 1;
        reg.flags.z = (reg.b == 0);

        reg.pc += 1;
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

inline bool GBEmu::half_carry_add_u16(uint16_t a, uint16_t b) {
    return (((a & 0x7ff) + (b & 0x7ff)) & 0x0800) == 0x0800;
}

inline bool GBEmu::carry_add_u16(uint16_t a, uint16_t b) {
    return ((((uint32_t)a & 0xffff) + ((uint32_t)b & 0xffff)) & 0x10000) == 0x10000;
}

void GBEmu::_cpu_ld_r8_imm8(uint8_t* r) {
    *r = read_mem(reg.pc + 1);
    reg.pc += 2;
}

void GBEmu::_cpu_ld_r16_imm16(uint16_t* r){
    *r = read_mem_u16(reg.pc + 1);
    reg.pc += 3;
}

void GBEmu::_cpu_inc_r16(uint16_t* r) {
    (*r)++;
    reg.pc += 1;
}

void GBEmu::_cpu_dec_r16(uint16_t* r) {
    (*r)--;
    reg.pc += 1;
}

void GBEmu::_cpu_inc_r8(uint8_t* r) {
    uint8_t u8 = *r;
    (*r)++;

    reg.flags.z = (*r == 0);
    reg.flags.n = 0;
    reg.flags.h = half_carry_add(u8, 1);

    reg.pc += 1;
}

void GBEmu::_cpu_dec_r8(uint8_t* r) {
    uint8_t u8 = *r;
    (*r)--;

    reg.flags.z = (*r == 0);
    reg.flags.n = 1;
    reg.flags.h = half_carry_sub(u8, 1);

    reg.pc += 1;
}

void GBEmu::_cpu_add_r16_r16(uint16_t* r1, uint16_t *r2) {
    uint16_t u16a = *r1;
    uint16_t u16b = *r2;

    *r1 += *r2;
    reg.flags.n = 0;
    reg.flags.h = half_carry_add_u16(u16a, u16b);
    reg.flags.c = carry_add_u16(u16a, u16b);

    reg.pc += 1;
}

void GBEmu::_cpu_ld_memr16_r8(uint16_t addr, uint8_t data) {
    write_mem(addr, data);
    reg.pc += 1;
}

void GBEmu::_cpu_ld_r8_memr16(uint8_t *r, uint16_t addr) {
    *r = read_mem(addr);
    reg.pc += 1;
}

void GBEmu::_cpu_ld_memimm16_r16(uint16_t r) {
    uint16_t addr = read_mem_u16(reg.pc + 1);
    write_mem_u16(addr, r);
    reg.pc += 3;
}

void GBEmu::_cpu_rotate_left_carry_r8(uint8_t* r) {
    reg.f = 0;

    uint8_t old_c = reg.flags.c;
    reg.flags.c = clib::getBit(*r, 7);
    *r = (*r << 1) + old_c;

    reg.pc += 1;
}
