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

#ifndef SCHISM_SC_VM_HPP
#define SCHISM_SC_VM_HPP

#include <cstdint>

#include <array>
#include <vector>
#include <memory>
#include <optional>

// TODO: Not pull in as many dependencies
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

extern const char* scGetRegisterName(scRegister regIndex);

// Represents the virtual machine that handles state for a provided scModule
class scVM {
protected:
    std::array<scVariable, 256> _stack {};
    std::vector<uint8_t> _memory {};

    std::array<scValue_u, static_cast<int>(scRegister::REGISTER_COUNT)> _registers;

    std::optional<scModule> _program;

    // ===============
    //  Ctor and Dtor
    // ===============
public:
    scVM(size_t memSize);

public:
    // ======================
    //  Program Manipulation
    // ======================
    void LoadProgram(const scModule& module);

    [[nodiscard]]
    std::optional<scModule> GetProgram() const {
        return _program;
    }

    //void LoadFragProgram(const scModule& module);

    // =======================
    //  Register Manipulation
    // =======================
    void MoveInstructionPointer(int offset);

    void SetRegister(scRegister regIndex, const scValue_u& value);

    [[nodiscard]]
    scValue_u GetRegister(scRegister regIndex) const {
        return _registers[static_cast<int>(regIndex)];
    }

    // =====================
    //  Memory Manipulation
    // =====================
    template<typename T>
    bool Poke(uint32_t index, T value) {
        if (index + sizeof(T) - 1 > _memory.size())
            return false;

        uint8_t* valPtr = (uint8_t*)&value;
        for (int m = 0; m < sizeof(T); m++) {
            _memory[index + m] = valPtr[m];
        }

        return true;
    }

    template<typename T>
    bool ReadValue(uint32_t cur, T& outValue) const {
        if (cur + (sizeof(T) - 1) >= _memory.size())
            return false;

        outValue = *((T*)(_memory.data() + cur));
        return true;
    }

    // ===========
    //  Debugging
    // ===========
    void PrintVariable(const scVariable& variable);

    void PrintStack();

    void PrintRegisters();

    // ====================
    //  Stack Manipulation
    // ====================
    void PushValue(scValue_u value, scValueType type);

    bool PopValue(scVariable& operand);

    // ===================
    //  Program Execution
    // ===================
    bool ExecuteOperation(const scModule& pModule, uint32_t encoded);

    void ResetRegisters();

    void ExecuteTillEnd();

    bool ExecuteStep();
};

#endif //SCHISM_SC_VM_HPP
