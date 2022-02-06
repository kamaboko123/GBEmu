#include "gbemu.hpp"

#if !SDL_VERSION_ATLEAST(2, 0, 17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    char rom_file_name[] = "res/cpu_instrs.gb";
    bool dump_regs = false;

    GBEmu *emu = new GBEmu();
    emu->run(rom_file_name);
    delete emu;

    return 0;
}
