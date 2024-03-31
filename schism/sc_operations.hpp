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
    OpALUF32F32 = 0x00
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
