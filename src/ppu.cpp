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

    lcd_status->lyc_ly_c = (ram[IO_REG::LY] == ram[IO_REG::LYC]) ? 1 : 0;
}



void GBEmu::init_win_lcd(void)
{
    lcd_scale = 4;

    win_lcd = SDL_CreateWindow(
        "[GBEmu] LCD",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        160 * lcd_scale,
        144 * lcd_scale,
        0);
    rend_lcd = SDL_CreateRenderer(win_lcd, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_RenderSetScale(rend_lcd, lcd_scale, lcd_scale);
    SDL_RenderPresent(rend_lcd);
}

void GBEmu::display_win_lcd(void)
{
    SDL_RenderSetScale(rend_lcd, lcd_scale, lcd_scale);
    SDL_SetRenderDrawColor(rend_lcd, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(rend_lcd);

    uint8_t scr_x = ram[IO_REG::SCX];
    uint8_t scr_y = ram[IO_REG::SCY];
    uint8_t *map = ram + 0x9800;
    /*
    map[0] = 0x40;
    map[1] = 0x40;
    map[2] = 0x40;
    map[3] = 0x40;
    map[4] = 0x40;
    */
    uint8_t map_index_y = (scr_y / 8);
    for (uint8_t lcd_y = 0; lcd_y < 19; lcd_y++) {
        uint8_t map_index_x = (scr_x / 8);

        for (uint8_t lcd_x = 0; lcd_x < 21; lcd_x++) {
            if ((scr_x % 8 != 0 && scr_y % 8 != 0) && (lcd_x == 0 && lcd_y == 0)) {
                draw_tile(rend_lcd, map[(map_index_y * 32) + map_index_x], 0, 0, scr_x % 8, scr_y % 8);
            }
            else {
                if (lcd_x == 0) {
                    draw_tile(rend_lcd, map[(map_index_y * 32) + map_index_x], 0, (lcd_y * 8) - (scr_y % 8), scr_x % 8, 0);
                }
                if (lcd_y == 0) {
                    draw_tile(rend_lcd, map[(map_index_y * 32) + map_index_x], (lcd_x * 8) - (scr_x % 8), 0, 0, scr_y % 8);
                }
            }

            if (lcd_x > 0 && lcd_y > 0) {
                draw_tile(rend_lcd, map[(map_index_y * 32) + map_index_x], (lcd_x * 8) - (scr_x % 8), (lcd_y * 8) - (scr_y % 8), 0, 0);
            }

            map_index_x++;
            if (map_index_x >= 32) {
                map_index_x = 0;
            }
        }
        map_index_y++;
        if (map_index_y >= 32) {
            map_index_y = 0;
        }
    }

    SDL_RenderPresent(rend_lcd);
}

void GBEmu::draw_tile(SDL_Renderer* r, uint8_t t, uint16_t x, uint16_t y, int16_t offset_x, int16_t offset_y) {
    for (int _y = offset_y; _y < 8; _y++) {
        uint8_t* addr = ram + VRAM_TILE_HEAD + ((uint64_t)16 * t) + (uint64_t)_y * 2;
        uint8_t l = *addr;      //下位が格納されているバイト
        uint8_t h = *(addr + 1);  //上位が格納されているバイト

        for (int _x = offset_x; _x < 8; _x++) {
            uint8_t col_no = (clib::getBit(h, 7 - _x) << 1) + clib::getBit(l, 7 - _x);

            SDL_SetRenderDrawColor(r, palette[col_no].r, palette[col_no].g, palette[col_no].b, 0x00);
            if (x + _x - offset_x < 0 || x + _x - offset_x > 160 || y + _y - offset_y < 0 || y + _y - offset_y > 144) continue;
            SDL_RenderDrawPoint(r, x + _x - offset_x, y + _y - offset_y);
        }
    }
}