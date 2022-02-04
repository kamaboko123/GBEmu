#include "gbemu.hpp"

void GBEmu::ppu_step(void){
    /* 参考
        http://imrannazar.com/GameBoy-Emulation-in-JavaScript:-GPU-Timings
        https://w.atwiki.jp/gbspec/pages/21.html
        http://bgb.bircd.org/pandocs.htm
    */
    printf("PPU_MODE: %d\nLY: %d\n", ppu_mode, read_mem(IO_REG::LY));
    ppu_mode_clock += last_instr_clock;

    switch(ppu_mode){
        case PPU_MODE_2: // Read OAM
            if(ppu_mode_clock >= PPU_MODE_CLOCKS[PPU_MODE_2]){
                ppu_mode_clock = 0;
                ppu_mode = 3;
            }
        break;
        case PPU_MODE_3: // Read VRAM and OAM
            if(ppu_mode_clock >= PPU_MODE_CLOCKS[PPU_MODE_3]){
                ppu_mode_clock = 0;
                ppu_mode  = 0;

                // TODO: render image
            }
        break;
        case PPU_MODE_0: // h-blank
            if(ppu_mode_clock >= PPU_MODE_CLOCKS[PPU_MODE_0]){
                ppu_mode_clock = 0;
                ppu_line++;
                
                //143行描画したらv-blankへ
                if(ppu_line == 143){
                    ppu_mode = PPU_MODE_1;
                }
            }
        break;
        case PPU_MODE_1: // v-blank
            //TODO: v-blank 割り込み実装
            if(ppu_mode_clock >= PPU_MODE_LINE_CLOCK){
                ppu_mode_clock = 0;
                ppu_line++;

                if(ppu_line == 154){
                    ppu_mode = 2;
                    ppu_line = 0;
                }
            }
        break;
    }

    write_mem(IO_REG::LY, ppu_line);
}