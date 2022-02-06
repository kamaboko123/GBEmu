#include "gbemu.hpp"

#if !SDL_VERSION_ATLEAST(2, 0, 17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    char rom_file_name[] = "res/cpu_instrs.gb";
    bool dump_regs = false;

    GBEmu *emu = new GBEmu();
    emu->run(rom_file_name);
    delete emu;

    return 0;
}
