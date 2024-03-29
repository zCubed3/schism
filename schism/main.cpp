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

// Represents the virtual machine that handles state for a provided scModule
template<size_t MEM_SIZE>
class scVM {
protected:
    std::array<scVariable, 256> _stack {};
    std::array<uint8_t, MEM_SIZE> _memory {};

    uint32_t _regIP = 0;
    uint32_t _regSP = 0;

    // VERTEX PROGRAM
    //  - Vertex programs output the clipping XYZW into these registers
    //  - Fragment programs output the framebuffer contents into these registers
    float _regOUT0 = 0;
    float _regOUT1 = 0;
    float _regOUT2 = 0;
    float _regOUT3 = 0;

    // FRAGMENT PROGRAM

public:
    // =========
    //  Getters
    // =========
    [[nodiscard]]
    float GetRegOUT0() const {
        return _regOUT0;
    }

    [[nodiscard]]
    float GetRegOUT1() const {
        return _regOUT1;
    }

    [[nodiscard]]
    float GetRegOUT2() const {
        return _regOUT2;
    }

    [[nodiscard]]
    float GetRegOUT3() const {
        return _regOUT3;
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

        for (int s = _regSP - 1; s >= 0; s--) {
            scVariable variable = _stack[s];

            std::cout << s << " : ";
            PrintVariable(variable);
            std::cout << std::endl;
        }

        std::cout << "-============================-\n";
    }
    void PrintRegisters() {
        std::cout << "-======= SCHISM REGISTERS =======-\n";

        std::cout << "IP = " << _regIP << std::endl;
        std::cout << "SP = " << _regSP << std::endl;
        std::cout << "OUT0 = " << _regOUT0 << std::endl;
        std::cout << "OUT1 = " << _regOUT1 << std::endl;
        std::cout << "OUT2 = " << _regOUT2 << std::endl;
        std::cout << "OUT3 = " << _regOUT3 << std::endl;

        std::cout << "-================================-\n";
    }

    // ====================
    //  Stack Manipulation
    // ====================
    void PushValue(scValue_u value, scValueType type) {
        _stack[_regSP] = { value, type };
        _regSP++;

        // TODO: Detect a stack overflow
    }

    bool PopValue(scVariable& operand) {
        // TODO: Detect stack underflow
        if (_regSP - 1 == 0xFF)
            return false;

        operand = _stack[--_regSP];
        return true;
    }

    // ===================
    //  Program Execution
    // ===================
    bool ExecuteOperation(const scModule& module, scOperation op) {
        //std::cout << "EXECUTING: " << scGetOperationName(op) << "\n";

        switch (op) {
            // =================
            //  VM Manipulation
            // =================
            case scOperation::OpExitProgram: {
                return false;
            }

            case scOperation::OpTXROut0:
            case scOperation::OpTXROut1:
            case scOperation::OpTXROut2:
            case scOperation::OpTXROut3: {
                scVariable variable;

                if (!PopValue(variable))
                    return false;

                switch (op) {
                    case scOperation::OpTXROut0:
                        _regOUT0 = variable.value.f32;
                        break;

                    case scOperation::OpTXROut1:
                        _regOUT1 = variable.value.f32;
                        break;

                    case scOperation::OpTXROut2:
                        _regOUT2 = variable.value.f32;
                        break;

                    case scOperation::OpTXROut3:
                        _regOUT3 = variable.value.f32;
                        break;
                }

                break;
            }

            case scOperation::OpLoadF32: {
                scValue_u loadPtr, value;

                // Where?
                if (module.ReadValue<uint32_t>(_regIP, loadPtr.u32) != scModuleState::OK)
                    return false;

                _regIP += sizeof(uint32_t);

                if (!ReadValue(loadPtr.u32, value.f32))
                    return false;

                PushValue(value, scValueType::F32);

                break;
            }

            // =================
            //  Push operations
            // =================
            case scOperation::OpPushF32: {
                scValue_u value;

                if (module.ReadValue<float>(_regIP, value.f32) != scModuleState::OK)
                    return false;

                _regIP += 4;

                PushValue(value, scValueType::F32);

                break;
            }

            case scOperation::OpPushI16: {
                scValue_u value;

                if (module.ReadValue<float>(_regIP, value.f32) != scModuleState::OK)
                    return false;

                _regIP += 2;

                PushValue(value, scValueType::I16);

                break;
            }

            // =================
            //  Math Operations
            // =================
            case scOperation::OpMulF32F32: {
                scVariable inLhs, inRhs;

                if (!PopValue(inLhs))
                    return false;

                if (!PopValue(inRhs))
                    return false;

                // TODO: Type check?

                scValue_u value;
                value.f32 = inLhs.value.f32 * inRhs.value.f32;

                PushValue(value, scValueType::F32);

                break;
            }

            case scOperation::OpDivF32F32: {
                scVariable inLhs, inRhs;

                if (!PopValue(inLhs))
                    return false;

                if (!PopValue(inRhs))
                    return false;

                scValue_u value;
                value.f32 = inLhs.value.f32 / inRhs.value.f32;

                PushValue(value, scValueType::F32);

                break;
            }
        }

        //PrintStack();

        return true;
    }

    void ResetRegisters() {
        _regIP = 0;
        _regSP = 0;
        _regOUT0 = 0;
        _regOUT1 = 0;
        _regOUT2 = 0;
        _regOUT3 = 0;
    }

    void ExecuteModule(const scModule& module) {
        bool alive = true;

        while (alive) {
            scOperation op;

            // TODO: Report the VM CRASH!
            if (module.ReadValue<scOperation>(_regIP, op) != scModuleState::OK)
                return;

            _regIP += sizeof(scOperation);

            alive = ExecuteOperation(module, op);
        }
    }
};

int main() {
    scAssembler assembler;

    scAssembledProgram program;
    scAssemblerState state = assembler.CompileSourceFile("test.scsa", program);

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

    // For each pixel, we will execute the virtual machine, starting anew each time
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int stride = (y * WIDTH * 3) + (x * 3);

            vm.Poke<float>(0, x);
            vm.Poke<float>(sizeof(int), y);

            vm.ResetRegisters();
            vm.ExecuteModule(module);

            bytes[stride] = vm.GetRegOUT0();
            bytes[stride + 1] = vm.GetRegOUT1();
            bytes[stride + 2] = vm.GetRegOUT2();
        }
    }

    stbi_write_jpg("./test.jpg", WIDTH, HEIGHT, 3, bytes.data(), 100);

    //std::cout.flush();
    return 0;

    //vm.PrintStack();
    //vm.PrintRegisters();
}
