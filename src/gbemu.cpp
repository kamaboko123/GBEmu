#include "gbemu.hpp"

int gbemu_argc;
char **gbemu_argv;

GBPalette GBEmu::palette;

GBEmu::GBEmu()
{
    _init();
}
GBEmu::~GBEmu()
{
    delete[] ram;
    delete[] rom;
}

void GBEmu::_init()
{
    ram = new uint8_t[MEM_SIZE]();
    rom = new uint8_t[ROM_SIZE]();

    // PPUの各モード間のクロック数計算
    double clk_rate = CLOCK_RATE;
    double clk_sec;
    // 1クロックあたりの秒数
    clk_sec = (double)1.0f / clk_rate;
    //各モードを処理に要する秒数を、1クロックあたりの秒数で割って、各モードの処理に必要なクロックを求める
    //ここで求めたクロック数を基準にPPUではモードを切り替えていく
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

    win_close = false;

    _sdlinit();
}

void GBEmu::_sdlinit()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER);
    init_win_ppu_tile();
    init_win_debug_gui();
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

void GBEmu::run(const char rom_file_path[], bool flg_dump_regs, uint16_t exit_pc)
{
    stop = false;
    reg.pc = 0x100;
    ppu_mode = 2;
    ppu_line = 0;

    bool load_rom = false;
    memset(ram, 0, sizeof(MEM_SIZE));
    memset(rom, 0, sizeof(ROM_SIZE));

    if (rom_file_path == nullptr) {
        printf("romfile: [NULL]\n");
    }
    else {
        printf("romfile: %s", rom_file_path);
        int fd;
        fd = open(rom_file_path, O_RDONLY);
        if (fd != -1) {
            if (read(fd, rom, ROM_SIZE) != -1) {
                printf("[OK]\n");
                load_rom = true;
            }
            else {
                printf("[NG](failed to read)\n");
            }
            close(fd);
        }
        else {
            printf("[NG](failed to open)\n");
        }
    }

    mbc = MBC::MBC0;

    ram[0x8000] = 0x18;
    ram[0x8001] = 0x18;
    ram[0x8002] = 0x3C;
    ram[0x8003] = 0x3C;
    ram[0x8004] = 0x66;
    ram[0x8005] = 0x66;
    ram[0x8006] = 0x66;
    ram[0x8007] = 0x66;
    ram[0x8008] = 0x7E;
    ram[0x8009] = 0x7E;
    ram[0x800a] = 0x66;
    ram[0x800b] = 0x66;
    ram[0x800c] = 0x66;
    ram[0x800d] = 0x66;
    ram[0x800e] = 0x00;
    ram[0x800f] = 0x00;

    ram[0x8010] = 0x7C;
    ram[0x8011] = 0x7C;
    ram[0x8012] = 0x66;
    ram[0x8013] = 0x66;
    ram[0x8014] = 0x66;
    ram[0x8015] = 0x66;
    ram[0x8016] = 0x7C;
    ram[0x8017] = 0x7C;
    ram[0x8018] = 0x66;
    ram[0x8019] = 0x66;
    ram[0x801a] = 0x66;
    ram[0x801b] = 0x66;
    ram[0x801c] = 0x7C;
    ram[0x801d] = 0x7C;
    ram[0x801e] = 0x00;
    ram[0x801f] = 0x00;

    ram[0x8020] = 0x3C;
    ram[0x8021] = 0x3C;
    ram[0x8022] = 0x66;
    ram[0x8023] = 0x66;
    ram[0x8024] = 0x60;
    ram[0x8025] = 0x60;
    ram[0x8026] = 0x60;
    ram[0x8027] = 0x60;
    ram[0x8028] = 0x60;
    ram[0x8029] = 0x60;
    ram[0x802a] = 0x66;
    ram[0x802b] = 0x66;
    ram[0x802c] = 0x3C;
    ram[0x802d] = 0x3C;
    ram[0x802e] = 0x00;
    ram[0x802f] = 0x00;

    ram[0x8100] = 0x7c;
    ram[0x8101] = 0x7c;
    ram[0x8102] = 0x00;
    ram[0x8103] = 0xc6;
    ram[0x8104] = 0xc6;
    ram[0x8105] = 0x00;
    ram[0x8106] = 0x00;
    ram[0x8107] = 0xfe;
    ram[0x8108] = 0xc6;
    ram[0x8109] = 0xc6;
    ram[0x810a] = 0x00;
    ram[0x810b] = 0xc6;
    ram[0x810c] = 0xc6;
    ram[0x810d] = 0x00;
    ram[0x810e] = 0x00;
    ram[0x810f] = 0x00;

    // initialize ragisters
    reg.af = 0x01b0;
    reg.bc = 0x0013;
    reg.de = 0x00d8;
    reg.hl = 0x014d;
    reg.sp = 0xfffe;

    // io registers
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
    ram[0xFF40] = 0x91;  // LCDC
    ram[0xFF42] = 0x00;  // SCY
    ram[0xFF43] = 0x00;  // SCX
    ram[0xFF45] = 0x00;  // LYC
    ram[0xFF47] = 0xFC;  // BGP
    ram[0xFF48] = 0xFF;  // OBP0
    ram[0xFF49] = 0xFF;  // OBP1
    ram[0xFF4A] = 0x00;  // WY
    ram[0xFF4B] = 0x00;  // WX
    ram[0xFFFF] = 0x00;  // IE

    main_loop();

    if (flg_dump_regs) {
        dump_regs();
    }

    // SDL_Delay(50);
    destroy_win_debug_gui();
    SDL_DestroyRenderer(rend_ppu_tile);
    SDL_DestroyWindow(win_ppu_tile);
    SDL_Quit();
}

void GBEmu::main_loop(void)
{
    while (!win_close) {
        // SDL steps
        sdl_event();
        display_win_ppu_tile();
        display_win_debug_gui();

        // emulator steps
        if (!stop) {
            cpu_step();
            ppu_step();
        }
    }
}

void GBEmu::sdl_event(void)
{
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        ImGui_ImplSDL2_ProcessEvent(&e);
        if (e.type == SDL_QUIT) {
            stop = true;
            win_close = true;
            break;
        }
        if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE) {
            if (e.window.windowID == SDL_GetWindowID(win_ppu_tile) || e.window.windowID == SDL_GetWindowID(win_debug_gui)) {
                stop = true;
                win_close = true;
                break;
            }
        }
        if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_ESCAPE) {
                stop = true;
                win_close = true;
                break;
            }
            if (e.key.keysym.sym == SDLK_F1) {
                stop = !stop;
            }
        }
    }
}
