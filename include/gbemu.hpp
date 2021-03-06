#ifndef GBEMU_GBEMU_H
#define GBEMU_GBEMU_H
#include <SDL.h>
#include <fcntl.h>
#include <io.h>

#include <cfloat>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <chrono>
#include <mutex>

#include "clib.hpp"
#include "color.hpp"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "SDL_thread.h"

//#define CLOCK_RATE 4194304.0f
#define CLOCK_RATE 4200000.0f

#define MEM_SIZE 1024 * 1024
#define ROM_SIZE 1024 * 1024
#define ROM_BANK_SIZE 0x4000

#define CARTRIDGE_RAM_BANK_SIZE  0x2000
#define CARTRIDGE_RAM_BANK_COUNT 4

#define BOOT_LOGO_HEAD 0x104
#define BOOT_LOGO_END 0x133
#define BOOT_LOGO_SIZE BOOT_LOGO_END - BOOT_LOGO_HEAD
#define VRAM_HEAD 0x8000
#define VRAM_TILE_HEAD VRAM_HEAD
#define VRAM_TILE_END 0x97FF

#define TILE_NO_MIN 0
#define TILE_NO_MAX 384
#define TILE_X_PIX 8
#define TILE_Y_PIX 8

#define BREAK_POINT_MAX 16
//デバッグウインドウに表示するスタック周辺のメモリの広さ
//スタックポインタを基準にここで指定した数値だけ上位と下位を2byte単位で表示する
#define DEBUG_SHOW_STACK_COUNT  8

#define FPS 60

typedef union {
    struct {
        bool bg_enable : 1;
        bool obj_enable : 1;
        bool obj_size : 1;
        bool bg_tile_map_show : 1;
        bool bg_window_tile_data : 1;
        bool window_enable : 1;
        bool window_tile_map_show : 1;
        bool lcd_enable : 1;
    };
    uint8_t lcd_control;
} IO_LCD_LCDC;

typedef union {
    struct {
        uint8_t mode : 2;
        bool lyc_ly_c : 1;
        bool h_blank : 1;
        bool v_blank : 1;
        bool oam_int : 1;
        bool lyc_int : 1;
        bool unused : 1;
    };
    uint8_t status;
} IO_LCD_STAT;

typedef union {
    struct {
        uint8_t unused : 3;
        bool joypad : 1;
        bool serial : 1;
        bool timmer : 1;
        bool lcd_stat : 1;
        bool vblank: 1;
    };
    uint8_t status;
} IO_IF_FLAG;



struct Registers {
    union {
        struct {
            union {
                struct {
                    uint8_t unused : 4;
                    bool c : 1;  // carry
                    bool h : 1;  // half carry
                    bool n : 1;  // add/sub (negative)
                    bool z : 1;  // zero
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


enum MBC: uint8_t {
    MBC0,  // NO MBC
    MBC1,
    MBC2,
    MBC3
};

struct MBCState {
    MBC type;
    bool ram_enable;
    uint8_t rom_bank_n;    //7bit(2 + 5bit)
    uint8_t ram_bank_n;    //2bit
    uint8_t bank_mode_sel;
};

class GBEmu {
private:
    bool stop;
    bool exit_emu;
    bool win_close;
    bool enable_debug;
    Registers reg;

    struct MBCState mbc_state;

    uint8_t* ram;
    uint8_t* rom;
    uint8_t* cartridge_ram;

    uint16_t fps_lim;
    uint16_t fps_max;

    uint8_t lcd_scale;

    std::mutex mtx_stop;

    bool debug_step_exec;
    bool debug_break;
    uint16_t debug_break_addr[BREAK_POINT_MAX];

    // ppu内での状態管理に使う
    uint16_t ppu_mode_clock;        //各モードでのクロック数のカウンタ(CPU命令数を基準に加算していく)
    uint16_t PPU_MODE_CLOCKS[4];   //各モードの処理にかかるクロック数
    uint16_t PPU_MODE_LINE_CLOCK;  // 1行の描画にかかるクロック数

    //最後に実行した命令のクロックサイクル数
    // PPUなどとの同期に使う
    uint8_t last_instr_clock;

    uint8_t cycle_nopfx[2][16 * 16] = {
        { //cycle with no branch
             4, 12,  8,  8,  4,  4,  8,  4, 20,  8,  8,  8,  4,  4,  8,  4,
             4, 12,  8,  8,  4,  4,  8,  4, 12,  8,  8,  8,  4,  4,  8,  4,
             8, 12,  8,  8,  4,  4,  8,  4,  8,  8,  8,  8,  4,  4,  8,  4,
             8, 12,  8,  8, 12, 12, 12,  4,  8,  8,  8,  8,  4,  4,  8,  4,
             4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
             4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
             4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
             8,  8,  8,  8,  8,  8,  4,  8,  4,  4,  4,  4,  4,  4,  8,  4,
             4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
             4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
             4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
             4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
             8, 12, 12, 16, 12, 16,  8, 16,  8, 16, 12,  4, 12, 24,  8, 16,
             8, 12, 12,  0, 12, 16,  8, 16,  8, 16, 12,  0, 12,  0,  8, 16,
            12, 12,  8,  0,  0, 16,  8, 16, 16,  4, 16,  0,  0,  0,  8, 16,
            12, 12,  8,  4,  0, 16,  8, 16, 12,  8, 16,  4,  0,  0,  8, 16
        },
        { //cycle with branch
             4, 12,  8,  8,  4,  4,  8,  4, 20,  8,  8,  8,  4,  4,  8,  4,
             4, 12,  8,  8,  4,  4,  8,  4, 12,  8,  8,  8,  4,  4,  8,  4,
            12, 12,  8,  8,  4,  4,  8,  4, 12,  8,  8,  8,  4,  4,  8,  4,
            12, 12,  8,  8, 12, 12, 12,  4, 12,  8,  8,  8,  4,  4,  8,  4,
             4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
             4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
             4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
             8,  8,  8,  8,  8,  8,  4,  8,  4,  4,  4,  4,  4,  4,  8,  4,
             4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
             4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
             4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
             4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
            20, 12, 16, 16, 24, 16,  8, 16, 20, 16, 16,  4, 24, 24,  8, 16,
            20, 12, 16,  0, 24, 16,  8, 16, 20, 16, 16,  0, 24,  0,  8, 16,
            12, 12,  8,  0,  0, 16,  8, 16, 16,  4, 16,  0,  0,  0,  8, 16,
            12, 12,  8,  4,  0, 16,  8, 16, 12,  8, 16,  4,  0,  0,  8, 16
        }
    };

    uint8_t cycle_pfx_cb[16 * 16] = {
         8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
         8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
         8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
         8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
         8,  8,  8,  8,  8,  8, 12,  8,  8,  8,  8,  8,  8,  8, 12,  8,
         8,  8,  8,  8,  8,  8, 12,  8,  8,  8,  8,  8,  8,  8, 12,  8,
         8,  8,  8,  8,  8,  8, 12,  8,  8,  8,  8,  8,  8,  8, 12,  8,
         8,  8,  8,  8,  8,  8, 12,  8,  8,  8,  8,  8,  8,  8, 12,  8,
         8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
         8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
         8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
         8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
         8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
         8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
         8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8,
         8,  8,  8,  8,  8,  8, 16,  8,  8,  8,  8,  8,  8,  8, 16,  8
    };

    uint8_t length_nopfx[16 * 16] = {
        1, 3, 1, 1, 1, 1, 2, 1, 3, 1, 1, 1, 1, 1, 2, 1,
        1, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
        2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
        2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 3, 3, 3, 1, 2, 1, 1, 1, 3, 1, 3, 3, 2, 1,
        1, 1, 3, 1, 3, 1, 2, 1, 1, 1, 3, 1, 3, 1, 2, 1,
        2, 1, 1, 1, 1, 1, 2, 1, 2, 1, 3, 1, 1, 1, 2, 1,
        2, 1, 1, 1, 1, 1, 2, 1, 2, 1, 3, 1, 1, 1, 2, 1
    };

    uint8_t length_pfx_cb[16 * 16] = {
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
    };

    uint32_t tick;

    SDL_Window* win_debug_gui;
    SDL_Renderer* rend_debug_gui;

    SDL_Window* win_ppu_tile;
    SDL_Renderer* rend_ppu_tile;

    SDL_Window* win_lcd;
    SDL_Renderer* rend_lcd;

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
    void init_regs(void);
    void init_io_ports(void);
    void cpu_step(void);
    void ppu_step(void);

    void init_win_ppu_tile(void);
    void display_win_ppu_tile(void);
    void init_win_lcd(void);
    void display_win_lcd(void);
    void sdl_loop(void);
    void sdl_event(void);
    int cpu_loop(void);
    void instrs_pfx_0xcb(uint8_t* opcode);

    void init_win_debug_gui(void);
    void init_imgui(void);
    void destory_imgui(void);
    void destroy_win_debug_gui(void);
    void display_win_debug_gui(void);

    static int cpu_loop_wrapper(void* data);
    bool is_break(uint16_t addr);
    
    bool half_carry_add(uint8_t a, uint8_t b);
    bool half_carry_sub(uint8_t a, uint8_t b);
    bool carry_add(uint8_t a, uint8_t b);
    bool carry_sub(uint8_t a, uint8_t b);
    bool half_carry_add_u16(uint16_t a, uint16_t b);
    bool carry_add_u16(uint16_t a, uint16_t b);

    void draw_tile(SDL_Renderer* r, uint8_t t, uint16_t x, uint16_t y, int16_t offset_x, int16_t offset_y);

    void _cpu_ld_r8_imm8(uint8_t* r);
    void _cpu_ld_r16_imm16(uint16_t* r);
    void _cpu_inc_r16(uint16_t* r);
    void _cpu_inc_r8(uint8_t* r);
    void _cpu_dec_r8(uint8_t* r);
    void _cpu_add_r16_r16(uint16_t* r1, uint16_t* r2);
    void _cpu_dec_r16(uint16_t* r);
    void _cpu_ld_memimm16_r16(uint16_t r);
    void _cpu_rotate_left_carry_r8(uint8_t* r);
    void _cpu_rotate_right_carry_r8(uint8_t* r);
    void _cpu_rotate_left_r8(uint8_t* r);
    void _cpu_rotate_right_r8(uint8_t* r);
    bool _cpu_jmp_if_relative_imm8(bool flg);
    void _cpu_complement_r8(uint8_t* r);
    void _cpu_inc_mem(uint16_t addr);
    void _cpu_dec_mem(uint16_t addr);
    void _cpu_ld_mem_imm8(uint16_t addr);
    void _cpu_jmp_relative_imm8(void);
    void _cpu_add_rega_r8(uint8_t* r);
    void _cpu_add_rega_r8_carry(uint8_t* r);
    void _cpu_add_rega_mem(uint16_t addr);
    void _cpu_add_rega_mem_carry(uint16_t addr);
    void _cpu_sub_rega_r8(uint8_t* r);
    void _cpu_sub_rega_r8_carry(uint8_t* r);
    void _cpu_sub_rega_mem(uint16_t addr);
    void _cpu_sub_rega_mem_carry(uint16_t addr);
    void _cpu_and_rega_r8(uint8_t* r);
    void _cpu_and_rega_mem(uint16_t addr);
    void _cpu_xor_rega_r8(uint8_t* r);
    void _cpu_xor_rega_mem(uint16_t addr);
    void _cpu_or_rega_r8(uint8_t* r);
    void _cpu_or_rega_mem(uint16_t addr);
    void _cpu_compare_rega_r8(uint8_t* r);
    void _cpu_compare_rega_mem(uint16_t addr);
    bool _cpu_ret_if(bool flg);
    bool _cpu_jmp_if_imm16(bool flg);
    bool _cpu_call_if_imm16(bool flg);
    void _cpu_restart(uint16_t addr);
    void _cpu_add_r16_imm8s(uint16_t* r);
    void _cpu_daa(void);

    void _cpu_ex_rotate_left_r8(uint8_t* r);
    void _cpu_ex_rotate_left_mem(uint16_t addr);
    void _cpu_ex_rotate_right_r8(uint8_t* r);
    void _cpu_ex_rotate_right_mem(uint16_t addr);
    void _cpu_ex_rotate_left_carry_r8(uint8_t* r);
    void _cpu_ex_rotate_left_carry_mem(uint16_t addr);
    void _cpu_ex_rotate_right_carry_r8(uint8_t* r);
    void _cpu_ex_rotate_right_carry_mem(uint16_t addr);
    void _cpu_ex_shift_left_r8(uint8_t* r);
    void _cpu_ex_shift_left_mem(uint16_t addr);
    void _cpu_ex_shift_right_arith_r8(uint8_t* r);
    void _cpu_ex_shift_right_arith_mem(uint16_t addr);
    void _cpu_ex_swap_r8(uint8_t* r);
    void _cpu_ex_swap_mem(uint16_t addr);
    void _cpu_ex_shift_right_r8(uint8_t* r);
    void _cpu_ex_shift_right_mem(uint16_t addr);

   public:
    GBEmu();
    ~GBEmu();
    void run(const char rom_file_path[]);
};

enum IO_REG : uint16_t {
    IE   = 0xffff,
    IF   = 0xff0f,
    LCDC = 0xff40,
    STAT = 0xff41,
    SCY  = 0xff42,
    SCX  = 0xff43,
    LY   = 0xff44,
    LYC  = 0xff45,
    DMA  = 0xff46,
    BGP  = 0xff47,
    OBP0 = 0xff48,
    OBP1 = 0xff49,
    WY   = 0xff4a,
    WX   = 0xff4b
};

enum PPU_MODE {
    PPU_MODE_0 = 0,  // H-Blank
    PPU_MODE_1 = 1,  // V-Blank
    PPU_MODE_2 = 2,  // Read OAM
    PPU_MODE_3 = 3   // Read VRAM and OAM
};

#endif