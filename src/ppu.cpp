#include "gbemu.hpp"

void GBEmu::init_win_ppu_tile(void)
{
    uint8_t tile_view_scale = 4;

    win_ppu_tile = SDL_CreateWindow(
        "[GBEmu(DEBUG)] Tile viewer",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        160 * tile_view_scale,
        144 * tile_view_scale,
        0);
    rend_ppu_tile = SDL_CreateRenderer(win_ppu_tile, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_RenderSetScale(rend_ppu_tile, tile_view_scale, tile_view_scale);
    SDL_RenderPresent(rend_ppu_tile);
}

void GBEmu::display_win_ppu_tile(void)
{
    SDL_SetRenderDrawColor(rend_ppu_tile, palette[0].r, palette[0].g, palette[0].b, 255);
    SDL_RenderClear(rend_ppu_tile);

    /*
        タイルのデコード処理
        タイルは 8 * 8 [px]、2bitカラー(4色)
        タイル1つは16byteで構成される
        2byteを1行として、8行分なので2 * 8 = 16 byte

        各行のデコード例
        ・2進数に変換
        byte0: 0xab -> 10101011
        byte1: 0xcc -> 11001100
        ・各ビットでbyte0側を下位、byte1側を上位として、2bitで色を示す
        11, 10, 01, 00, 11, 10, 01, 01 => 3, 2, 1, 0, 3, 2, 1, 1
        ・色はパレットで定義される。
        　通常は0(白)、1(薄いグレー)、2(濃いグレー)、3(黒)
        　指定された色で横向きに8ピクセル分塗り、次の行以降も同様に繰り返す
        　8行分(16byte分)で1タイルとなる
    */
    GBColor col;

    uint8_t l, h, l_bit, h_bit, col_n, render_x, render_y;
    for (uint16_t tile_no = TILE_NO_MIN; tile_no <= TILE_NO_MAX; tile_no++) {
        for (uint8_t y = 0; y < TILE_Y_PIX; y++) {
            uint16_t addr = VRAM_TILE_HEAD + (16 * tile_no) + (y * 2);
            l = read_mem(addr);      //下位が格納されているバイト
            h = read_mem(addr + 1);  //上位が格納されているバイト
            for (int x = 0; x < TILE_X_PIX; x++) {
                l_bit = clib::getBit(l, TILE_X_PIX - x - 1);
                h_bit = clib::getBit(h, TILE_X_PIX - x - 1);

                col_n = (h_bit << 1) + l_bit;
                col = palette[col_n];
                SDL_SetRenderDrawColor(rend_ppu_tile, col.r, col.g, col.b, 255);

                render_x = (tile_no % 16) * 8 + x;
                render_y = (tile_no / 16) * 8 + y;
                SDL_RenderDrawPoint(rend_ppu_tile, render_x, render_y);
            }
        }
    }

    SDL_RenderPresent(rend_ppu_tile);
}

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
    IO_LCD_STAT *lcd_status = (IO_LCD_STAT*)&ram[STAT];

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

void GBEmu::push(uint16_t data)
{
    reg.sp -= 2;
    write_mem_u16(reg.sp, data);
    // printf("!! 0x%04x:0x%04x\n", reg.sp, read_mem_u16(reg.sp));
}
uint16_t GBEmu::pop()
{
    uint16_t data = read_mem_u16(reg.sp);
    reg.sp += 2;
    return data;
}