#include "gbemu.hpp"

int main(int argc, char *argv[]) {
    gbemu_argc = argc;
    gbemu_argv = argv;

    if(argc < 2){
        printf("rom file is not specified in args.\n");
        return 1;
    }

    bool dump_regs = false;
    for(int i = 1; i < gbemu_argc; i++){
        if(strcmp(gbemu_argv[i], "--regs") == 0){
            dump_regs = true;
        }
    }

    GBEmu *emu = new GBEmu();
    emu->run(argv[1], dump_regs, 0x200);
    
    delete emu;
    return 0;
}
