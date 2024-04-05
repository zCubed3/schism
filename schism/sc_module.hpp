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

#ifndef SCHISM_SC_MODULE_HPP
#define SCHISM_SC_MODULE_HPP

#include <cstdint>
#include <vector>
#include <string>

#include <schism/sc_magic.hpp>

enum class scModuleType : uint16_t {
    Vertex = 0x0000,
    Fragment = 0x0001
};

enum class scModuleState {
    OK = 0,

    ReadOutOfBounds,

    FileNotFound,
    FileCorrupt,
};

#pragma pack(push, 1)
typedef struct scModuleHeader {
    scModuleType type;
    uint32_t len;
} scModuleHeader_t;
#pragma pack(pop)

// Represents a loaded shader module
class scModule {
protected:
    std::vector<uint8_t> _code {};

public:
    scModule(const std::vector<uint8_t>& code) {
        this->_code = code;
    }

    template<typename T>
    scModuleState ReadValue(uint32_t cur, T& outValue) const {
        if (cur + (sizeof(T) - 1) >= _code.size())
            return scModuleState::ReadOutOfBounds;

        outValue = *((T*)(_code.data() + cur));
        return scModuleState::OK;
    }

    scModuleState LoadFromFile(const std::string& path);

    [[nodiscard]]
    std::vector<uint8_t> GetCode() const {
        return _code;
    }
};

#endif //SCHISM_SC_MODULE_HPP
