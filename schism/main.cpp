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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <schism/sc_module.hpp>
#include <schism/sc_operations.hpp>
#include <schism/sc_assembler.hpp>


typedef union scValue {
    int16_t i16;
    int32_t i32;

    uint16_t u16;
    uint32_t u32;

    float f32;
} scValue_u;

enum class scValueType : uint16_t {
    F32,
    F64,

    I16,
    I32,
};

struct scVariable {
public:
    scValue_u value;
    scValueType type;
};

const char* GetRegisterName(scRegister regIndex) {
    switch (regIndex) {
        case scRegister::IP:
            return "IP";

        case scRegister::SP:
            return "SP";

        case scRegister::V0:
            return "V0";

        case scRegister::V1:
            return "V1";

        case scRegister::V2:
            return "V2";

        case scRegister::V3:
            return "V3";

        case scRegister::V4:
            return "V4";

        case scRegister::V5:
            return "V5";

        case scRegister::V6:
            return "V6";

        case scRegister::V7:
            return "V7";
    }

    return nullptr;
}

// Represents the virtual machine that handles state for a provided scModule
template<size_t MEM_SIZE>
class scVM {
protected:
    std::array<scVariable, 256> _stack {};
    std::array<uint8_t, MEM_SIZE> _memory {};

    std::array<scValue_u, static_cast<int>(scRegister::REGISTER_COUNT)> registers;

    // FRAGMENT PROGRAM

public:
    // =======================
    //  Register Manipulation
    // =======================
    void MoveInstructionPointer(int offset) {
        registers[static_cast<int>(scRegister::IP)].u32 += offset;
    }

    scValue_u GetRegister(scRegister regIndex) const {
        return registers[static_cast<int>(regIndex)];
    }

    void SetRegister(scRegister regIndex, const scValue_u& value) {
        registers[static_cast<int>(regIndex)] = value;
    }

    // =====================
    //  Memory Manipulation
    // =====================
    template<typename T>
    bool Poke(uint32_t index, T value) {
        if (index + sizeof(T) - 1 > MEM_SIZE)
            return false;

        uint8_t* valPtr = (uint8_t*)&value;
        for (int m = 0; m < sizeof(T); m++) {
            _memory[index + m] = valPtr[m];
        }

        return true;
    }

    template<typename T>
    bool ReadValue(uint32_t cur, T& outValue) const {
        if (cur + (sizeof(T) - 1) >= MEM_SIZE)
            return false;

        outValue = *((T*)(_memory.data() + cur));
        return true;
    }

    // ===========
    //  Debugging
    // ===========
    void PrintVariable(const scVariable& variable) {
        std::cout << std::hex;

        switch (variable.type) {
            case scValueType::F32:
                std::cout << "(F32) = " << std::dec << variable.value.f32;
                break;

            //case scValueType::SC_TYPE_F64:
            //    std::cout << "(F64) = 0x" << operand.value.f64;
            //    break;

            case scValueType::I16:
                std::cout << "(I16) = 0x" << variable.value.i16;
                break;

            case scValueType::I32:
                std::cout << "(I32) = 0x" << variable.value.i32;
                break;
        }

        std::cout << std::dec;
    }

    void PrintStack() {
        std::cout << "-======= SCHISM STACK =======-\n";

        for (int s = GetRegister(scRegister::SP).u32 - 1; s >= 0; s--) {
            scVariable variable = _stack[s];

            std::cout << s << " : ";
            PrintVariable(variable);
            std::cout << std::endl;
        }

        std::cout << "-============================-\n";
    }
    void PrintRegisters() {
        std::cout << "-======= SCHISM REGISTERS =======-\n";

        std::cout << std::hex;

        for (int r = 0; r < (int)scRegister::REGISTER_COUNT; r++) {
            scRegister reg = (scRegister)r;
            std::cout << GetRegisterName(reg) << GetRegister(reg).u32 << std::endl;
        }

        std::cout << std::dec;

        std::cout << "-================================-\n";
    }

    // ====================
    //  Stack Manipulation
    // ====================
    void PushValue(scValue_u value, scValueType type) {
        scValue_u sp = GetRegister(scRegister::SP);

        _stack[sp.u32++] = { value, type };

        SetRegister(scRegister::SP, sp);

        // TODO: Detect a stack overflow
    }

    bool PopValue(scVariable& operand) {
        scValue_u sp = GetRegister(scRegister::SP);

        // TODO: Detect stack underflow
        if (sp.u32 - 1 == 0xFF)
            return false;

        operand = _stack[--sp.u32];
        SetRegister(scRegister::SP, sp);

        return true;
    }

    // ===================
    //  Program Execution
    // ===================
    bool ExecuteOperation(const scModule& module, uint32_t encoded) {
        //std::cout << "EXECUTING: " << scGetOperationName(op) << "\n";

        // Begin to decode the instruction
        scInstructionGroup group = (scInstructionGroup)(encoded & 0xF);

        switch (group) {
            case scInstructionGroup::GroupZero: {
                scGroupZeroOperations op = (scGroupZeroOperations)((encoded >> 4) & 0xFF);
                switch (op) {
                    case scGroupZeroOperations::OpExitProgram:
                        return false;
                }
            }

            case scInstructionGroup::GroupOne: {
                scGroupOneOperations op = (scGroupOneOperations)((encoded >> 4) & 0xFF);
                scGroupOneSubOperations subOp = (scGroupOneSubOperations)((encoded >> 12) & 0xF);

                scRegister aRegister = (scRegister)((encoded >> 16) & 0xFF);
                scRegister bRegister = (scRegister)((encoded >> 24) & 0xFF);

                switch (op) {
                    case scGroupOneOperations::OpMOV: {
                        SetRegister(aRegister, GetRegister(bRegister));
                        break;
                    }

                    case scGroupOneOperations::OpALUF32F32: {
                        int simd = 1;

                        // TODO: Add a helper rather than this bs
                        if (aRegister == scRegister::V0) {
                            simd = 4;
                            aRegister = scRegister::S0;
                        }

                        if (bRegister == scRegister::V1) {
                            simd = 4;
                            bRegister = scRegister::S4;
                        }


                        for (int d = 0; d < simd; d++) {
                            scValue_u aValue = GetRegister((scRegister)((int)aRegister + d));
                            scValue_u bValue = GetRegister((scRegister)((int)bRegister + d));

                            //std::cout << "LHS | " << (int)aRegister + d << " | " << aValue.f32 << std::endl;
                            //std::cout << "RHS | " << (int)bRegister + d << " | " << bValue.f32 << std::endl;

                            switch (subOp) {
                                case scGroupOneSubOperations::SubOpAdd: {
                                    aValue.f32 = aValue.f32 + bValue.f32;
                                    break;
                                }

                                case scGroupOneSubOperations::SubOpSub: {
                                    aValue.f32 = aValue.f32 - bValue.f32;
                                    break;
                                }

                                case scGroupOneSubOperations::SubOpMul: {
                                    aValue.f32 = aValue.f32 * bValue.f32;
                                    break;
                                }

                                case scGroupOneSubOperations::SubOpDiv: {
                                    aValue.f32 = aValue.f32 / bValue.f32;
                                    break;
                                }

                                case scGroupOneSubOperations::SubOpMod: {
                                    aValue.f32 = std::fmod(aValue.f32, bValue.f32);
                                    break;
                                }

                                case scGroupOneSubOperations::SubOpPow: {
                                    aValue.f32 = powf(aValue.f32, bValue.f32);
                                    break;
                                }
                            }

                            SetRegister((scRegister)((int)aRegister + d), aValue);
                        }

                        break;
                    }
                }

                break;
            }

            case scInstructionGroup::GroupTwo: {
                scGroupTwoOperations op = (scGroupTwoOperations)((encoded >> 4) & 0xFF);
                scRegister targetRegister = (scRegister)((encoded >> 12) & 0xFF);

                // TODO: Break this into functions
                switch (op) {
                    case scGroupTwoOperations::OpSetF32: {
                        scValue_u value;
                        module.ReadValue(GetRegister(scRegister::IP).u32, value.f32);

                        MoveInstructionPointer(sizeof(float));

                        SetRegister(targetRegister, value);
                        break;
                    }

                    case scGroupTwoOperations::OpLoadF32: {
                        uint32_t ptr;
                        module.ReadValue(GetRegister(scRegister::IP).u32, ptr);

                        MoveInstructionPointer(sizeof(uint32_t));

                        scValue_u value;

                        if (!ReadValue(ptr, value.f32)) {
                            return false;
                        }

                        SetRegister(targetRegister, value);
                        break;
                    }

                    case scGroupTwoOperations::OpABSF32: {
                        scValue_u value = GetRegister(targetRegister);

                        value.f32 = fabs(value.f32);

                        SetRegister(targetRegister, value);
                    }
                }

                break;
            }
        }

        //PrintStack();

        return true;
    }

    void ResetRegisters() {
        for (int r = 0; r < (int)scRegister::REGISTER_COUNT; r++) {
            registers[r].u32 = 0;
        }
    }

    void ExecuteModule(const scModule& module) {
        bool alive = true;

        while (alive) {
            // TODO: Report the VM CRASH!
            uint32_t encoded;
            if (module.ReadValue<uint32_t>(GetRegister(scRegister::IP).u32, encoded) != scModuleState::OK)
                return;

            MoveInstructionPointer(sizeof(uint32_t));

            alive = ExecuteOperation(module, encoded);
        }
    }
};

int main() {
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
}
