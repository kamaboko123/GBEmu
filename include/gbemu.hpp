#ifndef GBEMU_GBEMU_H
#define GBEMU_GBEMU_H
#include <fcntl.h>
#include <unistd.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include <SDL2/SDL.h>
#include <gtk/gtk.h>

#include "clib.hpp"
#include "color.hpp"

#define MEM_SIZE 0xff * 0xff
#define ROM_SIZE 1024 * 1024

extern int gbemu_argc;
extern char **gbemu_argv;

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

enum MBC{
    MBC0, //NO MBC
    MBC1,
    MBC2,
    MBC3
};

class GBEmu {
   private:
    Registers reg;
    uint8_t *ram;
    uint8_t *rom;
    uint8_t scale;
    MBC mbc;
    SDL_Window* window;
    SDL_Renderer* renderer;
    
    static GBPalette palette;

    void dump_rom(uint32_t from, uint32_t bytes);
    void dump_regs();

    void _init();
    uint8_t read_mem(uint16_t addr);
    uint16_t read_mem_u16(uint16_t addr);
    void write_mem(uint16_t addr, uint8_t data);
    void write_mem_u16(uint16_t addr, uint16_t data);
    void push(uint16_t data);
    uint16_t pop();

    void _sdlinit(void);
    void display(void);

   public:
    GBEmu();
    ~GBEmu();
    void run(const char rom_file_path[], bool flg_dump_regs, uint16_t exit_pc=0);
};
#endif