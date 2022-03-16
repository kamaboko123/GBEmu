#include "gbemu.hpp"

/* 参考
    https://izik1.github.io/gbops/
    http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf

    TODO:
    0x10 stop
    0x27 daa
    0x76 halt
    0xc7 rst 00h
    0xcf rst 08h
*/

void GBEmu::cpu_step(){
    uint16_t addr;
    int8_t i8a;
    uint8_t u8a, u8b;

    bool is_branch = false;
    uint8_t opcode = 0x00;
    bool is_cb_prefix = false;
    bool is_jmp_ins = false;

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
            break;
        case 0x01: //ld bc, u16
            _cpu_ld_r16_imm16(&reg.bc);
            break;
        case 0x02: //ld (bc), a
            write_mem(reg.bc, reg.a);
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
            _cpu_rotate_left_r8(&reg.a);
            break;
        case 0x08: //ld (u16), sp
            _cpu_ld_memimm16_r16(reg.sp);
            break;
        case 0x09: //add hl, bc
            _cpu_add_r16_r16(&reg.hl, &reg.bc);
            break;
        case 0x0a: //ld a, (bc)
            reg.a = read_mem(reg.bc);
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
        case 0x0f: //rrca
            _cpu_rotate_right_r8(&reg.a);
            break;
        case 0x11: //ld de, u16
            _cpu_ld_r16_imm16(&reg.de);
            break;
        case 0x12: //ld (de), a
            write_mem(reg.de, reg.a);
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
        case 0x17: //rla
            _cpu_rotate_left_carry_r8(&reg.a);
            break;
        case 0x18: //jr u8
            is_jmp_ins = true;
            _cpu_jmp_relative_imm8();
            break;
        case 0x19: //add hl, de
            _cpu_add_r16_r16(&reg.hl, &reg.de);
            break;
        case 0x1a: //ld a, (de)
            reg.a = read_mem(reg.de);
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
        case 0x1f: //rra
            _cpu_rotate_right_carry_r8(&reg.a);
            break;
        case 0x20: //jr if not zero, i8
            is_jmp_ins = true;
            is_branch = _cpu_jmp_if_relative_imm8(!reg.flags.z);
            break;
        case 0x21: //ld hl, u16
            _cpu_ld_r16_imm16(&reg.hl);
            break;
        case 0x22: //ldi (hl), a
            write_mem(reg.hl++, reg.a);
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
        case 0x28: //jr if z, i8
            is_jmp_ins = true;
            is_branch = _cpu_jmp_if_relative_imm8(reg.flags.z);
            break;
        case 0x29: //add hl, hl
            _cpu_add_r16_r16(&reg.hl, &reg.hl);
            break;
        case 0x2a: //ldi, a (hl)
            reg.a = read_mem(reg.hl++);
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
        case 0x2f: //cpl
            _cpu_complement_r8(&reg.a);
            break;
        case 0x30: //jr if not carry, i8
            is_jmp_ins = true;
            is_branch = _cpu_jmp_if_relative_imm8(!reg.flags.c);
            break;
        case 0x31: //ld sp, u16
            _cpu_ld_r16_imm16(&reg.sp);
            break;
        case 0x32: //ldd (hl), a
            write_mem(reg.hl--, reg.a);
            break;
        case 0x33: //inc sp
            _cpu_inc_r16(&reg.sp);
            break;
        case 0x34: //inc (hl)
            _cpu_inc_mem(reg.hl);
            break;
        case 0x35: // dec (hl)
            _cpu_dec_mem(reg.hl);
            break;
        case 0x36: // ld (hl), u8
            _cpu_ld_mem_imm8(reg.hl);
            break;
        case 0x37: // scf
            reg.flags.n = false;
            reg.flags.h = false;
            reg.flags.c = true;
            break;
        case 0x38: //jr if c, i8
            is_jmp_ins = true;
            is_branch = _cpu_jmp_if_relative_imm8(reg.flags.c);
            break;
        case 0x39: //add hl, sp
            _cpu_add_r16_r16(&reg.hl, &reg.sp);
            break;
        case 0x3a: //ldd a, (hl)
            reg.a = read_mem(reg.hl--);
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
        case 0x3f: //ccf
            reg.flags.n = false;
            reg.flags.h = false;
            reg.flags.c = !reg.flags.c;
            break;
        case 0x40: //ld b, b
            reg.b = reg.b;
            break;
        case 0x41: //ld b, c
            reg.b = reg.c;
            break;
        case 0x42: //ld b, d
            reg.b = reg.d;
            break;
        case 0x43: //ld b, e
            reg.b = reg.e;
            break;
        case 0x44: //ld b, h
            reg.b = reg.h;
            break;
        case 0x45: //ld b, l
            reg.b = reg.l;
            break;
        case 0x46: //ld b, (hl)
            reg.b = read_mem(reg.hl);
            break;
        case 0x47: //ld b, a
            reg.b = reg.a;
            break;
        case 0x48: //ld c, b
            reg.c = reg.b;
            break;
        case 0x49: //ld c, c
            reg.c = reg.c;
            break;
        case 0x4a: //ld c, d
            reg.c = reg.d;
            break;
        case 0x4b: //ld c, e
            reg.c = reg.e;
            break;
        case 0x4c: //ld c, h
            reg.c = reg.h;
            break;
        case 0x4d: //ld c, l
            reg.c = reg.l;
            break;
        case 0x4e: //ld c, (hl)
            reg.c = read_mem(reg.hl);
            break;
        case 0x4f: //ld c, a
            reg.c = reg.a;
            break;
        case 0x50: //ld d, b
            reg.d = reg.b;
            break;
        case 0x51: //ld d, c
            reg.d = reg.c;
            break;
        case 0x52: //ld d, d
            reg.d = reg.d;
            break;
        case 0x53: //ld d, e
            reg.d = reg.e;
            break;
        case 0x54: //ld d, h
            reg.d = reg.h;
            break;
        case 0x55: //ld d, l
            reg.d = reg.l;
            break;
        case 0x56: //ld d, (hl)
            reg.d = read_mem(reg.hl);
            break;
        case 0x57: //ld d, a
            reg.d = reg.a;
            break;
        case 0x58: //ld e, b
            reg.e = reg.b;
            break;
        case 0x59: //ld e, c
            reg.e = reg.c;
            break;
        case 0x5a: //ld e, d
            reg.e = reg.d;
            break;
        case 0x5b: //ld e, e
            reg.e = reg.e;
            break;
        case 0x5c: //ld e, h
            reg.e = reg.h;
            break;
        case 0x5d: //ld e, l
            reg.e = reg.l;
            break;
        case 0x5e: //ld e, (hl)
            reg.e = read_mem(reg.hl);
            break;
        case 0x5f: //ld e, a
            reg.e = reg.a;
            break;
        case 0x60: //ld h, b
            reg.h = reg.b;
            break;
        case 0x61: //ld h, c
            reg.h = reg.c;
            break;
        case 0x62: //ld h, d
            reg.h = reg.d;
            break;
        case 0x63: //ld h, e
            reg.h = reg.e;
            break;
        case 0x64: //ld h, h
            reg.h = reg.h;
            break;
        case 0x65: //ld h, l
            reg.h = reg.l;
            break;
        case 0x66: //ld h, (hl)
            reg.h = read_mem(reg.hl);
            break;
        case 0x67: //ld h, a
            reg.h = reg.a;
            break;
        case 0x68: //ld l, b
            reg.l = reg.b;
            break;
        case 0x69: //ld l, c
            reg.l = reg.c;
            break;
        case 0x6a: //ld l, d
            reg.l = reg.d;
            break;
        case 0x6b: //ld l, e
            reg.l = reg.e;
            break;
        case 0x6c: //ld l, h
            reg.l = reg.h;
            break;
        case 0x6d: //ld l, l
            reg.l = reg.l;
            break;
        case 0x6e: //ld l, (hl)
            reg.l = read_mem(reg.hl);
            break;
        case 0x6f: //ld l, a
            reg.l = reg.a;
            break;
        case 0x70: //ld (hl), b
            write_mem(reg.hl, reg.b);
            break;
        case 0x71: //ld (hl), c
            write_mem(reg.hl, reg.c);
            break;
        case 0x72: //ld (hl), d
            write_mem(reg.hl, reg.d);
            break;
        case 0x73: //ld (hl), e
            write_mem(reg.hl, reg.e);
            break;
        case 0x74: //ld (hl), h
            write_mem(reg.hl, reg.h);
            break;
        case 0x75: //ld (hl), l
            write_mem(reg.hl, reg.l);
            break;
        case 0x77: //ld (hl), a
            write_mem(reg.hl, reg.a);
            break;
        case 0x78: //ld a, b
            reg.a = reg.b;
            break;
        case 0x79: //ld a, c
            reg.a = reg.c;
            break;
        case 0x7a: //ld a, d
            reg.a = reg.d;
            break;
        case 0x7b: //ld a, e
            reg.a = reg.e;
            break;
        case 0x7c: //ld a, h
            reg.a = reg.h;
            break;
        case 0x7d: //ld a, l
            reg.a = reg.l;
            break;
        case 0x7e: //ld a, (hl)
            reg.a = read_mem(reg.hl);
            break;
        case 0x7f: //ld a, a
            reg.a = reg.a;
            break;
        case 0x80: //add a, b
            _cpu_add_rega_r8(&reg.b);
            break;
        case 0x81: //add a, c
            _cpu_add_rega_r8(&reg.c);
            break;
        case 0x82: //add a, d
            _cpu_add_rega_r8(&reg.d);
            break;
        case 0x83: //add a, e
            _cpu_add_rega_r8(&reg.e);
            break;
        case 0x84: //add a, h
            _cpu_add_rega_r8(&reg.h);
            break;
        case 0x85: //add a, l
            _cpu_add_rega_r8(&reg.l);
            break;
        case 0x86: //add a, (hl)
            _cpu_add_rega_mem(reg.hl);
            break;
        case 0x87: //add a, a
            _cpu_add_rega_r8(&reg.a);
            break;
        case 0x88: //adc a, b
            _cpu_add_rega_r8_carry(&reg.b);
            break;
        case 0x89: //adc a, c
            _cpu_add_rega_r8_carry(&reg.c);
            break;
        case 0x8a: //adc a, d
            _cpu_add_rega_r8_carry(&reg.d);
            break;
        case 0x8b: //adc a, e
            _cpu_add_rega_r8_carry(&reg.e);
            break;
        case 0x8c: //adc a, h
            _cpu_add_rega_r8_carry(&reg.h);
            break;
        case 0x8d: //adc a, l
            _cpu_add_rega_r8_carry(&reg.l);
            break;
        case 0x8e: //adc a, (hl)
            _cpu_add_rega_mem_carry(reg.hl);
            break;
        case 0x8f: //adc a, a
            _cpu_add_rega_r8_carry(&reg.a);
            break;
        case 0x90: //sub a, b
            _cpu_sub_rega_r8(&reg.b);
            break;
        case 0x91: //sub a, c
            _cpu_sub_rega_r8(&reg.c);
            break;
        case 0x92: //sub a, d
            _cpu_sub_rega_r8(&reg.d);
            break;
        case 0x93: //sub a, e
            _cpu_sub_rega_r8(&reg.e);
            break;
        case 0x94: //sub a, h
            _cpu_sub_rega_r8(&reg.h);
            break;
        case 0x95: //sub a, l
            _cpu_sub_rega_r8(&reg.l);
            break;
        case 0x96: //sub a, (hl)
            _cpu_sub_rega_mem(reg.hl);
            break;
        case 0x97: //sub a, a
            _cpu_sub_rega_r8(&reg.a);
            break;
        case 0x98: //sbc a, b
            _cpu_sub_rega_r8_carry(&reg.b);
            break;
        case 0x99: //sbc a, c
            _cpu_sub_rega_r8_carry(&reg.c);
            break;
        case 0x9a: //sbc a, d
            _cpu_sub_rega_r8_carry(&reg.d);
            break;
        case 0x9b: //sbc a, e
            _cpu_sub_rega_r8_carry(&reg.e);
            break;
        case 0x9c: //sbc a, h
            _cpu_sub_rega_r8_carry(&reg.h);
            break;
        case 0x9d: //scbc a, l
            _cpu_sub_rega_r8_carry(&reg.l);
            break;
        case 0x9e: //sbc a, (hl)
            _cpu_sub_rega_mem_carry(reg.hl);
            break;
        case 0x9f: //sbc a, a
            _cpu_sub_rega_r8_carry(&reg.a);
            break;
        case 0xa0: //and a, b
            _cpu_and_rega_r8(&reg.b);
            break;
        case 0xa1: //and a, c
            _cpu_and_rega_r8(&reg.c);
            break;
        case 0xa2: //and a, d
            _cpu_and_rega_r8(&reg.d);
            break;
        case 0xa3: //and a, e
            _cpu_and_rega_r8(&reg.e);
            break;
        case 0xa4: //and a, h
            _cpu_and_rega_r8(&reg.h);
            break;
        case 0xa5: //and a, l
            _cpu_and_rega_r8(&reg.l);
            break;
        case 0xa6: //and a, (hl)
            _cpu_and_rega_mem(reg.hl);
            break;
        case 0xa7: //and a, a
            _cpu_and_rega_r8(&reg.a);
            break;
        case 0xa8: //xor a, b
            _cpu_xor_rega_r8(&reg.b);
            break;
        case 0xa9: //xor a, c
            _cpu_xor_rega_r8(&reg.c);
            break;
        case 0xaa: //xor a, d
            _cpu_xor_rega_r8(&reg.d);
            break;
        case 0xab: //xor a, e
            _cpu_xor_rega_r8(&reg.e);
            break;
        case 0xac: //xor a, h
            _cpu_xor_rega_r8(&reg.h);
            break;
        case 0xad: //xor a, l
            _cpu_xor_rega_r8(&reg.l);
            break;
        case 0xae: //xor a, (hl)
            _cpu_xor_rega_mem(reg.hl);
            break;
        case 0xaf: //xor a, a
            _cpu_xor_rega_r8(&reg.a);
            break;
        case 0xb0: //or a, b
            _cpu_or_rega_r8(&reg.b);
            break;
        case 0xb1: //or a, c
            _cpu_or_rega_r8(&reg.c);
            break;
        case 0xb2: //or a, d
            _cpu_or_rega_r8(&reg.d);
            break;
        case 0xb3: //or a, e
            _cpu_or_rega_r8(&reg.e);
            break;
        case 0xb4: //or a, h
            _cpu_or_rega_r8(&reg.h);
            break;
        case 0xb5: //or a, l
            _cpu_or_rega_r8(&reg.l);
            break;
        case 0xb6: //or a, (hl)
            _cpu_or_rega_mem(reg.hl);
            break;
        case 0xb7: //or a, a
            _cpu_or_rega_r8(&reg.a);
            break;
        case 0xb8: //cp a, b
            _cpu_compare_rega_r8(&reg.b);
            break;
        case 0xb9: //cp a, c
            _cpu_compare_rega_r8(&reg.c);
            break;
        case 0xba: //cp a, d
            _cpu_compare_rega_r8(&reg.d);
            break;
        case 0xbb: //cp a, e
            _cpu_compare_rega_r8(&reg.e);
            break;
        case 0xbc: //cp a, h
            _cpu_compare_rega_r8(&reg.h);
            break;
        case 0xbd: //cp a, l
            _cpu_compare_rega_r8(&reg.l);
            break;
        case 0xbe: // cp a, (hl)
            _cpu_compare_rega_mem(reg.hl);
            break;
        case 0xbf: //cp a, a
            _cpu_compare_rega_r8(&reg.a);
            break;
        case 0xc0: //ret if not zero
            is_jmp_ins = true;
            is_branch = _cpu_ret_if(!reg.flags.z);
            break;
        case 0xc1: //pop bc
            reg.bc = pop();
            break;
        case 0xc2: //jmp if not zero u16
            is_jmp_ins = true;
            is_branch = _cpu_jmp_if_imm16(!reg.flags.z);
            break;
        case 0xc3: //jmp
            is_jmp_ins = true;
            reg.pc = read_mem_u16(reg.pc + 1);
            break;
        case 0xc4: //call nz, u16
            is_jmp_ins = true;
            is_branch = _cpu_call_if_imm16(!reg.flags.z);
            break;
        case 0xc5: //push bc
            push(reg.bc);
            break;
        case 0xc6: //add A, u8
            /*
            u8a = reg.a;
            u8b = read_mem(reg.pc + 1);
            reg.a += u8b;

            reg.flags.z = (reg.a == 0);
            reg.flags.n = 0;
            reg.flags.h = half_carry_add(u8a, u8b);
            reg.flags.c = carry_add(u8a, u8b);
            */
            _cpu_add_rega_mem(reg.pc + 1);
            break;
        case 0xc8:
            is_jmp_ins = true;
            is_branch = _cpu_ret_if(reg.flags.z);
            break;
        case 0xc9: //ret
            is_jmp_ins = true;
            reg.pc = pop();
            break;
        case 0xca:
            is_jmp_ins = true;
            is_branch = _cpu_jmp_if_imm16(reg.flags.z);
        case 0xcb: //extend instructions
            is_cb_prefix = true;
            instrs_pfx_0xcb(&opcode);
            break;
        case 0xcc:
            is_jmp_ins = true;
            is_branch = _cpu_call_if_imm16(reg.flags.z);
            break;
        case 0xcd: //call u16
            is_jmp_ins = true;
            push(reg.pc + 3); //store next instruction address
            reg.pc = read_mem_u16(reg.pc + 1);
            break;
        case 0xce:
            _cpu_add_rega_mem_carry(reg.pc + 1);
            break;
        case 0xd1: //pop de
            reg.de = pop();
            break;
        case 0xd5: //push de
            push(reg.de);
            break;
        case 0xd6: //sub a, u8
            u8a = reg.a;
            u8b = read_mem(reg.pc + 1);
            reg.a -= u8b;

            reg.flags.z = (reg.a == 0);
            reg.flags.n = true;
            reg.flags.h = half_carry_sub(u8a, u8b);
            reg.flags.c = carry_sub(u8a, u8b);

            break;
        case 0xe0: //ld (0xff00 + u8), A
            write_mem(0xff00 + read_mem(reg.pc + 1), reg.a);
            break;
        case 0xe1: //pop hl
            reg.hl = pop();
            break;
        case 0xe5: //push hl
            push(reg.hl);
            break;
        case 0xe6: //and a, u8
            reg.a &= read_mem(reg.pc + 1);
            reg.f = 0;
            reg.flags.h = 1;
            reg.flags.z = (reg.a == 0);
            break;
        case 0xea: //ld (u16), a
            addr = read_mem_u16(reg.pc + 1);
            write_mem(addr, reg.a);
            break;
        case 0xf0: //ld a, (0xff00 + u8)
            u8a = read_mem(reg.pc + 1);
            reg.a = read_mem(0xff00 + u8a);
            break;
        case 0xf1: //pop af
            reg.af = pop();
            break;
        case 0xf3: //di
            write_mem(0xffff, 0x00);
            break;
        case 0xf5: //push af
            push(reg.af);
            break;
        case 0xfa: //ld a, (u16)
            addr = read_mem_u16(reg.pc + 2);
            reg.a = read_mem(addr);
            break;
        case 0xfe: //cp a, u8
            u8a = read_mem(reg.pc + 1);

            reg.flags.n = 1;
            reg.flags.z = (reg.a == u8a);
            reg.flags.c = (reg.a < u8a);
            reg.flags.h = half_carry_sub(reg.a, u8a);
            break;
        default:
            stop = true;
            break;
    }

    //update clock and program counter
    if (!stop) {
        if (is_cb_prefix) {
            last_instr_clock = cycle_pfx_cb[opcode];
            if (!is_jmp_ins) {
                reg.pc += length_pfx_cb[opcode];
            }
        }
        else {
            last_instr_clock = cycle_nopfx[is_branch][opcode];
            if (!is_jmp_ins) {
                reg.pc += length_nopfx[opcode];
            }
        }
    }

    if (debug_step_exec) {
        mtx_stop.lock();
        stop = true;
        mtx_stop.unlock();
    }
}

void GBEmu::instrs_pfx_0xcb(uint8_t* opcode) {
    *opcode = read_mem(reg.pc + 1);
    switch (*opcode) {
    case 0x38://srl b
        reg.f = 0;
        reg.flags.c = clib::getBit(reg.b, 0);
        reg.b = reg.b >> 1;
        reg.flags.z = (reg.b == 0);
        break;
    default:
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
}

void GBEmu::_cpu_ld_r16_imm16(uint16_t* r){
    *r = read_mem_u16(reg.pc + 1);
}

void GBEmu::_cpu_inc_r16(uint16_t* r) {
    (*r)++;
}

void GBEmu::_cpu_dec_r16(uint16_t* r) {
    (*r)--;
}

void GBEmu::_cpu_inc_r8(uint8_t* r) {
    uint8_t u8 = *r;
    (*r)++;

    reg.flags.z = (*r == 0);
    reg.flags.n = false;
    reg.flags.h = half_carry_add(u8, 1);
}

void GBEmu::_cpu_dec_r8(uint8_t* r) {
    uint8_t u8 = *r;
    (*r)--;

    reg.flags.z = (*r == 0);
    reg.flags.n = true;
    reg.flags.h = half_carry_sub(u8, 1);
}

void GBEmu::_cpu_add_r16_r16(uint16_t* r1, uint16_t *r2) {
    uint16_t u16a = *r1;
    uint16_t u16b = *r2;

    *r1 += *r2;
    reg.flags.n = false;
    reg.flags.h = half_carry_add_u16(u16a, u16b);
    reg.flags.c = carry_add_u16(u16a, u16b);
}

void GBEmu::_cpu_ld_memimm16_r16(uint16_t r) {
    uint16_t addr = read_mem_u16(reg.pc + 1);
    write_mem_u16(addr, r);
}

void GBEmu::_cpu_rotate_left_carry_r8(uint8_t* r) {
    uint8_t old_c = reg.flags.c;
    reg.f = 0;
    reg.flags.c = clib::getBit(*r, 7);
    *r = (*r << 1) + old_c;
}

void GBEmu::_cpu_rotate_right_carry_r8(uint8_t* r) {
    uint8_t old_c = reg.flags.c;
    reg.f = 0;
    reg.flags.c = clib::getBit(*r, 0);
    *r = (*r >> 1) + (old_c << 7);
}

void GBEmu::_cpu_rotate_left_r8(uint8_t* r) {
    reg.f = 0;
    reg.flags.c = clib::getBit(*r, 7);
    *r = (*r << 1) + reg.flags.c;
}

void GBEmu::_cpu_rotate_right_r8(uint8_t* r) {
    reg.f = 0;
    reg.flags.c = clib::getBit(*r, 0);
    *r = (*r >> 1) + (reg.flags.c << 7);
}

bool GBEmu::_cpu_jmp_if_relative_imm8(bool flg) {
    int8_t i8a = (int8_t)read_mem(reg.pc + 1);
    reg.pc += 2;
    if (flg) {
        reg.pc += i8a;
        return true;
    }
    return false;
}

void GBEmu::_cpu_complement_r8(uint8_t *r) {
    *r = ~*r;
    reg.flags.n = true;
    reg.flags.h = true;
}

void GBEmu::_cpu_inc_mem(uint16_t addr) {
    uint8_t u8a = read_mem(addr);
    write_mem(addr, u8a + 1);

    reg.flags.z = ((u8a + 1) == 0);
    reg.flags.n = false;
    reg.flags.h = half_carry_add(u8a, 1);
}

void GBEmu::_cpu_dec_mem(uint16_t addr) {
    uint8_t u8a = read_mem(addr);
    write_mem(addr, u8a - 1);

    reg.flags.z = ((u8a - 1) == 0);
    reg.flags.n = false;
    reg.flags.h = half_carry_sub(u8a, 1);
}

void GBEmu::_cpu_ld_mem_imm8(uint16_t addr) {
    write_mem(addr, read_mem(reg.pc + 1));
}

void GBEmu::_cpu_jmp_relative_imm8(void) {
    int i8a = read_mem(reg.pc + 1);
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
}

void GBEmu::_cpu_add_rega_r8(uint8_t* r) {
    uint8_t u8a = reg.a;
    reg.a += *r;
    reg.flags.z = (reg.a == 0);
    reg.flags.n = false;
    reg.flags.h = half_carry_add(u8a, *r);
    reg.flags.c = carry_add(u8a, *r);
}

void GBEmu::_cpu_add_rega_r8_carry(uint8_t* r) {
    uint8_t u8a = reg.a;
    uint8_t u8b = *r + reg.flags.c;
    reg.a += u8b;
    reg.flags.z = (reg.a == 0);
    reg.flags.n = false;
    reg.flags.h = half_carry_add(u8a, u8b);
    reg.flags.c = carry_add(u8a, u8b);
}

void GBEmu::_cpu_add_rega_mem(uint16_t addr) {
    uint8_t u8a = reg.a;
    uint8_t u8b = read_mem(addr);
    reg.a += u8b;
    reg.flags.z = (reg.a == 0);
    reg.flags.n = false;
    reg.flags.h = half_carry_add(u8a, u8b);
    reg.flags.c = carry_add(u8a, u8b);
}

void GBEmu::_cpu_add_rega_mem_carry(uint16_t addr) {
    uint8_t u8a = reg.a;
    uint8_t u8b = read_mem(addr) + reg.flags.c;
    reg.a += u8b;
    reg.flags.z = (reg.a == 0);
    reg.flags.n = false;
    reg.flags.h = half_carry_add(u8a, u8b);
    reg.flags.c = carry_add(u8a, u8b);
}

void GBEmu::_cpu_sub_rega_r8(uint8_t* r) {
    uint8_t u8a = reg.a;
    reg.a -= *r;

    reg.flags.z = (reg.a == 0);
    reg.flags.n = true;
    reg.flags.h = half_carry_sub(u8a, *r);
    reg.flags.c = carry_sub(u8a, *r);
}

void GBEmu::_cpu_sub_rega_mem(uint16_t addr) {
    uint8_t u8a = reg.a;
    uint8_t u8b = read_mem(addr);
    reg.a -= u8b;

    reg.flags.z = (reg.a == 0);
    reg.flags.n = true;
    reg.flags.h = half_carry_sub(u8a, u8b);
    reg.flags.c = carry_sub(u8a, u8b);
}

void GBEmu::_cpu_sub_rega_r8_carry(uint8_t* r) {
    uint8_t u8a = reg.a;
    uint8_t u8b = *r + reg.flags.c;
    reg.a -= u8b;

    reg.flags.z = (reg.a == 0);
    reg.flags.n = true;
    reg.flags.h = half_carry_sub(u8a, u8b);
    reg.flags.c = carry_sub(u8a, u8b);
}

void GBEmu::_cpu_sub_rega_mem_carry(uint16_t addr) {
    uint8_t u8a = reg.a;
    uint8_t u8b = read_mem(addr) + reg.flags.c;
    reg.a -= u8b;

    reg.flags.z = (reg.a == 0);
    reg.flags.n = true;
    reg.flags.h = half_carry_sub(u8a, u8b);
    reg.flags.c = carry_sub(u8a, u8b);
}

void GBEmu::_cpu_and_rega_r8(uint8_t* r) {
    reg.a &= *r;
    reg.flags.z = (reg.a == 0);
    reg.flags.n = false;
    reg.flags.h = true;
    reg.flags.c = false;
}

void GBEmu::_cpu_and_rega_mem(uint16_t addr) {
    reg.a &= read_mem(addr);
    reg.flags.z = (reg.a == 0);
    reg.flags.n = false;
    reg.flags.h = true;
    reg.flags.c = false;
}

void GBEmu::_cpu_xor_rega_r8(uint8_t* r) {
    reg.a ^= *r;
    reg.f = 0;
    reg.flags.z = (reg.a == 0);
}

void GBEmu::_cpu_xor_rega_mem(uint16_t addr) {
    reg.a ^= read_mem(addr);
    reg.f = 0;
    reg.flags.z = (reg.a == 0);
}


void GBEmu::_cpu_or_rega_r8(uint8_t* r) {
    reg.a |= *r;
    reg.f = 0;
    reg.flags.z = (reg.a == 0);
}

void GBEmu::_cpu_or_rega_mem(uint16_t addr) {
    reg.a |= read_mem(addr);
    reg.f = 0;
    reg.flags.z = (reg.a == 0);
}

void GBEmu::_cpu_compare_rega_r8(uint8_t* r) {
    reg.flags.z = (reg.a == *r);
    reg.flags.n = true;
    reg.flags.h = half_carry_sub(reg.a, *r); //REVIEW
    reg.flags.c = (reg.a < *r);
}

void GBEmu::_cpu_compare_rega_mem(uint16_t addr) {
    uint8_t u8a = read_mem(addr);
    reg.flags.z = (reg.a == u8a);
    reg.flags.n = true;
    reg.flags.h = half_carry_sub(reg.a, u8a); //REVIEW
    reg.flags.c = (reg.a < u8a);
}

bool GBEmu::_cpu_ret_if(bool flg) {
    reg.pc += 1;
    if (flg) {
        reg.pc = pop();
    }
    return flg;
}

bool GBEmu::_cpu_jmp_if_imm16(bool flg) {
    if (flg) {
        reg.pc = read_mem_u16(reg.pc + 1);
    }
    else {
        reg.pc += 3;
    }
    return flg;
}

bool GBEmu::_cpu_call_if_imm16(bool flg) {
    if (flg) {
        push(reg.pc + 3);
        reg.pc = read_mem_u16(reg.pc + 1);
    }
    else {
        reg.pc += 3;
    }
    return flg;
}

