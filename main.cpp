//====================================================================================
// BSD 3-Clause License
//
// Copyright (c) 2024, Liam Reese (zCubed)
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

enum class scModuleType : uint16_t {
    SC_MODULE_VERT = 0x0000,
    SC_MODULE_FRAG = 0x0001
};

enum class scMagicType : uint32_t {
    SC_MAGIC_MODULE = 0x4D534353
};

#pragma pack(1)
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
    bool LoadFromFile(const std::string& path) {
        std::ifstream file(path, std::ifstream::binary);

        scMagicType magic;
        file.read(reinterpret_cast<char*>(&magic), sizeof(scMagicType));

        if (magic != scMagicType::SC_MAGIC_MODULE)
            return false;

        scModuleHeader_t header {};
        file.read(reinterpret_cast<char*>(&header), sizeof(header));

        _code.resize(header.len);
        file.read(reinterpret_cast<char*>(_code.data()), header.len);

        return true;
    }

    template<typename T>
    bool GetValue(uint32_t cur, T& outValue) const {
        if (cur + (sizeof(T) - 1) >= _code.size())
            return false;

        outValue = *((T*)(_code.data() + cur));
        return true;
    }
};

const size_t OPERATION_SIZE = 2;

enum class scOperation : uint16_t {
    // =================
    //  VM Manipulation
    // =================
    SC_OP_EXIT_PROGRAM  = 0x0000,

    SC_OP_STORE_F32     = 0x00A0,
    SC_OP_STORE_F64     = 0x00A1,
    SC_OP_STORE_I16     = 0x00A2,
    SC_OP_STORE_I32     = 0x00A3,

    SC_OP_LOAD_F32      = 0x00C0,
    SC_OP_LOAD_F64      = 0x00C1,
    SC_OP_LOAD_I16      = 0x00C2,
    SC_OP_LOAD_I32      = 0x00C3,

    // ==================
    //  Stack operations
    // ==================
    SC_OP_PUSH_F32      = 0x0100,
    SC_OP_PUSH_F64      = 0x0101,
    SC_OP_PUSH_I16      = 0x0102,
    SC_OP_PUSH_I32      = 0x0103,

    // =================
    //  Math Operations
    // =================
    SC_OP_ADD_F32_F32   = 0x0200,
    SC_OP_SUB_F32_F32   = 0x0201,
    SC_OP_MUL_F32_F32   = 0x0202,
    SC_OP_DIV_F32_F32   = 0x0203,

    SC_OP_ADD_F64_F64   = 0x0210,
    SC_OP_SUB_F64_F64   = 0x0211,
    SC_OP_MUL_F64_F64   = 0x0212,
    SC_OP_DIV_F64_F64   = 0x0213,

    SC_OP_ADD_I16_I16   = 0x0220,
    SC_OP_SUB_I16_I16   = 0x0221,
    SC_OP_MUL_I16_I16   = 0x0222,
    SC_OP_DIV_I16_I16   = 0x0223,

    SC_OP_ADD_I32_I32   = 0x0230,
    SC_OP_SUB_I32_I32   = 0x0231,
    SC_OP_MUL_I32_I32   = 0x0232,
    SC_OP_DIV_I32_I32   = 0x0233,
};

typedef union scValue {
    int16_t i16;
    int32_t i32;
    float   f32;
} scValue_u;

enum class scValueType : uint16_t {
    SC_TYPE_F32,
    SC_TYPE_F64,

    SC_TYPE_I8,
    SC_TYPE_I16,
    SC_TYPE_I32,
};

struct scOperand {
public:
    scValue_u value;
    scValueType type;
};

// Represents the virtual machine that handles state for a provided scModule
template<size_t MEM_SIZE>
class scVM {
protected:
    std::array<scOperand, 256> _stack {};
    std::array<uint8_t, MEM_SIZE> _backing {};

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
    // ===========
    //  Debugging
    // ===========
    const char* GetOperationName(const scOperation& op) {
        switch (op) {
            // =================
            //  VM Manipulation
            // =================
            case scOperation::SC_OP_EXIT_PROGRAM:
                return "SC_OP_EXIT_PROGRAM";

            // =================
            //  Push operations
            // =================
            case scOperation::SC_OP_PUSH_F32:
                return "SC_OP_PUSH_F32";

            case scOperation::SC_OP_PUSH_F64:
                return "SC_OP_PUSH_F64";

            case scOperation::SC_OP_PUSH_I16:
                return "SC_OP_PUSH_I16";

            case scOperation::SC_OP_PUSH_I32:
                return "SC_OP_PUSH_I32";

            // =================
            //  Math Operations
            // =================
            case scOperation::SC_OP_ADD_F32_F32:
                return "SC_OP_ADD_F32_F32";

            case scOperation::SC_OP_SUB_F32_F32:
                return "SC_OP_SUB_F32_F32";

            case scOperation::SC_OP_MUL_F32_F32:
                return "SC_OP_MUL_F32_F32";

            case scOperation::SC_OP_DIV_F32_F32:
                return "SC_OP_DIV_F32_F32";
        }

        return "UNKNOWN OPERATION!";
    }

    void PrintOperand(const scOperand& operand) {
        std::cout << std::hex;

        switch (operand.type) {
            case scValueType::SC_TYPE_F32:
                std::cout << "(F32) = " << std::dec << operand.value.f32;
                break;

            //case scValueType::SC_TYPE_F64:
            //    std::cout << "(F64) = 0x" << operand.value.f64;
            //    break;

            case scValueType::SC_TYPE_I16:
                std::cout << "(I16) = 0x" << operand.value.i16;
                break;

            case scValueType::SC_TYPE_I32:
                std::cout << "(I32) = 0x" << operand.value.i32;
                break;
        }

        std::cout << std::dec;
    }

    void PrintStack() {
        std::cout << "-======= SCHISM STACK =======-\n";

        for (int s = _regSP - 1; s >= 0; s--) {
            scOperand operand = _stack[s];

            std::cout << s << " : ";
            PrintOperand(operand);
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

    bool PopValue(scOperand& operand) {
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
        std::cout << "EXECUTING: " << GetOperationName(op) << std::endl;

        switch (op) {
            // =================
            //  VM Manipulation
            // =================
            case scOperation::SC_OP_EXIT_PROGRAM: {
                return false;
            }

            // =================
            //  Push operations
            // =================
            case scOperation::SC_OP_PUSH_F32: {
                scValue_u value;

                if (!module.GetValue<float>(_regIP, value.f32))
                    return false;

                _regIP += 4;

                PushValue(value, scValueType::SC_TYPE_F32);

                break;
            }

            case scOperation::SC_OP_PUSH_I16: {
                scValue_u value;

                if (!module.GetValue<float>(_regIP, value.f32))
                    return false;

                _regIP += 2;

                PushValue(value, scValueType::SC_TYPE_I16);

                break;
            }

            // =================
            //  Math Operations
            // =================
            case scOperation::SC_OP_MUL_F32_F32: {
                scOperand inLhs, inRhs;

                if (!PopValue(inLhs))
                    return false;

                if (!PopValue(inRhs))
                    return false;

                // TODO: Type check

                scValue_u value;
                value.f32 = inLhs.value.f32 * inRhs.value.f32;

                PushValue(value, scValueType::SC_TYPE_F32);

                break;
            }
        }

        PrintStack();

        return true;
    }

    void ExecuteModule(const scModule& module) {
        bool alive = true;

        while (alive) {
            scOperation op;

            // TODO: VM CRASH!
            if (!module.GetValue<scOperation>(_regIP, op))
                return;

            _regIP += OPERATION_SIZE;

            alive = ExecuteOperation(module, op);
        }
    }
};

class scAssembledProgram {
public:
    scModuleHeader header;
    std::vector<uint8_t> binary;

    scAssembledProgram(const std::vector<uint8_t>& binary, scModuleType type) {
        this->binary = binary;
        this->header = scModuleHeader {
            type,
            static_cast<uint32_t>(binary.size())
        };
    }

    void WriteToFile(const std::string& path) {
        std::ofstream file(path, std::ofstream::binary);

        scMagicType magic = scMagicType::SC_MAGIC_MODULE;

        file.write(reinterpret_cast<char*>(&magic), sizeof(scMagicType));
        file.write(reinterpret_cast<char*>(&header), sizeof(scModuleHeader));
        file.write(reinterpret_cast<char*>(binary.data()), binary.size());

        file.close();
    }
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

    bool TryParseFloat(const std::string& str, float& out) {
        char* end;
        out = std::strtod(str.c_str(), &end);

        return end != str.c_str();
    }

    std::optional<scAssembledProgram> CompileSourceFile(const std::string& path) {
        std::ifstream file(path);
        std::string line;

        std::vector<uint8_t> program;

        while (std::getline(file, line)) {
            // TODO: Use semicolons

            for (char& ch: line)
                ch = std::toupper(ch);

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

            std::cout << operation << std::endl;

            for (const auto& arg: args) {
                std::cout << arg << std::endl;
            }

            // Then parse our sections

            // TODO: Replace this later
            if (operation == "PUSH_F32"){
                Emit(program, scOperation::SC_OP_PUSH_F32);

                for (const std::string& arg : args) {
                    float val = 0.0F;

                    // TODO: Crash
                    if (!TryParseFloat(arg, val))
                        return {};

                    Emit(program, val);
                }
            }

            if (operation == "EXIT") {
                Emit(program, scOperation::SC_OP_EXIT_PROGRAM);
            }

            if (operation == "MUL_F32_F32") {
                Emit(program, scOperation::SC_OP_MUL_F32_F32);
            }
        }

        // TODO: Detect module type
        return scAssembledProgram(program, scModuleType::SC_MODULE_VERT);
    }
};

int main() {
    scAssembler assembler;

    std::optional<scAssembledProgram> compiled = assembler.CompileSourceFile("test.scsa");

    if (!compiled) {
        return 1;
    }

    compiled.value().WriteToFile("test.scsm");

    scModule module;

    module.LoadFromFile("test.scsm");

    scVM<512> vm;

    vm.ExecuteModule(module);
    vm.PrintStack();
    vm.PrintRegisters();
}
