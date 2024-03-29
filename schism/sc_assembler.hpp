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

#ifndef SCHISM_SC_ASSEMBLER_HPP
#define SCHISM_SC_ASSEMBLER_HPP

#include <string>
#include <vector>
#include <optional>

#include <schism/sc_module.hpp>

enum class scAssemblerState {
    OK,

    UnknownInstruction,
    InvalidArgument
};

class scAssembledProgram {
public:
    scModuleHeader header;
    std::vector<uint8_t> binary;

    scAssembledProgram() = default;
    scAssembledProgram(const std::vector<uint8_t>& binary, scModuleType type);

    void WriteToFile(const std::string& path);
};

class scAssembler {
public:
    template<class T>
    void Emit(std::vector<uint8_t>& program, T value) {
        uint8_t* valPtr = (uint8_t*)&value;
        for (int x = 0; x < sizeof(T); x++) {
            program.emplace_back(valPtr[x]);
        }
    }

    scAssemblerState CompileSourceFile(const std::string& path, scAssembledProgram& outProgram);

public:
    bool TryParseFloat(const std::string& str, float& out);

    bool TryParseHex(const std::string& str, uint32_t& out);
};

#endif //SCHISM_SC_ASSEMBLER_HPP
