
## I/O Registers

All schism registers are the size of a DWORD (32-bit)

| **NAME** |      **USAGE**      |
| :------: | :-----------------: |
|    SP    |    Stack Pointer    |
|    IP    | Instruction Pointer |
|          |                     |
|   FB0    |  Framebuffer R (0)  |
|   FB1    |  Framebuffer G (1)  |
|   FB2    |  Framebuffer B (2)  |
|   FB3    |  Framebuffer A (3)  |

### [[Schism Registers]] - Refer to this for user registers


## Instructions

***ALL SCHISM INSTRUCTIONS ARE 32-BIT**

### Group Zero `0x0` Instructions

#### Info
---
These are all immediate call instructions, their purpose is entirely to operate on known data within the VM without needing any instructions

#### Encoding / Decoding
---
Since these functions take in no inputs, they are executed immediately

| **NAME** | **BINARY** |
| -------- | ---------- |
| `EXIT`   | `0b0000`   |
|          |            |
|          |            |

### Group One `0x1` Instructions

### Info
---
Since these instructions all operate on registers, decoding them is a tad complicated as a result of this unique structure

### Encoding / Decoding
---
**Note: Operations will store the result in Register A**
```
|00000000|00000000|0000|00000000|0001|
| E      | D      | C  | B      | A  |

A = Group
B = Operation
C = Sub Operation
D = A Register
E = B Register
```

|    NAME     |          ASM           |  OPERATION   |                              SUB OP                               |
| :---------: | :--------------------: | :----------: | :---------------------------------------------------------------: |
|  MOVE (R)   |      `mov %A %B`       | `0b00000000` |                             `0b0000`                              |
|             |                        |              |                                                                   |
| ALU_F32_F32 | `alu_f32_f32 OP %A %B` | `0b00000001` | [[Schism Instruction Set#ALU SUBOPERATION BLOCK\|ALU OPERATIONS]] |

##### ALU SUBOPERATION BLOCK

| NAME |  ASM  |  SUB OP  |
| :--: | :---: | :------: |
| ADD  | `ADD` | `0b0000` |
| SUB  | `SUB` | `0b0001` |
| MUL  | `MUL` | `0b0010` |
| DIV  | `DIV` | `0b0011` |
| MOD  | `MOD` | `0b0100` |
| POW  | `POW` | `0b0101` |

### Group Two `0x2` Instructions

### Info
---
These operations all operate upon registers using an immediate value after the encoded instruction

### Encoding / Decoding
---
```
|000000000000|00000000|00000000|0010|
| D          | C      | B      | A  |

A = Group (4 Bits)
B = Operation (8 Bits)
C = IN Register (8 Bits)
D = RESERVED (12 Bits)
```

|    NAME     |          ASM           |  OPERATION   |
| :---------: | :--------------------: | :----------: |
| SET_F32_RX  | `set_f32 %REG IMM_VAL` | `0b00000000` |
| LOAD_F32_RX | `ld_f32 %REG IMM_PTR`  | `0b00000001` |
|   ABS_F32   |     `abs_f32 %REG`     | `0b00000010` |

### Group Two `0x3` Instructions

### Info
---
These operations all operate in SIMD mode, taking only V0-V7 registers

### Encoding / Decoding
---
``

|    NAME     |          ASM           |  OPERATION   |
| :---------: | :--------------------: | :----------: |
| SET_F32_RX  | `set_f32 %REG IMM_VAL` | `0b00000000` |
| LOAD_F32_RX | `ld_f32 %REG IMM_PTR`  | `0b00000001` |
|   ABS_F32   |     `abs_f32 %REG`     | `0b00000010` |