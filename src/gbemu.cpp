#include "gbemu.hpp"

int gbemu_argc;
char **gbemu_argv;

GBPalette GBEmu::palette;

GBEmu::GBEmu() {
    _init();

    /*
    GtkWidget *w;
    gtk_init(&gbemu_argc, &gbemu_argv);
    w = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(w), "GBEmu");
    gtk_widget_show_all(w);
    gtk_main();
    */
}
GBEmu::~GBEmu() {
    delete[] ram;
    delete[] rom;

    //SDL_DestroyRenderer(renderer);
    //SDL_DestroyWindow(window);
    //SDL_Delay(500);
    //SDL_Quit();
}

void GBEmu::_init() {
    ram = new uint8_t[MEM_SIZE]();
    rom = new uint8_t[ROM_SIZE]();

    //PPUの各モード間のクロック数計算
    double clk_rate = CLOCK_RATE;
    double clk_sec;
    //1クロックあたりの秒数
    clk_sec = (double)1.0f / clk_rate;
    //各モードを処理に要する秒数を、1クロックあたりの秒数で割って、各モードの処理に必要なクロックを求める
    //ここで求めたクロック数を基準にPPUではモードを切り替えていく
    PPU_MODE_CLOCKS[PPU_MODE_0] = (uint16_t)(48.6e-6 / clk_sec);
    PPU_MODE_CLOCKS[PPU_MODE_1] = (uint16_t)(1.08e-3 / clk_sec);
    PPU_MODE_CLOCKS[PPU_MODE_2] = (uint16_t)(19.0e-6 / clk_sec);
    PPU_MODE_CLOCKS[PPU_MODE_3] = (uint16_t)(41.0e-6 / clk_sec);
    PPU_MODE_LINE_CLOCK = PPU_MODE_CLOCKS[PPU_MODE_2] + PPU_MODE_CLOCKS[PPU_MODE_3] + PPU_MODE_CLOCKS[PPU_MODE_0];

    printf("[PPU MODE CLOCKS]\n");
    printf("sec/clk: %.30lf sec/clk\n", clk_sec);
    printf("MODE0: %d clk\n", PPU_MODE_CLOCKS[PPU_MODE_0]);
    printf("MODE1: %d clk\n", PPU_MODE_CLOCKS[PPU_MODE_1]);
    printf("MODE2: %d clk\n", PPU_MODE_CLOCKS[PPU_MODE_2]);
    printf("MODE3: %d clk\n", PPU_MODE_CLOCKS[PPU_MODE_3]);
    //_sdlinit();
}

void GBEmu::_sdlinit(){
    scale = 2;
    
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow("GBEmu", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 160*scale, 144*scale, 0);
    renderer = SDL_CreateRenderer(window, -1, 0);
}

void GBEmu::display() {
    SDL_RenderSetScale(renderer, scale, scale);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for(int i = 0; i < 160/2; i++){
        SDL_RenderDrawPoint(renderer, i, 0);
        SDL_RenderDrawPoint(renderer, i, 1);
    }
    SDL_RenderPresent(renderer);
}

void GBEmu::dump_regs(){
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

void GBEmu::dump_rom(uint32_t from, uint32_t bytes) {
    printf("[ROM dump(units: 1byte)] [%d - %d]", from, from + bytes);
    for (uint32_t i = from; i <= from + bytes; i++) {
        printf("[%d] 0x%x\n", i, rom[i]);
    }
}
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
void GBEmu::push(uint16_t data){
    reg.sp -= 2;
    write_mem_u16(reg.sp, data);
    //printf("!! 0x%04x:0x%04x\n", reg.sp, read_mem_u16(reg.sp));
}
uint16_t GBEmu::pop(){
    uint16_t data = read_mem_u16(reg.sp);
    reg.sp += 2;
    return data;
}

void GBEmu::run(const char rom_file_path[], bool flg_dump_regs, uint16_t exit_pc) {
    printf("romfile: %s\n", rom_file_path);
    reg.pc = 0x100;

    int fd;
    fd = open(rom_file_path, O_RDONLY);
    if (fd == -1) {
        printf("failed to ROM file.(1)\n");
        exit(-1);
    }

    if (read(fd, rom, ROM_SIZE) == -1) {
        printf("failed to ROM file.(2)\n");
        exit(-1);
    }
    close(fd);

    mbc = MBC::MBC0;

    printf("\n");

    end = false;

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

    ppu_mode = 2;
    ppu_line = 0;

    while(!end){
        cpu_step();
        ppu_step();
    }

    if(flg_dump_regs){
        dump_regs();
        printf("LY: %d\n", read_mem(IO_REG::LY));
    }

    /*
    while(true){
        display();

        SDL_Event e;
        if(SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT){
                break;
            }
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE){
                break;
            }
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_1){
                scale = 1;
                SDL_SetWindowSize(window, 160*scale, 144*scale);
            }
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_2){
                scale = 2;
                SDL_SetWindowSize(window, 160*scale, 144*scale);
            }
        }
    }*/
    //display();
}