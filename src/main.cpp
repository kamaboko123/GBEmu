#include "gbemu.hpp"

static_assert(-1 >> 1 == -1, "signed right shift is not arithmetic.");

#if !SDL_VERSION_ATLEAST(2, 0, 17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <tchar.h>

int _tmain(int argc, _TCHAR* argv[])
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    char rom_file_name[] = "res/cpu_instrs.gb";
    bool dump_regs = false;

    GBEmu *emu = new GBEmu();
    emu->run(rom_file_name);
    delete emu;
    _CrtDumpMemoryLeaks();
    return 0;
}
