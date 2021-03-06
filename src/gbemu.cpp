
#include "gbemu.hpp"

// int gbemu_argc;
// char **gbemu_argv;

GBPalette GBEmu::palette;

GBEmu::GBEmu()
{
    _init();
}
GBEmu::~GBEmu()
{
    //終了処理入れると何故か安定しなくなるんじゃ....
    destory_imgui();
    SDL_Quit();

    delete[] ram;
    delete[] rom;
    delete[] cartridge_ram;
}

void GBEmu::_init()
{
    ram = new uint8_t[MEM_SIZE];
    rom = new uint8_t[ROM_SIZE];
    cartridge_ram = new uint8_t[CARTRIDGE_RAM_BANK_SIZE * CARTRIDGE_RAM_BANK_COUNT];

    // PPUの各モード間のクロック数計算
    double _clk_rate = CLOCK_RATE;
    double clk_sec;

    clk_sec = (double)1.0f / _clk_rate;

    // 参考: http://imrannazar.com/GameBoy-Emulation-in-JavaScript:-GPU-Timings
    PPU_MODE_CLOCKS[PPU_MODE_0] = 204;
    PPU_MODE_CLOCKS[PPU_MODE_1] = 456;
    PPU_MODE_CLOCKS[PPU_MODE_2] = 80;
    PPU_MODE_CLOCKS[PPU_MODE_3] = 172;

    PPU_MODE_LINE_CLOCK = PPU_MODE_CLOCKS[PPU_MODE_2] + PPU_MODE_CLOCKS[PPU_MODE_3] + PPU_MODE_CLOCKS[PPU_MODE_0];

    /*各モードを処理に要する秒数を、1クロックあたりの秒数で割って、各モードの処理に必要なクロックを求める
    ここで求めたクロック数を基準にPPUではモードを切り替えていく
    誤差で死んだので、一旦参考サイトから値拾ってくる。後で直す
    PPU_MODE_CLOCKS[PPU_MODE_0] = (uint16_t)(48.6e-6 / clk_sec);
    PPU_MODE_CLOCKS[PPU_MODE_1] = (uint16_t)(1.08e-3 / clk_sec);
    PPU_MODE_CLOCKS[PPU_MODE_2] = (uint16_t)(19.0e-6 / clk_sec);
    PPU_MODE_CLOCKS[PPU_MODE_3] = (uint16_t)(41.0e-6 / clk_sec);
    PPU_MODE_LINE_CLOCK = PPU_MODE_CLOCKS[PPU_MODE_2] +
                          PPU_MODE_CLOCKS[PPU_MODE_3] +
                          PPU_MODE_CLOCKS[PPU_MODE_0];

    printf("[PPU MODE CLOCKS]\n");
    printf("sec/clk: %.30lf sec/clk\n", clk_sec);
    printf("MODE0: %d clk\n", PPU_MODE_CLOCKS[PPU_MODE_0]);
    printf("MODE1: %d clk\n", PPU_MODE_CLOCKS[PPU_MODE_1]);
    printf("MODE2: %d clk\n", PPU_MODE_CLOCKS[PPU_MODE_2]);
    printf("MODE3: %d clk\n", PPU_MODE_CLOCKS[PPU_MODE_3]);
    */
    
    //lcd_status = (IO_LCD_STAT*)&ram[STAT];

    for (int i = 0; i < BREAK_POINT_MAX; i++) debug_break_addr[i] = 0;
    debug_break_addr[0] = 0x0100;
    debug_break_addr[1] = 0x0749;
    //debug_break_addr[1] = 0x0747;
    //debug_break_addr[2] = 0x073e; //問題の箇所ここから
    debug_break_addr[2] = 0x0753;
    debug_break_addr[3] = 0x0788;

    debug_break_addr[3] = 0x075b; //タイルデータのコピー前
    debug_break = true;

    enable_debug = true;
    _sdlinit();
}

void GBEmu::_sdlinit()
{
    //SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER);
    SDL_Init(SDL_INIT_EVERYTHING);
    if (enable_debug) {
        init_imgui();
        init_win_debug_gui();
        init_win_ppu_tile();
    }
    init_win_lcd();
}

void GBEmu::dump_regs()
{
    printf("[registers:start]\n");
    printf("af: 0x%04x\n", reg.af);
    printf("bc: 0x%04x\n", reg.bc);
    printf("de: 0x%04x\n", reg.de);
    printf("hl: 0x%04x\n", reg.hl);
    printf("sp: 0x%04x\n", reg.sp);
    printf("pc: 0x%04x\n", reg.pc);
    printf("a: 0x%02x\n", reg.a);
    printf("f: 0x%02x\n", reg.f);
    printf("b: 0x%02x\n", reg.b);
    printf("c: 0x%02x\n", reg.c);
    printf("b: 0x%02x\n", reg.d);
    printf("c: 0x%02x\n", reg.e);
    printf("h: 0x%02x\n", reg.h);
    printf("l: 0x%02x\n", reg.l);
    printf("carry: %d\n", reg.flags.c);
    printf("half: %d\n", reg.flags.h);
    printf("add_sub: %d\n", reg.flags.n);
    printf("zero: %d\n", reg.flags.z);
    printf("[registers:end]\n");
}

void GBEmu::dump_rom(uint32_t from, uint32_t bytes)
{
    printf("[ROM dump(units: 1byte)] [%d - %d]", from, from + bytes);
    for (uint32_t i = from; i <= from + bytes; i++) {
        printf("[%d] 0x%x\n", i, rom[i]);
    }
}

void GBEmu::init_regs(void) {
    // initialize ragisters
    reg.af = 0x01b0;
    reg.bc = 0x0013;
    reg.de = 0x00d8;
    reg.hl = 0x014d;
    reg.sp = 0xfffe;
    reg.pc = 0x0100;
}

void GBEmu::init_io_ports(void) {
    mbc_state.ram_enable = false;

    //WHY: BGBだと1になってる
    ((IO_LCD_STAT*)&ram[IO_REG::STAT])->unused = 1;

    //WHY: BGBだと初期化時に1になってる
    ((IO_IF_FLAG*)&ram[IO_REG::IF])->vblank = 1;

    //ppuカウンタ初期化
    ppu_mode_clock = 0;
    //BGB実装だと開始時点でmode1になってる様子
    ((IO_LCD_STAT*)&ram[IO_REG::STAT])->mode = 1;
    ram[IO_REG::LY] = 153;
    //BGB実装だと0x100の時点でこの値っぽい
    ppu_mode_clock = PPU_MODE_LINE_CLOCK - 56;



    ram[0xFF05] = 0x00;  // TIMA
    ram[0xFF06] = 0x00;  // TMA
    ram[0xFF07] = 0x00;  // TAC
    ram[0xFF10] = 0x80;  // NR10
    ram[0xFF11] = 0xBF;  // NR11
    ram[0xFF12] = 0xF3;  // NR12
    ram[0xFF14] = 0xBF;  // NR14
    ram[0xFF16] = 0x3F;  // NR21
    ram[0xFF17] = 0x00;  // NR22
    ram[0xFF19] = 0xBF;  // NR24
    ram[0xFF1A] = 0x7F;  // NR30
    ram[0xFF1B] = 0xFF;  // NR31
    ram[0xFF1C] = 0x9F;  // NR32
    ram[0xFF1E] = 0xBF;  // NR33
    ram[0xFF20] = 0xFF;  // NR41
    ram[0xFF21] = 0x00;  // NR42
    ram[0xFF22] = 0x00;  // NR43
    ram[0xFF23] = 0xBF;  // NR30
    ram[0xFF24] = 0x77;  // NR50
    ram[0xFF25] = 0xF3;  // NR51
    ram[0xFF26] = 0xF1;  // NR52
    ram[IO_REG::LCDC] = 0x91;
    ram[IO_REG::SCY] = 0x00; 
    ram[IO_REG::SCX] = 0x00; 
    ram[IO_REG::LYC] = 0x00;
    ram[IO_REG::BGP] = 0xFC;
    ram[IO_REG::OBP0] = 0xFF;
    ram[IO_REG::OBP1] = 0xFF;
    ram[IO_REG::WY] = 0x00;
    ram[IO_REG::WX] = 0x00; 
    ram[0xFFFF] = 0x00;  // IE
}

void GBEmu::run(const char rom_file_path[])
{
    mbc_state.type = MBC::MBC1;
    mbc_state.ram_enable = false;
    mbc_state.rom_bank_n = 0;
    mbc_state.bank_mode_sel = 0;

    memset(ram, 0, MEM_SIZE);
    memset(rom, 0, ROM_SIZE);

    init_regs();
    init_io_ports();

    stop = false;
    win_close = false;

    bool load_rom = false;

    if (rom_file_path == nullptr) {
        printf("romfile: [NULL]\n");
    }
    else {
        printf("romfile: %s", rom_file_path);
        int fd;
        
        if (_sopen_s(&fd, rom_file_path, _O_RDONLY, _SH_DENYNO, 0)) {
            printf("[NG](failed to open)\n");
        }
        else{
            if (_read(fd, rom, ROM_SIZE) > 0) {
                printf("[OK]\n");
                load_rom = true;
            }
            else {
                printf("[NG](failed to read)\n");
            }
            _close(fd);
        }

    }

    /*
    std::thread cpu(&GBEmu::cpu_loop, this);
    cpu.join();
    */
    SDL_Thread* cpu;
    cpu = SDL_CreateThread(&GBEmu::cpu_loop_wrapper, "cpu_loop", this);

    sdl_loop();
    dump_regs();

    int _ret;
    SDL_WaitThread(cpu, &_ret);
    printf("CPU Thread finished.(%d)", _ret);

    // SDL_Delay(50);
    if (enable_debug) {
        destroy_win_debug_gui();

        SDL_DestroyRenderer(rend_ppu_tile);
        SDL_DestroyWindow(win_ppu_tile);
    }

    SDL_DestroyRenderer(rend_lcd);
    SDL_DestroyWindow(win_lcd);
}

int GBEmu::cpu_loop_wrapper(void* data)
{
    GBEmu* self = static_cast<GBEmu*>(data);
    return self->cpu_loop();
}

int  GBEmu::cpu_loop(void) {
    // emulator steps
    while (!exit_emu) {
        auto start = std::chrono::system_clock::now();
        
        cpu_step();
        ppu_step();

        auto end = std::chrono::system_clock::now();

        //実際の処理にかかった時間
        auto process_time = end - start;
        auto process_time_micro = std::chrono::duration_cast<std::chrono::microseconds>(process_time);
        if (process_time_micro.count() != 0) {
            fps_max = (uint16_t)((long long)1e6 / process_time_micro.count());
        }
    }
    printf("[end] 0x%04x: 0x%02x\n", reg.pc, read_mem(reg.pc));
    printf("%d\n", stop);
    return 0;
}

void GBEmu::sdl_loop(void)
{
    while (!win_close) {
        // SDL steps
        sdl_event();
        display_win_lcd();
        if (enable_debug) {
            display_win_ppu_tile();
            display_win_debug_gui();
        }
    }
}

void GBEmu::sdl_event(void)
{
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (enable_debug) {
            ImGui_ImplSDL2_ProcessEvent(&e);
        }
        if (e.type == SDL_QUIT) {
            exit_emu = true;
            win_close = true;
            break;
        }
        if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE) {
            // if (e.window.windowID == SDL_GetWindowID(win_ppu_tile) || e.window.windowID == SDL_GetWindowID(win_debug_gui))
            exit_emu = true;
            win_close = true;
            break;
        }
        if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_ESCAPE) {
                exit_emu = 1;
                win_close = true;
            }
            if (e.key.keysym.sym == SDLK_F1) {
                stop = !stop;
            }
            if (e.key.keysym.sym == SDLK_F7) {
                mtx_stop.lock();
                stop = false;
                debug_step_exec = true;
                mtx_stop.unlock();
            }
            if (e.key.keysym.sym == SDLK_F9) {
                mtx_stop.lock();
                stop = false;
                debug_step_exec = false;
                mtx_stop.unlock();
            }
            if (e.key.keysym.sym == SDLK_F10) {
                mtx_stop.lock();
                debug_break = !debug_break;
                mtx_stop.unlock();
            }
        }
    }
}
