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

enum class scOperation : uint16_t {
    // =================
    //  VM Manipulation
    // =================
    OpExitProgram  = 0x0000,

    OpTXROut0      = 0x0050,
    OpTXROut1      = 0x0051,
    OpTXROut2      = 0x0052,
    OpTXROut3      = 0x0053,

    OpStoreF32     = 0x00A0,
    OpStoreF64     = 0x00A1,
    OpStoreI16     = 0x00A2,
    OpStoreI32     = 0x00A3,

    OpLoadF32      = 0x00C0,
    OpLoadF64      = 0x00C1,
    OpLoadI16      = 0x00C2,
    OpLoadI32      = 0x00C3,

    // ==================
    //  Stack operations
    // ==================
    OpPushF32      = 0x0100,
    OpPushF64      = 0x0101,
    OpPushI16      = 0x0102,
    OpPushI32      = 0x0103,

    // =================
    //  Math Operations
    // =================
    OpAddF32F32   = 0x0200,
    OpSubF32F32   = 0x0201,
    OpMulF32F32   = 0x0202,
    OpDivF32F32   = 0x0203,
};

extern const char* scGetOperationName(scOperation op);

#endif //SCHISM_SC_OPERATIONS_HPP
