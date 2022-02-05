#include "gbemu.hpp"

uint16_t GBEmu::read_mem_u16(uint16_t addr) {
    uint16_t h, l;
    l = read_mem(addr);
    h = read_mem(addr + 1) << 8;
    return h + l;
}
void GBEmu::write_mem_u16(uint16_t addr, uint16_t data) {
    uint8_t h, l;
    h = data >> 8;
    l = data & 0xff;

    write_mem(addr, l);
    write_mem(addr + 1, h);
    //printf("[16]data:%04x h:%02x, l:%02x\n", data, h, l);
}
uint8_t GBEmu::read_mem(uint16_t addr) {
    if (0x0000 <= addr && addr <= 0x3fff) {
        // rom(bank0)
        return rom[addr];
    } else if (0x4000 <= addr && addr <= 0x7FFF) {
        // rom(bankN)
    } else if (0x8000 <= addr && addr <= 0x9FFF) {
        // 8KB vram
        return ram[addr];
    } else if (0xa000 <= addr && addr <= 0xbfff) {
        // 8KB cartridge ram
    } else if (0xc000 <= addr && addr <= 0xcfff) {
        // wram(bank0)
        return ram[addr];
    } else if (0xd000 <= addr && addr <= 0xdfff) {
        // wram(bankN)
        return ram[addr];
    } else if (0xe000 <= addr && addr <= 0xfdff) {
        // mirror 0xc000 - 0xddff
    } else if (0xfe00 <= addr && addr <= 0xfe9f) {
        // oam
    } else if (0xfea0 <= addr && addr <= 0xfeff) {
        // unused
    } else if (0xff00 <= addr && addr <= 0xff7f) {
        // I/O register
        return ram[addr];
    } else if (0xff80 <= addr && addr <= 0xfffe) {
        // HRAM
    } else if (0xffff) {
        // interrupt enable/disable
    } else {
        // none
    }
    return 0;
}
void GBEmu::write_mem(uint16_t addr, uint8_t data) {
    if (0x0000 <= addr && addr <= 0x3fff) {
        // rom(bank0)
        rom[addr] = data;
    } else if (0x4000 <= addr && addr <= 0x7FFF) {
        // rom(bankN)
    } else if (0x8000 <= addr && addr <= 0x9FFF) {
        // 8KB vram
        ram[addr] = data;
    } else if (0xa000 <= addr && addr <= 0xbfff) {
        // 8KB cartridge ram
    } else if (0xc000 <= addr && addr <= 0xcfff) {
        // wram(bank0)
        ram[addr] = data;
    } else if (0xd000 <= addr && addr <= 0xdfff) {
        // wram(bankN)
        ram[addr] = data;
    } else if (0xe000 <= addr && addr <= 0xfdff) {
        // mirror 0xc000 - 0xddff
    } else if (0xfe00 <= addr && addr <= 0xfe9f) {
        // oam
    } else if (0xfea0 <= addr && addr <= 0xfeff) {
        // unused
    } else if (0xff00 <= addr && addr <= 0xff7f) {
        // I/O register
        ram[addr] = data;
    } else if (0xff80 <= addr && addr <= 0xfffe) {
        // HRAM
    } else if (0xffff) {
        // interrupt enable/disable
    } else {
        // none
    }
}