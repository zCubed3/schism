//====================================================================================
// BSD 3-Clause License
//
// Copyright (c) 2024, Liam Reese
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//====================================================================================

#include <iostream>
#include <array>
#include <vector>
#include <string>
#include <optional>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <thread>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <SDL.h>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>

#include <schism/sc_assembler.hpp>
#include <schism/sc_vm.hpp>

template<size_t START, size_t END>
void PrintRegisterTable(const scVM& vm, const char* pTable) {
    constexpr size_t RANGE = END - START;

    ImGui::BeginTable(pTable, RANGE, ImGuiTableFlags_Borders);

    for (int r = START; r < END; r++) {
        ImGui::TableSetupColumn(scGetRegisterName((scRegister) r));
    }

    ImGui::TableHeadersRow();

    ImGui::TableNextRow();

    for (int r = START; r < END; r++) {
        scRegister reg = (scRegister) r;

        ImGui::TableNextColumn();

        if (r >= (int) scRegister::FB0) {
            ImGui::Text("%f", vm.GetRegister(reg).f32);
        } else {
            ImGui::Text("%x", vm.GetRegister(reg).u32);
        }
    }

    ImGui::EndTable();
}

void RenderAsync(scVM& vm, SDL_Texture* pSurfaceTex, int curSurfaceWidth, int curSurfaceHeight) {
    uint8_t* pRenderPixels;
    int pitch;
    SDL_LockTexture(pSurfaceTex, nullptr, (void **) &pRenderPixels, &pitch);

    for (int y = 0; y < curSurfaceHeight; y++) {
        for (int x = 0; x < curSurfaceWidth; x++) {
            vm.ResetRegisters();

            vm.Poke<float>(0, x);
            vm.Poke<float>(sizeof(int), y);
            vm.ExecuteTillEnd();

            int index = (y * curSurfaceWidth * 4) + (x * 4);

            // A
            pRenderPixels[index + 3] = vm.GetRegister(scRegister::FB3).f32 * 255;

            // R
            pRenderPixels[index + 2] = vm.GetRegister(scRegister::FB0).f32 * 255;

            // G
            pRenderPixels[index + 1] = vm.GetRegister(scRegister::FB1).f32 * 255;

            // B
            pRenderPixels[index + 0] = vm.GetRegister(scRegister::FB2).f32 * 255;
        }
    }

    SDL_UnlockTexture(pSurfaceTex);
}

void RenderPixel() {

}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* pWindow = SDL_CreateWindow(
        "Schism GUI",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1024,
        768,
        SDL_WINDOW_RESIZABLE
    );

    // TODO: Only use the software renderer for macOS
    SDL_Renderer* pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED);

    SDL_Event sdlEvent;
    bool run = true;

    ImGuiContext* imCtx = ImGui::CreateContext();
    ImGui::SetCurrentContext(imCtx);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplSDL2_InitForSDLRenderer(pWindow, pRenderer);
    ImGui_ImplSDLRenderer2_Init(pRenderer);

    std::string strSource = "";

    scAssembler assembler;
    scAssembledProgram program;
    scAssemblerState lastAsmState = scAssemblerState::OK;

    scVM vm(512);
    vm.ResetRegisters();

    int curSurfaceWidth = 64;
    int curSurfaceHeight = 64;

    int newSurfaceWidth = 64;
    int newSurfaceHeight = 64;

    float displayDimensions[2] = { 64, 64 };

    int renderPoint[2] = {0, 0};

    SDL_Texture* pSurfaceTex = SDL_CreateTexture(
        pRenderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        curSurfaceWidth,
        curSurfaceHeight
    );

    vm.Poke<float>(sizeof(int) * 2, curSurfaceWidth - 1);
    vm.Poke<float>(sizeof(int) * 3, curSurfaceHeight - 1);

    bool autoStep = false;
    bool autoPixIsDone = false;
    bool needStepInit = false;
    int autoSubSteps = 1;

    //std::thread renderThread;

    while (run) {
        while (SDL_PollEvent(&sdlEvent)) {
            ImGui_ImplSDL2_ProcessEvent(&sdlEvent);

            if (sdlEvent.type == SDL_QUIT) {
                return false;
            }
        }

        // Auto stepping

        if (autoStep) {
            if (autoPixIsDone || needStepInit) {
                vm.ResetRegisters();

                if (autoPixIsDone) {
                    if (++renderPoint[0] >= curSurfaceWidth) {
                        renderPoint[0] = 0;
                        renderPoint[1]++;
                    }

                    if (renderPoint[1] >= curSurfaceHeight) {
                        autoStep = false;
                    }
                }

                vm.Poke<float>(0, renderPoint[0]);
                vm.Poke<float>(sizeof(int), renderPoint[1]);

                autoPixIsDone = false;
                needStepInit = false;
            } else {
                for (int s = 0; s < autoSubSteps; s++) {
                    if (!vm.ExecuteStep()) {
                        autoPixIsDone = true;
                        break;
                    }
                }


                uint8_t * pRenderPixels;
                int pitch;
                SDL_LockTexture(pSurfaceTex, nullptr, (void**) &pRenderPixels, &pitch);

                int index = (renderPoint[1] * curSurfaceWidth * 4) + (renderPoint[0] * 4);

                // A
                pRenderPixels[index + 3] = vm.GetRegister(scRegister::FB3).f32 * 255;

                // R
                pRenderPixels[index + 2] = vm.GetRegister(scRegister::FB0).f32 * 255;

                // G
                pRenderPixels[index + 1] = vm.GetRegister(scRegister::FB1).f32 * 255;

                // B
                pRenderPixels[index + 0] = vm.GetRegister(scRegister::FB2).f32 * 255;

                SDL_UnlockTexture(pSurfaceTex);
            }
        } else {
            autoPixIsDone = false;
            needStepInit = true;
        }

        ImGui_ImplSDL2_NewFrame();
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui::NewFrame();

        // ========================

        ImGui::ShowDemoWindow();

        //
        // Assembler GUI
        //
        ImGui::Begin("Assembler");

        if (ImGui::Button("Compile")) {
            lastAsmState = assembler.CompileSourceText(strSource, program);

            if (lastAsmState == scAssemblerState::OK) {
                vm.LoadProgram(program.CreateModule());
            }
        }

        if (lastAsmState != scAssemblerState::OK) {
            ImGui::Text("Compilation returned code %i", (int) lastAsmState);
        } else {
            ImGui::Text("Compilation successful!");
        }

        ImGui::InputTextMultiline("Source", &strSource, ImVec2(-FLT_MIN, -FLT_MIN));

        ImGui::End();

        //
        // VM Gui
        //
        ImGui::Begin("Virtual Machine");

        {
            if (ImGui::Button("Step")) {
                vm.ExecuteStep();
            }

            ImGui::SameLine();

            if (ImGui::Button("Render Surface")) {
                //if (renderThread.joinable())
                //    renderThread.join(); // Kill the previous thread first

                //renderThread = std::thread(RenderAsync, vm, pSurfaceTex, curSurfaceWidth, curSurfaceHeight);
                RenderAsync(vm, pSurfaceTex, curSurfaceWidth, curSurfaceHeight);
            }
        }
        {
            ImGui::Checkbox("Auto Step", &autoStep);

            ImGui::SameLine();

            ImGui::DragInt("Substeps", &autoSubSteps, 0.1F, 1, 1000);
        }
        {
            if (ImGui::Button("Reset")) {
                vm.ResetRegisters();
                renderPoint[0] = 0;
                renderPoint[1] = 0;
            }
        }

        if (ImGui::CollapsingHeader("Fragment Debug")) {
            ImVec4 curColor = ImVec4(
                vm.GetRegister(scRegister::FB0).f32,
                vm.GetRegister(scRegister::FB1).f32,
                vm.GetRegister(scRegister::FB2).f32,
                vm.GetRegister(scRegister::FB3).f32
            );

            ImGui::ColorButton("Fragment Color", curColor, 0, ImVec2(80, 80));
        }

        ImGui::End();

        //
        // Registers
        //
        ImGui::Begin("Registers");

        {
            ImGui::BeginTable("sc_sys_registers_container", 1, ImGuiTableFlags_Borders);

            ImGui::TableSetupColumn("SYSTEM REGISTERS");
            ImGui::TableHeadersRow();
            ImGui::TableNextColumn();

            PrintRegisterTable<0, 2>(vm, "sc_sys_registers");

            ImGui::TableNextColumn();

            PrintRegisterTable<2, 6>(vm, "sc_fb_registers");

            ImGui::EndTable();
        }

        ImGui::Spacing();

        {
            ImGui::BeginTable("sc_m0_registers_container", 1, ImGuiTableFlags_Borders);

            ImGui::TableSetupColumn("M0");
            ImGui::TableHeadersRow();

            ImGui::TableNextColumn();
            ImGui::BeginTable("sc_v0_registers_container", 1, ImGuiTableFlags_Borders);
            ImGui::TableSetupColumn("V0");
            ImGui::TableHeadersRow();

            ImGui::TableNextColumn();
            PrintRegisterTable<6, 10>(vm, "sc_v0_registers");

            ImGui::EndTable();

            ImGui::TableNextColumn();
            ImGui::BeginTable("sc_v1_registers_container", 1, ImGuiTableFlags_Borders);
            ImGui::TableSetupColumn("V1");
            ImGui::TableHeadersRow();

            ImGui::TableNextColumn();
            PrintRegisterTable<10, 14>(vm, "sc_v1_registers");

            ImGui::EndTable();

            ImGui::TableNextColumn();
            ImGui::BeginTable("sc_v2_registers_container", 1, ImGuiTableFlags_Borders);
            ImGui::TableSetupColumn("V2");
            ImGui::TableHeadersRow();

            ImGui::TableNextColumn();
            PrintRegisterTable<14, 18>(vm, "sc_v2_registers");

            ImGui::EndTable();

            ImGui::TableNextColumn();
            ImGui::BeginTable("sc_v3_registers_container", 1, ImGuiTableFlags_Borders);
            ImGui::TableSetupColumn("V1");
            ImGui::TableHeadersRow();

            ImGui::TableNextColumn();
            PrintRegisterTable<18, 22>(vm, "sc_v3_registers");

            ImGui::EndTable();

            ImGui::EndTable();
        }

        ImGui::Spacing();

        {
            ImGui::BeginTable("sc_m1_registers_container", 1, ImGuiTableFlags_Borders);

            ImGui::TableSetupColumn("M0");
            ImGui::TableHeadersRow();

            ImGui::TableNextColumn();
            ImGui::BeginTable("sc_v4_registers_container", 1, ImGuiTableFlags_Borders);
            ImGui::TableSetupColumn("V4");
            ImGui::TableHeadersRow();

            ImGui::TableNextColumn();
            PrintRegisterTable<22, 26>(vm, "sc_v4_registers");

            ImGui::EndTable();

            ImGui::TableNextColumn();
            ImGui::BeginTable("sc_v5_registers_container", 1, ImGuiTableFlags_Borders);
            ImGui::TableSetupColumn("V5");
            ImGui::TableHeadersRow();

            ImGui::TableNextColumn();
            PrintRegisterTable<26, 30>(vm, "sc_v5_registers");

            ImGui::EndTable();

            ImGui::TableNextColumn();
            ImGui::BeginTable("sc_v6_registers_container", 1, ImGuiTableFlags_Borders);
            ImGui::TableSetupColumn("V6");
            ImGui::TableHeadersRow();

            ImGui::TableNextColumn();
            PrintRegisterTable<30, 34>(vm, "sc_v6_registers");

            ImGui::EndTable();

            ImGui::TableNextColumn();
            ImGui::BeginTable("sc_v7_registers_container", 1, ImGuiTableFlags_Borders);
            ImGui::TableSetupColumn("V7");
            ImGui::TableHeadersRow();

            ImGui::TableNextColumn();
            PrintRegisterTable<34, 38>(vm, "sc_v7_registers");

            ImGui::EndTable();

            ImGui::EndTable();
        }

        ImGui::End();

        //
        // Assembled Program
        //
        ImGui::Begin("Loaded Program");

        if (vm.GetProgram().has_value()) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

            size_t idx = 0;
            for (uint8_t byte: vm.GetProgram()->GetCode()) {
                bool at = idx++ == vm.GetRegister(scRegister::IP).u32;

                ImGui::Text("0x%02x %c", byte, at ? '<' : ' ');
            }

            ImGui::PopStyleVar();
        }

        ImGui::End();

        //
        // Surface GUI
        //
        ImGui::Begin("Surface");

        ImGui::InputInt("Width", &newSurfaceWidth);
        ImGui::InputInt("Height", &newSurfaceHeight);

        if (ImGui::Button("Update")) {
            SDL_DestroyTexture(pSurfaceTex);

            curSurfaceWidth = newSurfaceWidth;
            curSurfaceHeight = newSurfaceHeight;

            pSurfaceTex = SDL_CreateTexture(
                pRenderer,
                SDL_PIXELFORMAT_ARGB8888,
                SDL_TEXTUREACCESS_STREAMING,
                curSurfaceWidth,
                curSurfaceHeight
            );

            vm.Poke<float>(sizeof(int) * 2, curSurfaceWidth - 1);
            vm.Poke<float>(sizeof(int) * 3, curSurfaceHeight - 1);
        }

        // TODO: Clamping

        ImGui::DragFloat2("Display Dimensions", displayDimensions, 0.01F);
        ImGui::Image((void*)pSurfaceTex, ImVec2(displayDimensions[0], displayDimensions[1]));

        ImGui::End();

        // ========================

        SDL_RenderClear(pRenderer);
        //SDL_RenderCopy(renderer, target_texture, NULL, NULL);

        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

        SDL_RenderPresent(pRenderer);
    }

    return 0;

    /*
    scAssembler assembler;

    scAssembledProgram program;
    scAssemblerState state = assembler.CompileSourceFile("./asm/tester.scsa", program);

    if (state != scAssemblerState::OK) {
        return 1;
    }

    program.WriteToFile("test.scsm");

    scModule module;

    module.LoadFromFile("test.scsm");

    const int WIDTH = 64;
    const int HEIGHT = 64;

    std::vector<char> bytes;
    bytes.resize(WIDTH * HEIGHT * 3);

    scVM<512> vm;

    // Tell the VM about our pixel info
    vm.Poke<float>(sizeof(int) * 2, WIDTH - 1);
    vm.Poke<float>(sizeof(int) * 3, HEIGHT - 1);

    std::cout << "[SCHISM] Executing test.scsm | " << WIDTH << "x" << HEIGHT << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    // For each pixel, we will execute the virtual machine, starting anew each time
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int stride = (y * WIDTH * 3) + (x * 3);

            vm.Poke<float>(0, x);
            vm.Poke<float>(sizeof(int), y);

            vm.ResetRegisters();
            vm.ExecuteModule(module);

            bytes[stride] = vm.GetRegister(scRegister::FB0).f32;
            bytes[stride + 1] = vm.GetRegister(scRegister::FB1).f32;
            bytes[stride + 2] = vm.GetRegister(scRegister::FB2).f32;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "[SCHISM] Test.scsm took " << millis.count() << "ms (" << micros.count() << "us) to execute" << std::endl;

    stbi_write_jpg("./test.jpg", WIDTH, HEIGHT, 3, bytes.data(), 100);

    //std::cout.flush();
    return 0;

    //vm.PrintStack();
    //vm.PrintRegisters();
     */
}
