#include "gbemu.hpp"


void GBEmu::ppu_step(void)
{
    /* 参考
        http://imrannazar.com/GameBoy-Emulation-in-JavaScript:-GPU-Timings
        https://w.atwiki.jp/gbspec/pages/21.html
        http://bgb.bircd.org/pandocs.htm
    */
    // printf("PPU_MODE: %d\nLY: %d\nppu_mode_clock:%d\n", ppu_mode, read_mem(IO_REG::LY), ppu_mode_clock);
    ppu_mode_clock += last_instr_clock;

    //map drawed line to IO port
    uint8_t *ppu_line = &ram[IO_REG::LY];
    IO_LCD_STAT * lcd_status = (IO_LCD_STAT*)&ram[IO_REG::STAT];
    IO_LCD_LCDC * lcd_control = (IO_LCD_LCDC*)&ram[IO_REG::LCDC];

    if (lcd_control->lcd_enable == 0) {
        //LCD無効時はmodeは0にしておくべき？
        lcd_status->mode = 2;
        ppu_line = 0;
        ppu_mode_clock = 0;
        return;
    }

    switch (lcd_status->mode) {
        case PPU_MODE_2:
            // Read OAM
            if (ppu_mode_clock >= PPU_MODE_CLOCKS[PPU_MODE_2]) {
                ppu_mode_clock = 0;
                lcd_status->mode = PPU_MODE_3;
            }
            break;
        case PPU_MODE_3:
            // Read VRAM and OAM
            if (ppu_mode_clock >= PPU_MODE_CLOCKS[PPU_MODE_3]) {
                ppu_mode_clock = 0;
                lcd_status->mode = PPU_MODE_0;

                // TODO: render image
            }
            break;
        case PPU_MODE_0:
            // h-blank
            if (ppu_mode_clock >= PPU_MODE_CLOCKS[PPU_MODE_0]) {
                ppu_mode_clock = 0;
                (*ppu_line)++;

                // 143行描画したらv-blankへ
                if (*ppu_line == 143) {
                    lcd_status->mode = PPU_MODE_1;
                }
                else {
                    lcd_status->mode = PPU_MODE_2;
                }
            }
            break;
        case PPU_MODE_1:
            //printf("ppu_mode: %d  ppu_cnt:%d(%d)  ppu_line: %d\n", lcd_status->mode, ppu_mode_clock, PPU_MODE_LINE_CLOCK, *ppu_line);
            // v-blank
            // TODO: v-blank 割り込み実装
            if (((IO_IF_FLAG*)&ram[IO_REG::IF])->vblank == 0) {
                ((IO_IF_FLAG*)&ram[IO_REG::IF])->vblank = 1;
            }
            if (ppu_mode_clock >= PPU_MODE_LINE_CLOCK) {
                ppu_mode_clock = 0;
                (*ppu_line)++;
                printf("ppu_line: %d\n", *ppu_line);

                if (*ppu_line >= 153) {
                    lcd_status->mode = PPU_MODE_2;
                    *ppu_line = 0;
                }
            }
            break;
    }
}



void GBEmu::init_win_lcd(void)
{
    uint8_t scale = 4;

    win_lcd = SDL_CreateWindow(
        "[GBEmu] LCD",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        160 * scale,
        144 * scale,
        0);
    rend_lcd = SDL_CreateRenderer(win_lcd, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_RenderSetScale(rend_lcd, scale, scale);
    SDL_RenderPresent(rend_lcd);
}

void GBEmu::display_win_lcd(void)
{
    SDL_SetRenderDrawColor(rend_lcd, palette[0].r, palette[0].g, palette[0].b, 255);
    SDL_RenderClear(rend_lcd);
    SDL_RenderDrawPoint(rend_lcd, 1, 1);
    SDL_RenderPresent(rend_lcd);
}
