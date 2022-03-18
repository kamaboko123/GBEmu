#include "gbemu.hpp"

void GBEmu::init_imgui(void)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
}

void GBEmu::destory_imgui(void)
{
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void GBEmu::init_win_debug_gui(void)
{
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    win_debug_gui = SDL_CreateWindow(
        "[GBEmu(DEBUG)]",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        1280,
        960,
        window_flags);

    rend_debug_gui = SDL_CreateRenderer(
        win_debug_gui,
        -1,
        SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

    ImGui_ImplSDL2_InitForSDLRenderer(win_debug_gui, rend_debug_gui);
    ImGui_ImplSDLRenderer_Init(rend_debug_gui);

    float scale = 1.0f;
    SDL_RenderSetScale(rend_debug_gui, scale, scale);
}

void GBEmu::destroy_win_debug_gui(void)
{
    SDL_DestroyRenderer(rend_debug_gui);
    SDL_DestroyWindow(win_debug_gui);
}

void GBEmu::display_win_debug_gui(void)
{
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    {
        ImGui::Begin("Registers");
        bool z = reg.flags.z;
        bool n = reg.flags.n;
        bool h = reg.flags.h;
        bool c = reg.flags.c;

        ImGui::Text("af: $%04x\n", reg.af);
        ImGui::Text("bc: $%04x\n", reg.bc);
        ImGui::Text("de: $%04x\n", reg.de);
        ImGui::Text("hl: $%04x\n", reg.hl);
        ImGui::Text("sp: $%04x\n", reg.sp);
        ImGui::Text("pc: $%04x\n", reg.pc);
        ImGui::Separator();

        ImGui::Text("[Flags]");
        ImGui::Checkbox("z", &z);
        ImGui::Checkbox("n", &n);
        ImGui::Checkbox("h", &h);
        ImGui::Checkbox("c", &c);
        ImGui::End();
    }

    {
        bool lcdc[8] = {0};
        lcdc[0] = ((IO_LCD_LCDC *)ram + IO_REG::LCDC)->bg_enable;
        lcdc[1] = ((IO_LCD_LCDC *)ram + IO_REG::LCDC)->obj_enable;
        lcdc[2] = ((IO_LCD_LCDC *)ram + IO_REG::LCDC)->obj_size;
        lcdc[3] = ((IO_LCD_LCDC *)ram + IO_REG::LCDC)->bg_tile_map_show;
        lcdc[4] = ((IO_LCD_LCDC *)ram + IO_REG::LCDC)->bg_window_tile_data;
        lcdc[5] = ((IO_LCD_LCDC *)ram + IO_REG::LCDC)->window_enable;
        lcdc[6] = ((IO_LCD_LCDC *)ram + IO_REG::LCDC)->bg_tile_map_show;
        lcdc[7] = ((IO_LCD_LCDC *)ram + IO_REG::LCDC)->lcd_enable;

        ImGui::Begin("LCD");
        ImGui::Text("LCDC[FF40]: %d($%02x)\n", ram[IO_REG::LCDC], ram[IO_REG::LCDC]);
        ImGui::Text("STAT[FF41]: %d($%02x)\n", ram[IO_REG::STAT], ram[IO_REG::STAT]);
        ImGui::Text("SXY[FF42] : %d($%02x)\n", ram[IO_REG::SCY], ram[IO_REG::SCY]);
        ImGui::Text("SCY[FF43] : %d($%02x)\n", ram[IO_REG::SCX], ram[IO_REG::SCX]);
        ImGui::Text("LY[FF44]  : %d($%02x)\n", ram[IO_REG::LY], ram[IO_REG::LY]);
        ImGui::Text("LYC[FF45] : %d($%02x)\n", ram[IO_REG::LYC], ram[IO_REG::LYC]);
        ImGui::Separator();
        ImGui::Checkbox("lcd on", &lcdc[7]);
        ImGui::Text("win tile map: %s\n", lcdc[6] == 0 ? "9800 - 9BFF" : "9C00 - 9FFF");
        ImGui::Checkbox("win", &lcdc[5]);
        ImGui::Text("bg tile map show: %s\n", lcdc[4] == 0 ? "8800 - 97FF" : "8000 - 8FFF");
        ImGui::Text("bg window tile data: %s\n", lcdc[3] == 0 ? "8800 - 9BFF" : "9C00 - 9FFF");
        ImGui::Text("obj", lcdc[3] == 0 ? "8 x 8" : "8 x 16");
        ImGui::Checkbox("bg(on)", &lcdc[7]);
        ImGui::Separator();
        ImGui::Text("mode: %d\n", ((IO_LCD_STAT*)&ram[IO_REG::STAT])->mode);
        ImGui::Text("ppu cnt: %d\n", ppu_mode_clock);
        ImGui::Text("Instruction per sec: %d\n", fps_max);
        ImGui::End();
    }
    {
        ImGui::Begin("BreakPoint");
        for (int i = 0; i < BREAK_POINT_MAX; i++) {
            ImGui::Text("[%02d] 0x%04x\n", i, debug_break_addr[i]);
        }
        ImGui::End();
    }
    {
        ImGui::Begin("Stack");
        for (uint16_t sp = reg.sp + DEBUG_SHOW_STACK_COUNT * 2; sp >= reg.sp - DEBUG_SHOW_STACK_COUNT * 2; sp -= 2) {
            if (sp <= (uint16_t)ram || sp >= (uint16_t)ram + MEM_SIZE) break;
            char p = (sp == reg.sp) ? '>' : ' ';
            ImGui::Text("%c [0x%04x] 0x%04x\n", p, sp, read_mem_u16(sp));
        }
        ImGui::End();
    }
    {
        ImGui::Begin("Exec Instruction");
        ImGui::Text("pc    : 0x%04x", reg.pc);
        ImGui::Text("opcode: 0x%02x", read_mem(reg.pc));
        ImGui::Separator();
        ImGui::Text("[Execution flags]");
        ImGui::Checkbox("stop", &stop);
        ImGui::Checkbox("step", &debug_step_exec);
        ImGui::Checkbox("break", &debug_break);
        ImGui::Separator();
        ImGui::Text("[Run command] Run: F9  Trace: F7  Stop: F1  Break Enable/Disable: F10");
        ImGui::End();
    }
    {
        ImGui::Begin("Interrupt Flag");
        IO_IF_FLAG* iflag = (IO_IF_FLAG*) & ram[IO_REG::IF];
        bool _vblank = iflag->vblank;
        bool _lcd_stat =iflag->lcd_stat;
        bool _timer = iflag->timmer;
        bool _serial = iflag->serial;
        bool _joypad = iflag->joypad;

        ImGui::Text("[Flags]");
        ImGui::Checkbox("vblank", &_vblank);
        ImGui::Checkbox("lcd   ", &_lcd_stat);
        ImGui::Checkbox("timer ", &_timer);
        ImGui::Checkbox("serial", &_serial);
        ImGui::Checkbox("joypad", &_joypad);
        ImGui::End();
    }

    ImGui::Render();
    SDL_SetRenderDrawColor(rend_debug_gui, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
    SDL_RenderClear(rend_debug_gui);
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(rend_debug_gui);
}


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