#include "gbemu.hpp"

void GBEmu::init_win_debug_gui(void) {
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    win_debug_gui = SDL_CreateWindow(
        "[GBEmu(DEBUG)]",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640,
        480,
        window_flags);

    rend_debug_gui = SDL_CreateRenderer(
        win_debug_gui,
        -1,
        SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(win_debug_gui, rend_debug_gui);
    ImGui_ImplSDLRenderer_Init(rend_debug_gui);
}

void GBEmu::display_win_debug_gui(void) {
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Registers");
    ImGui::Text("af: $%04x\n", reg.af);
    ImGui::Text("bc: $%04x\n", reg.bc);
    ImGui::Text("de: $%04x\n", reg.de);
    ImGui::Text("hl: $%04x\n", reg.hl);
    ImGui::Text("sp: $%04x\n", reg.sp);
    ImGui::Text("pc: $%04x\n", reg.pc);
    ImGui::End();

    ImGui::Begin("LCD");
    ImGui::Text("mode: %d\n", ppu_mode);
    ImGui::Text("LY: %d($%02x)\n", ram[IO_REG::LY], ram[IO_REG::LY]);
    ImGui::End();

    ImGui::Render();
    SDL_SetRenderDrawColor(rend_debug_gui, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
    SDL_RenderClear(rend_debug_gui);
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(rend_debug_gui);
}