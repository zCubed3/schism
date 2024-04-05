#include "sc_vm.hpp"

#include <iostream>

#define ENUM_DEBUG_REGISTER_NAME(VAL) \
    case scRegister::##VAL##:         \
        return #VAL;

const char* scGetRegisterName(scRegister regIndex) {
    switch (regIndex) {
        ENUM_DEBUG_REGISTER_NAME(SP)
        ENUM_DEBUG_REGISTER_NAME(IP)

        ENUM_DEBUG_REGISTER_NAME(FB0)
        ENUM_DEBUG_REGISTER_NAME(FB1)
        ENUM_DEBUG_REGISTER_NAME(FB2)
        ENUM_DEBUG_REGISTER_NAME(FB3)

        ENUM_DEBUG_REGISTER_NAME(S0)
        ENUM_DEBUG_REGISTER_NAME(S1)
        ENUM_DEBUG_REGISTER_NAME(S2)
        ENUM_DEBUG_REGISTER_NAME(S3)
        ENUM_DEBUG_REGISTER_NAME(S4)
        ENUM_DEBUG_REGISTER_NAME(S5)
        ENUM_DEBUG_REGISTER_NAME(S6)
        ENUM_DEBUG_REGISTER_NAME(S7)
        ENUM_DEBUG_REGISTER_NAME(S8)
        ENUM_DEBUG_REGISTER_NAME(S9)
        ENUM_DEBUG_REGISTER_NAME(S10)
        ENUM_DEBUG_REGISTER_NAME(S11)
        ENUM_DEBUG_REGISTER_NAME(S12)
        ENUM_DEBUG_REGISTER_NAME(S13)
        ENUM_DEBUG_REGISTER_NAME(S14)
        ENUM_DEBUG_REGISTER_NAME(S15)
        ENUM_DEBUG_REGISTER_NAME(S16)
        ENUM_DEBUG_REGISTER_NAME(S17)
        ENUM_DEBUG_REGISTER_NAME(S18)
        ENUM_DEBUG_REGISTER_NAME(S19)
        ENUM_DEBUG_REGISTER_NAME(S20)
        ENUM_DEBUG_REGISTER_NAME(S21)
        ENUM_DEBUG_REGISTER_NAME(S22)
        ENUM_DEBUG_REGISTER_NAME(S23)
        ENUM_DEBUG_REGISTER_NAME(S24)
        ENUM_DEBUG_REGISTER_NAME(S25)
        ENUM_DEBUG_REGISTER_NAME(S26)
        ENUM_DEBUG_REGISTER_NAME(S27)
        ENUM_DEBUG_REGISTER_NAME(S28)
        ENUM_DEBUG_REGISTER_NAME(S29)
        ENUM_DEBUG_REGISTER_NAME(S30)
        ENUM_DEBUG_REGISTER_NAME(S31)
    }

    return nullptr;
}

// ===============
//  Ctor and Dtor
// ===============
scVM::scVM(size_t memSize) {
    this->_memory.resize(memSize);
}

// ======================
//  Program Manipulation
// ======================
void scVM::LoadProgram(const scModule& module) {
    ResetRegisters();
    _program = module;
}

// =======================
//  Register Manipulation
// =======================
void scVM::MoveInstructionPointer(int offset) {
    _registers[static_cast<int>(scRegister::IP)].u32 += offset;
}

void scVM::SetRegister(scRegister regIndex, const scValue_u& value) {
    _registers[static_cast<int>(regIndex)] = value;
}

// =====================
//  Memory Manipulation
// =====================

// .....

// ===========
//  Debugging
// ===========
void scVM::PrintVariable(const scVariable& variable) {
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

void scVM::PrintStack() {
    std::cout << "-======= SCHISM STACK =======-\n";

    for (int s = GetRegister(scRegister::SP).u32 - 1; s >= 0; s--) {
        scVariable variable = _stack[s];

        std::cout << s << " : ";
        PrintVariable(variable);
        std::cout << std::endl;
    }

    std::cout << "-============================-\n";
}

void scVM::PrintRegisters() {
    std::cout << "-======= SCHISM REGISTERS =======-\n";

    std::cout << std::hex;

    for (int r = 0; r < (int)scRegister::REGISTER_COUNT; r++) {
        scRegister reg = (scRegister)r;
        std::cout << scGetRegisterName(reg) << GetRegister(reg).u32 << std::endl;
    }

    std::cout << std::dec;

    std::cout << "-================================-\n";
}

// ====================
//  Stack Manipulation
// ====================
void scVM::PushValue(scValue_u value, scValueType type) {
    scValue_u sp = GetRegister(scRegister::SP);

    _stack[sp.u32++] = { value, type };

    SetRegister(scRegister::SP, sp);

    // TODO: Detect a stack overflow
}

bool scVM::PopValue(scVariable& operand) {
    scValue_u sp = GetRegister(scRegister::SP);

    // TODO: Detect stack underflow
    if (sp.u32 - 1 == 0xFF)
        return false;

    operand = _stack[--sp.u32];
    SetRegister(scRegister::SP, sp);

    return true;
}

// ===================
//  Program Execution
// ===================
bool scVM::ExecuteOperation(const scModule& module, uint32_t encoded) {
    //std::cout << "EXECUTING: " << scGetOperationName(op) << "\n";

    // Begin to decode the instruction
    scInstructionGroup group = (scInstructionGroup)(encoded & 0xF);

    switch (group) {
        case scInstructionGroup::GroupZero: {
            scGroupZeroOperations op = (scGroupZeroOperations)((encoded >> 4) & 0xFF);
            switch (op) {
                case scGroupZeroOperations::OpExitProgram:
                    return false;
            }
        }

        case scInstructionGroup::GroupOne: {
            scGroupOneOperations op = (scGroupOneOperations)((encoded >> 4) & 0xFF);
            scGroupOneSubOperations subOp = (scGroupOneSubOperations)((encoded >> 12) & 0xF);

            scRegister aRegister = (scRegister)((encoded >> 16) & 0xFF);
            scRegister bRegister = (scRegister)((encoded >> 24) & 0xFF);

            switch (op) {
                case scGroupOneOperations::OpMOV: {
                    SetRegister(aRegister, GetRegister(bRegister));
                    break;
                }

                case scGroupOneOperations::OpALUF32F32: {
                    int simd = 1;

                    // TODO: Add a helper rather than this bs
                    if (aRegister == scRegister::V0) {
                        simd = 4;
                        aRegister = scRegister::S0;
                    }

                    if (bRegister == scRegister::V1) {
                        simd = 4;
                        bRegister = scRegister::S4;
                    }


                    for (int d = 0; d < simd; d++) {
                        scValue_u aValue = GetRegister((scRegister)((int)aRegister + d));
                        scValue_u bValue = GetRegister((scRegister)((int)bRegister + d));

                        //std::cout << "LHS | " << (int)aRegister + d << " | " << aValue.f32 << std::endl;
                        //std::cout << "RHS | " << (int)bRegister + d << " | " << bValue.f32 << std::endl;

                        switch (subOp) {
                            case scGroupOneSubOperations::SubOpAdd: {
                                aValue.f32 = aValue.f32 + bValue.f32;
                                break;
                            }

                            case scGroupOneSubOperations::SubOpSub: {
                                aValue.f32 = aValue.f32 - bValue.f32;
                                break;
                            }

                            case scGroupOneSubOperations::SubOpMul: {
                                aValue.f32 = aValue.f32 * bValue.f32;
                                break;
                            }

                            case scGroupOneSubOperations::SubOpDiv: {
                                aValue.f32 = aValue.f32 / bValue.f32;
                                break;
                            }

                            case scGroupOneSubOperations::SubOpMod: {
                                aValue.f32 = std::fmod(aValue.f32, bValue.f32);
                                break;
                            }

                            case scGroupOneSubOperations::SubOpPow: {
                                aValue.f32 = powf(aValue.f32, bValue.f32);
                                break;
                            }
                        }

                        SetRegister((scRegister)((int)aRegister + d), aValue);
                    }

                    break;
                }
            }

            break;
        }

        case scInstructionGroup::GroupTwo: {
            scGroupTwoOperations op = (scGroupTwoOperations)((encoded >> 4) & 0xFF);
            scRegister targetRegister = (scRegister)((encoded >> 12) & 0xFF);

            // TODO: Break this into functions
            switch (op) {
                case scGroupTwoOperations::OpSetF32: {
                    scValue_u value;
                    module.ReadValue(GetRegister(scRegister::IP).u32, value.f32);

                    MoveInstructionPointer(sizeof(float));

                    SetRegister(targetRegister, value);
                    break;
                }

                case scGroupTwoOperations::OpLoadF32: {
                    uint32_t ptr;
                    module.ReadValue(GetRegister(scRegister::IP).u32, ptr);

                    MoveInstructionPointer(sizeof(uint32_t));

                    scValue_u value;

                    if (!ReadValue(ptr, value.f32)) {
                        return false;
                    }

                    SetRegister(targetRegister, value);
                    break;
                }

                case scGroupTwoOperations::OpABSF32: {
                    scValue_u value = GetRegister(targetRegister);

                    value.f32 = fabs(value.f32);

                    SetRegister(targetRegister, value);
                }
            }

            break;
        }
    }

    //PrintStack();

    return true;
}

void scVM::ResetRegisters() {
    for (int r = 0; r < (int)scRegister::REGISTER_COUNT; r++) {
        _registers[r].u32 = 0;
    }
}

void scVM::ExecuteTillEnd() {
    bool alive = true;

    while (ExecuteStep()) {

    }
}

bool scVM::ExecuteStep() {
    // TODO: Report the VM CRASH!
    uint32_t encoded;
    if (_program->ReadValue<uint32_t>(GetRegister(scRegister::IP).u32, encoded) != scModuleState::OK)
        return false;

    MoveInstructionPointer(sizeof(uint32_t));

    return ExecuteOperation(_program.value(), encoded);
}