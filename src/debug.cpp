#include "gbemu.hpp"

void GBEmu::init_imgui(void)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
}

void GBEmu::destory_imgui(void)
{
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void GBEmu::init_win_debug_gui(void)
{
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    win_debug_gui = SDL_CreateWindow(
        "[GBEmu(DEBUG)]",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        1280,
        960,
        window_flags);

    rend_debug_gui = SDL_CreateRenderer(
        win_debug_gui,
        -1,
        SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

    ImGui_ImplSDL2_InitForSDLRenderer(win_debug_gui, rend_debug_gui);
    ImGui_ImplSDLRenderer_Init(rend_debug_gui);

    float scale = 1.0f;
    SDL_RenderSetScale(rend_debug_gui, scale, scale);
}

void GBEmu::destroy_win_debug_gui(void)
{
    SDL_DestroyRenderer(rend_debug_gui);
    SDL_DestroyWindow(win_debug_gui);
}

void GBEmu::display_win_debug_gui(void)
{
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    {
        ImGui::Begin("Registers");
        bool z = reg.flags.z;
        bool n = reg.flags.n;
        bool h = reg.flags.h;
        bool c = reg.flags.c;

        ImGui::Text("af: $%04x\n", reg.af);
        ImGui::Text("bc: $%04x\n", reg.bc);
        ImGui::Text("de: $%04x\n", reg.de);
        ImGui::Text("hl: $%04x\n", reg.hl);
        ImGui::Text("sp: $%04x\n", reg.sp);
        ImGui::Text("pc: $%04x\n", reg.pc);
        ImGui::Separator();

        ImGui::Text("[Flags]");
        ImGui::Checkbox("z", &z);
        ImGui::Checkbox("n", &n);
        ImGui::Checkbox("h", &h);
        ImGui::Checkbox("c", &c);
        ImGui::End();
    }

    {
        ImGui::Begin("LCD");
        ImGui::Text("mode: %d\n", ((IO_LCD_STAT *) &ram[IO_REG::STAT])->mode);
        ImGui::Text("STAT[FF41]: %d($%02x)\n", ram[IO_REG::STAT], ram[IO_REG::STAT]);
        ImGui::Text("SXY[FF42] : %d($%02x)\n", ram[IO_REG::SCY], ram[IO_REG::SCY]);
        ImGui::Text("SCY[FF43] : %d($%02x)\n", ram[IO_REG::SCX], ram[IO_REG::SCX]);
        ImGui::Text("LY[FF44]  : %d($%02x)\n", ram[IO_REG::LY], ram[IO_REG::LY]);
        ImGui::Text("LYC[FF45] : %d($%02x)\n", ram[IO_REG::LYC], ram[IO_REG::LYC]);
        ImGui::Separator();
        ImGui::Text("Instruction per sec: %d\n", fps_max);
        ImGui::End();
    }

    ImGui::Render();
    SDL_SetRenderDrawColor(rend_debug_gui, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
    SDL_RenderClear(rend_debug_gui);
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(rend_debug_gui);
}