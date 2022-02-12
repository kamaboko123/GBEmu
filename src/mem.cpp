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
        if (mbc_state.type == MBC::MBC1) {
            //REVIEW: not tested
            uint32_t offset = mbc_state.rom_bank_n * ROM_BANK_SIZE;
            return rom[offset + addr - 0x4000];
        }
    } else if (0x8000 <= addr && addr <= 0x9FFF) {
        // 8KB vram
        return ram[addr];
    } else if (0xa000 <= addr && addr <= 0xbfff) {
        // 8KB cartridge ram
        if (mbc_state.type == MBC::MBC1) {
            uint32_t offset = mbc_state.ram_bank_n * RAM_BANK_SIZE;
            return *(ram_bank + offset + addr - 0xa000);
        }
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
        // REVIEW: MBC0,1,2これでいい？
        if (mbc_state.type == MBC::MBC1) {
            if (0x0000 <= addr && addr <= 0x1fff) { //RAM有効化
                mbc_state.ram_enable = (data == 0x0a ? true : false);
            }
            else if (0x2000 <= addr && addr <= 0x3fff) { //romバンク指定(下位5bit)
                // REVIEW: not tested
                // バンクの下位5bitを指定する、上位2bitはセカンダリバンクレジスタ(0x4000-0x5fff)で制御される
                if (data == 0) {
                    // bank0は0x0000 - 0x3fffにマッピングされる
                    // 0x00が書き込まれると0x01が書き込まれたときと同じ動作になる(bank1の選択)
                    // これはバンク番号上位2bitを使用する場合(0x20, 0x40, 0x60)を使用する場合も同様(それぞれ0x21, 0x41, 0x61を指定することになる)
                    // したがってバンク番号0x20, 0x40, 0x60は使用不可である
                    data = 1;
                }
                mbc_state.rom_bank_n = (mbc_state.rom_bank_n & 0x60) + (data & 0x1f);
            }
            else if (0x4000 <= addr && addr <= 0x5fff) {
                // セカンダリバンクレジスタ
                // バンクモードセレクトレジスタ(0x6000-0x7fff)の値に応じて、以下2つのうちいずれかを指定する
                // ・RAMバンクの選択
                // ・ROMバンクの上位2bitを切り替える
                if (mbc_state.bank_mode_sel == 0) { //rom bank
                    mbc_state.rom_bank_n = (data & 0x60) + (mbc_state.rom_bank_n & 0x1f);
                }
                else if (mbc_state.bank_mode_sel == 1) { //ram bank
                    //TODO: 仕様調べて実装する
                }

            }
            else if (0x6000 <= addr && addr <= 0x7fff) {
                // バンクモードセレクト
                if (data == 0 || data == 1) {
                    //0: rom bank mode
                    //1: ram bank mode
                    mbc_state.bank_mode_sel = data;
                }
            }
        }
    } else if (0x8000 <= addr && addr <= 0x9FFF) {
        // 8KB vram
        ram[addr] = data;
    } else if (0xa000 <= addr && addr <= 0xbfff) {
        // 8KB cartridge ram
        if (mbc_state.type == MBC::MBC1) {
            uint32_t offset = mbc_state.ram_bank_n + RAM_BANK_SIZE;
            *(ram_bank + offset + addr - 0xa000) = data;
        }
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