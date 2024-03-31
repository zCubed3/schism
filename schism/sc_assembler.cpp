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
            nextSpace = afterOperation.find_first_of(' ');

            if (nextSpace != std::string::npos) {
                args.push_back(afterOperation.substr(0, nextSpace));
                afterOperation = afterOperation.substr(nextSpace + 1);
            } else {
                break;
            }
        }

        // Check if there is still string remaining
        if (hasArgs && !afterOperation.empty())
            args.push_back(afterOperation);

        scAssemblerState state = scAssemblerState::UnknownInstruction;

        state = AssembleGroupZero(program, operation, args);

        if (state == scAssemblerState::UnknownInstruction) {
            std::cout << "[scAssembler]: Unknown group zero instruction (" << operation << ")" << std::endl;
            return state;
        }

        state = AssembleGroupOne(program, operation, args);

        if (state == scAssemblerState::UnknownInstruction) {
            std::cout << "[scAssembler]: Unknown group one instruction (" << operation << ")" << std::endl;
            return state;
        }

        state = AssembleGroupTwo(program, operation, args);

        if (state == scAssemblerState::UnknownInstruction) {
            std::cout << "[scAssembler]: Unknown group two instruction (" << operation << ")" << std::endl;
            return state;
        }
    }

    outProgram = scAssembledProgram(program, scModuleType::Fragment);
    return scAssemblerState::OK;
}

uint8_t scAssembler::DecodeRegister(const std::string& name) {
    // TODO: Split the text off and just parse the number?
    if (name == "%V0") {
        return 2;
    }

    if (name == "%V1") {
        return 3;
    }

    if (name == "%V2") {
        return 4;
    }

    if (name == "%V3") {
        return 5;
    }

    if (name == "%V4") {
        return 6;
    }

    if (name == "%V5") {
        return 7;
    }

    if (name == "%V6") {
        return 8;
    }

    if (name == "%V7") {
        return 9;
    }

    if (name == "%FB0") {
        return 10;
    }

    if (name == "%FB1") {
        return 11;
    }

    if (name == "%FB2") {
        return 12;
    }

    if (name == "%FB3") {
        return 13;
    }

    return -1;
}

scAssemblerState scAssembler::AssembleGroupZero(std::vector<uint8_t>& program,
                                                const std::string& op,
                                                const std::vector<std::string>& args) {
    uint32_t encoded = 0x0000;

    SetGroup(scInstructionGroup::GroupZero, encoded);

    return scAssemblerState::NoInstructionFound;
}

scAssemblerState scAssembler::AssembleGroupOne(std::vector<uint8_t>& program,
                                                const std::string& op,
                                                const std::vector<std::string>& args) {
    uint32_t encoded = 0x0000;

    SetGroup(scInstructionGroup::GroupOne, encoded);

    // TODO: Refactor this
    if (op == "ALU_F32_F32") {
        scGroupOneSubOperations subOp;
        SetInstruction(scGroupOneOperations::OpALUF32F32, encoded);

        // Parse the subop
        if (args[0] == "ADD") {
            subOp = scGroupOneSubOperations::SubOpAdd;
        } else if (args[0] == "SUB") {
            subOp = scGroupOneSubOperations::SubOpSub;
        } else if (args[0] == "MUL") {
            subOp = scGroupOneSubOperations::SubOpMul;
        } else if (args[0] == "DIV") {
            subOp = scGroupOneSubOperations::SubOpDiv;
        } else if (args[0] == "MOD") {
            subOp = scGroupOneSubOperations::SubOpMod;
        } else if (args[0] == "POW") {
            subOp = scGroupOneSubOperations::SubOpPow;
        } else {
            return scAssemblerState::InvalidArgument;
        }

        uint8_t aRegister = DecodeRegister(args[1]);
        uint8_t bRegister = DecodeRegister(args[2]);

        for (int b = 0; b < 4; b++) {
            SetBit(encoded, 12 + b, ((int)subOp) & (1 << b));
        }

        for (int b = 0; b < 8; b++) {
            SetBit(encoded, 16 + b, aRegister & (1 << b));
            SetBit(encoded, 24 + b, bRegister & (1 << b));
        }

        Emit(program, encoded);
        return scAssemblerState::OK;
    }

    return scAssemblerState::NoInstructionFound;
}

scAssemblerState scAssembler::AssembleGroupTwo(std::vector<uint8_t>& program,
                                               const std::string& op,
                                                const std::vector<std::string>& args) {
    uint32_t encoded = 0x0000;

    SetGroup(scInstructionGroup::GroupTwo, encoded);

    uint8_t targetRegister = DecodeRegister(args[0]);

    if (op == "SET_F32") {
        SetInstruction(scGroupTwoOperations::OpSetF32, encoded);

        for (int b = 0; b < 8; b++) {
            SetBit(encoded, 12 + b, targetRegister & (1 << b));
        }

        Emit(program, encoded);

        float arg = 0;

        if (!TryParseFloat(args[1], arg))
            return scAssemblerState::InvalidArgument;

        Emit(program, arg);

        return scAssemblerState::OK;
    }

    if (op == "LD_F32") {
        SetInstruction(scGroupTwoOperations::OpLoadF32, encoded);

        for (int b = 0; b < 8; b++) {
            SetBit(encoded, 12 + b, targetRegister & (1 << b));
        }

        Emit(program, encoded);

        uint32_t arg = 0;

        if (!TryParseHex(args[1], arg))
            return scAssemblerState::InvalidArgument;

        Emit(program, arg);

        return scAssemblerState::OK;
    }

    if (op == "LD_F32") {
        SetInstruction(scGroupTwoOperations::OpLoadF32, encoded);

        for (int b = 0; b < 8; b++) {
            SetBit(encoded, 12 + b, targetRegister & (1 << b));
        }

        Emit(program, encoded);

        uint32_t arg = 0;

        if (!TryParseHex(args[1], arg))
            return scAssemblerState::InvalidArgument;

        Emit(program, arg);

        return scAssemblerState::OK;
    }

    return scAssemblerState::NoInstructionFound;
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
