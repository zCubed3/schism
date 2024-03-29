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

#include "sc_assembler.hpp"

#include <fstream>

#include <schism/sc_operations.hpp>
#include <iostream>

// TODO: Move this
const char* scGetOperationName(scOperation op) {
    switch (op) {
        // =================
        //  VM Manipulation
        // =================
        case scOperation::OpExitProgram:
            return "OpExitProgram";

        case scOperation::OpTXROut0:
            return "OpTXROut0";

        case scOperation::OpTXROut1:
            return "OpTXROut1";

        case scOperation::OpTXROut2:
            return "OpTXROut2";

        case scOperation::OpTXROut3:
            return "OpTXROut3";

        // =================
        //  Push operations
        // =================
        case scOperation::OpPushF32:
            return "OpPushF32";

        case scOperation::OpPushF64:
            return "OpPushF64";

        case scOperation::OpPushI16:
            return "OpPushI16";

        case scOperation::OpPushI32:
            return "OpPushI32";

        // =================
        //  Math Operations
        // =================
        case scOperation::OpAddF32F32:
            return "OpAddF32F32";

        case scOperation::OpSubF32F32:
            return "OpSubF32F32";

        case scOperation::OpMulF32F32:
            return "OpMulF32F32";

        case scOperation::OpDivF32F32:
            return "OpDivF32f32";
    }

    return "UNKNOWN OPERATION!";
}

void scAssembledProgram::WriteToFile(const std::string& path) {
    std::ofstream file(path, std::ofstream::binary);

    scMagicType magic = scMagicType::SC_MAGIC_MODULE;

    file.write(reinterpret_cast<char*>(&magic), sizeof(scMagicType));
    file.write(reinterpret_cast<char*>(&header), sizeof(scModuleHeader));
    file.write(reinterpret_cast<char*>(binary.data()), binary.size());

    file.close();
}

scAssembledProgram::scAssembledProgram(const std::vector<uint8_t>& binary, scModuleType type) {
    this->binary = binary;
    this->header = scModuleHeader {
        type,
        static_cast<uint32_t>(binary.size())
    };
}

scAssemblerState scAssembler::CompileSourceFile(const std::string& path, scAssembledProgram& outProgram) {
    std::ifstream file(path);
    std::string line;

    std::vector<uint8_t> program;

    while (std::getline(file, line)) {
        // TODO: Use semicolons

        for (char& ch: line)
            ch = std::toupper(ch);

        // TODO: Better check for if the line is empty
        if (line.empty())
            continue;

        if (line[0] == ';') // Comment
            continue;

        // Read the operation
        size_t nextSpace = line.find_first_of(' ');

        std::string operation = line.substr(0, nextSpace);

        bool hasArgs = nextSpace != std::string::npos;
        std::string afterOperation = "";

        if (hasArgs)
            afterOperation = line.substr(nextSpace + 1);

        std::vector<std::string> args;

        while (hasArgs) {
            args.emplace_back(afterOperation.substr(0, nextSpace));

            nextSpace = afterOperation.find_first_of(' ');

            if (nextSpace != std::string::npos) {
                afterOperation = afterOperation.substr(nextSpace + 1);
            } else {
                break;
            }
        }

        /* DEBUGGING
        std::cout << operation << std::endl;

        for (const auto& arg: args) {
            std::cout << arg << std::endl;
        }
        */

        // TODO: Replace this mess later

        // =================
        //  VM Manipulation
        // =================
        if (operation == "EXIT") {
            Emit(program, scOperation::OpExitProgram);

            continue;
        }

        if (operation == "TXR_OUT0") {
            Emit(program, scOperation::OpTXROut0);

            continue;
        }

        if (operation == "TXR_OUT1") {
            Emit(program, scOperation::OpTXROut1);

            continue;
        }

        if (operation == "TXR_OUT2") {
            Emit(program, scOperation::OpTXROut2);

            continue;
        }

        if (operation == "TXR_OUT3") {
            Emit(program, scOperation::OpTXROut3);

            continue;
        }

        if (operation == "LOAD_F32") {
            Emit(program, scOperation::OpLoadF32);

            for (const std::string& arg: args) {
                uint32_t val = 0.0F;

                // TODO: Crash
                if (!TryParseHex(arg, val))
                    return scAssemblerState::InvalidArgument;

                Emit(program, val);
            }

            continue;
        }


        // ==================
        //  Stack operations
        // ==================
        if (operation == "PUSH_F32") {
            Emit(program, scOperation::OpPushF32);

            for (const std::string& arg: args) {
                float val = 0.0F;

                // TODO: Crash
                if (!TryParseFloat(arg, val))
                    return scAssemblerState::InvalidArgument;

                Emit(program, val);
            }

            continue;
        }

        // =================
        //  Math Operations
        // =================
        if (operation == "MUL_F32_F32") {
            Emit(program, scOperation::OpMulF32F32);

            continue;
        }

        if (operation == "DIV_F32_F32") {
            Emit(program, scOperation::OpDivF32F32);

            continue;
        }

        std::cout << "[scAssembler]: Unknown instruction (" << operation << ")" << std::endl;
        return scAssemblerState::UnknownInstruction;
    }

    outProgram = scAssembledProgram(program, scModuleType::Fragment);
    return scAssemblerState::OK;
}

bool scAssembler::TryParseFloat(const std::string& str, float& out) {
    char* end;
    out = std::strtod(str.c_str(), &end);

    return end != str.c_str();
}

bool scAssembler::TryParseHex(const std::string& str, uint32_t& out) {
    char* end;
    out = std::strtoul(str.c_str(), &end, 16);

    return end != str.c_str();
}
