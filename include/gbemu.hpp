#ifndef GBEMU_GBEMU_H
#define GBEMU_GBEMU_H
#include <SDL2/SDL.h>
#include <fcntl.h>
#include <unistd.h>

#include <cfloat>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>

#include "clib.hpp"
#include "color.hpp"
#include "imgui/include/imgui.h"
#include "imgui/include/imgui_impl_sdl.h"
#include "imgui/include/imgui_impl_sdlrenderer.h"

//#define CLOCK_RATE 4194304.0f
#define CLOCK_RATE 4200000.0f

#define MEM_SIZE 0xff * 0xff
#define ROM_SIZE 1024 * 1024

#define BOOT_LOGO_HEAD 0x104
#define BOOT_LOGO_END 0x133
#define BOOT_LOGO_SIZE BOOT_LOGO_END - BOOT_LOGO_HEAD
#define VRAM_HEAD 0x8000
#define VRAM_TILE_HEAD VRAM_HEAD
#define VRAM_TILE_END 0x97FF

#define TILE_NO_MIN 0
#define TILE_NO_MAX 191
#define TILE_X_PIX 8
#define TILE_Y_PIX 8

extern int gbemu_argc;
extern char** gbemu_argv;

struct Registers {
    union {
        struct {
            union {
                struct {
                    uint8_t unused : 4;
                    uint8_t c : 1;  // carry
                    uint8_t h : 1;  // half carry
                    uint8_t n : 1;  // add/sub (negative)
                    uint8_t z : 1;  // zero
                } flags;
                uint8_t f;
            };
            uint8_t a;
        };
        uint16_t af;
    };
    union {
        struct {
            uint8_t c;
            uint8_t b;
        };
        uint16_t bc;
    };
    union {
        struct {
            uint8_t e;
            uint8_t d;
        };
        uint16_t de;
    };
    union {
        struct {
            uint8_t l;
            uint8_t h;
        };
        uint16_t hl;
    };
    uint16_t sp;
    uint16_t pc;
};

enum MBC {
    MBC0,  // NO MBC
    MBC1,
    MBC2,
    MBC3
};

class GBEmu {
   private:
    bool end;
    Registers reg;
    uint8_t* ram;
    uint8_t* rom;
    uint8_t scale;

    // ppu内での状態管理に使う
    uint8_t ppu_mode;
    uint8_t ppu_mode_clock;        //各モードでのクロック数のカウンタ(CPU命令数を基準に加算していく)
    uint8_t ppu_line;              // ppuで描画した行数0 - 143(描画)、144-153(v-blank)
    uint16_t PPU_MODE_CLOCKS[4];   //各モードの処理にかかるクロック数
    uint16_t PPU_MODE_LINE_CLOCK;  // 1行の描画にかかるクロック数

    //最後に実行した命令のクロックサイクル数
    // PPUなどとの同期に使う
    uint8_t last_instr_clock;

    uint32_t tick;
    MBC mbc;

    SDL_Window* win_debug_gui;
    SDL_Renderer* rend_debug_gui;

    SDL_Window* win_ppu_tile;
    SDL_Renderer* rend_ppu_tile;

    static GBPalette palette;

    void dump_rom(uint32_t from, uint32_t bytes);
    void dump_regs();

    void _init();
    uint8_t read_mem(uint16_t addr);
    uint16_t read_mem_u16(uint16_t addr);
    void write_mem(uint16_t addr, uint8_t data);
    void write_mem_u16(uint16_t addr, uint16_t data);
    void push(uint16_t data);
    uint16_t pop(void);

    void _sdlinit(void);
    void display(void);
    void cpu_step(void);
    void ppu_step(void);

    void init_win_ppu_tile(void);
    void display_win_ppu_tile(void);
    void main_loop(void);
    void sdl_event(void);

    void init_win_debug_gui(void);
    void display_win_debug_gui(void);

   public:
    GBEmu();
    ~GBEmu();
    void run(const char rom_file_path[], bool flg_dump_regs, uint16_t exit_pc = 0);
};

enum IO_REG : uint16_t {
    LCDC = 0xff40,
    STAT = 0xff41,
    SCY = 0xff42,
    SCX = 0xff43,
    LY = 0xff44,
    LYC = 0xff45,
    DMA = 0xff46,
    BGP = 0xff47,
    OBP0 = 0xff48,
    OBP1 = 0xff49,
    WY = 0xff4a,
    WX = 0xff4b
};

enum PPU_MODE {
    PPU_MODE_0,  // H-Blank
    PPU_MODE_1,  // V-Blank
    PPU_MODE_2,  // Read OAM
    PPU_MODE_3   // Read VRAM and OAM
};

#endif