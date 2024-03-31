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

#ifndef SCHISM_SC_OPERATIONS_HPP
#define SCHISM_SC_OPERATIONS_HPP

#include <cstdint>

// enum scRegister
//   - Represents a register within the Schism VM runtime
//   - The lower half of registers are actually all virtual and do not physically exist
enum class scRegister : uint8_t {
    // ==================
    //  System Registers
    // ==================
    SP,
    IP,

    FB0,
    FB1,
    FB2,
    FB3,

    // ================
    //  User Registers
    // ================

    // M0 - Matrix 0
    //  V0
    S0, S1, S2, S3,

    //  V1
    S4, S5, S6, S7,

    //  V2
    S8, S9, S10, S11,

    //  V3
    S12, S13, S14, S15,

    // M1 - Matrix 1
    //  V4
    S16, S17, S18, S19,

    //  V5
    S20, S21, S22, S23,

    //  V6
    S24, S25, S26, S27,

    //  V7
    S28, S29, S30, S31,

    // =======================
    //  End of real registers
    // =======================
    REGISTER_COUNT,

    // ===================
    //  Virtual Registers
    // ===================
    V0 = 0xF0,
    V1 = 0xF1,
    V2 = 0xF2,
    V3 = 0xF3,
    V4 = 0xF4,
    V5 = 0xF5,
    V6 = 0xF6,
    V7 = 0xF7,

    M0 = 0xF8,
    M1 = 0xF9,

    UNKNOWN = 0xFF
};

enum class scInstructionGroup : uint8_t {
    GroupZero      = 0x0,
    GroupOne       = 0x1,
    GroupTwo       = 0x2
};

enum class scGroupZeroOperations : uint8_t {
    // =======================
    //  Group Zero Operations
    // =======================
    OpExitProgram  = 0x00,
};

enum class scGroupOneOperations : uint8_t {
    // ======================
    //  Group One Operations
    // ======================
    OpMOV       = 0x00,
    OpALUF32F32 = 0x01
};

enum class scGroupOneSubOperations : uint8_t {
    // =========================
    //  Group One SubOperations
    // =========================
    SubOpAdd = 0x00,
    SubOpSub = 0x01,
    SubOpMul = 0x02,
    SubOpDiv = 0x03,
    SubOpMod = 0x04,
    SubOpPow = 0x05,
};

enum class scGroupTwoOperations : uint8_t {
    // ======================
    //  Group Two Operations
    // ======================
    OpSetF32       = 0x00,
    OpLoadF32      = 0x01,
    OpABSF32       = 0x02
};

//extern const char* scGetOperationName(scOperation op);

#endif //SCHISM_SC_OPERATIONS_HPP
