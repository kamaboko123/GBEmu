#include "gbemu.hpp"

#if !SDL_VERSION_ATLEAST(2, 0, 17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

int main(int argc, char *argv[])
{
    gbemu_argc = argc;
    gbemu_argv = argv;
    char *rom_file_name;

    bool dump_regs = false;
    if (argc < 2) {
        printf("[erorr] rom file is not specified in args.\n");
        rom_file_name = nullptr;
    }
    else {
        rom_file_name = gbemu_argv[1];
    }

    if (argc >= 3) {
        for (int i = 2; i < gbemu_argc; i++) {
            if (strcmp(gbemu_argv[i], "--regs") == 0) {
                dump_regs = true;
            }
        }
    }

    GBEmu *emu = new GBEmu();
    emu->run(argv[1], dump_regs, 0x200);
    delete emu;

    return 0;
}
