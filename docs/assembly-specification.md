# 6502 Assembly Specification

## Values
Syntax examples for different value formats.

| Value Type  | Format                     | Note             |
| :---:       | :---:                      | :---             |
| Hexadecimal | $Ab12                      | Case insensitive.|
| Binary      | %01000101                  |                  |
| Decimal     | 1234                       |                  |
| Symbol Addr | symbol_name                |                  |
| String      | "Any text between quotes." | No escape chars. |

> [!WARNING]
> **Zero-Page Optimization:** Currently, if a numeric address evaluates to `<= $FF`, `a65` automatically optimizes the instruction to use Zero-Page addressing (e.g., `LDA $0012` is compiled as `LDA $12`).
>
> There is currently no mechanism to override this behavior and force 16-bit Absolute addressing for low-byte values. Support for the `>` (force absolute) and `<` (force zero-page) unary operators will be added in a future release.

## Directives
The behavior and argument format of every directive.

| Directive | Syntax                 | Behavior                                                               |
| :---:     | :---:                  | :--                                                                    |
| org       | .org $HHLL             | Set the VPC to the value.                                              |
| res       | .res $HHLL             | Increment the VPC by the value.                                        |
| byte      | .byte $LL,$LL,         | Insert listed bytes into memory, incrementing VPC proportionally.      |
| word      | .word $HHLL,$HHLL,     | Insert listed words into memory in **little-endian** format, incrementing VPC proportionally.      |
| equ       | .equ symbol_name $HHLL | Initialize a value in the symbol table. Redefinitions are forbidden.   |
| fill      | .fill $HHLL $LL        | Fill the next X bytes with the specified value. X = $HHLL, value = $LL.|
| ascii     | .ascii "string"        | Insert ascii bytes into memory, incrementing VPC proportionally.       |
| align     | .align value           | Set the VPC to the next multiple of value. Value must be a power of 2. |

## Addressing Modes
The expected syntax and notes for each addressing mode. The hex value formats
 shown in the table are examples, you can use any number value format or
 symbol name as defined in [Values](#Values).

| MODE        | Syntax      | Note                   |
| :---:       | :---:       | :---                   |
| **A**       | INS A       |                        |
| **ABS**     | INS $HHLL   |                        |
| **ABSX**    | INS $HHLL,X | X is case insensitive. |
| **ABSY**    | INS $HHLL,Y | Y is case insensitive. |
| **IMM**     | INS #$LL    |                        |
| **IMPLIED** | INS         |                        |
| **IND**     | INS ($HHLL) |                        |
| **INDX**    | INS ($LL,X) | X is case insensitive. |
| **INDY**    | INS ($LL),Y | Y is case insensitive. |
| **REL**     | INS $BB     | $BB is a signed byte. Valid for any value that resolves to -128/+127 of PC+2. |
| **ZPG**     | INS $LL     |                        |
| **ZPGX**    | INS $LL,X   | X is case insensitive. |
| **ZPGY**    | INS $LL,Y   | Y is case insensitive. |

## Instructions

| INS     | A    | ABS  | ABSX | ABSY | IMM  | IMPL | IND  | INDX | INDY | REL  | ZPG  | ZPGX | ZPGY |
| :---:   | :---:| :---:| :---:| :---:| :---:| :---:| :---:| :---:| :---:| :---:| :---:| :---:| :---:|
| **ADC** | --   | 6D   | 7D   | 79   | 69   | --   | --   | 61   | 71   | --   | 65   | 75   | --   |
| **AND** | --   | 2D   | 3D   | 39   | 29   | --   | --   | 21   | 31   | --   | 25   | 35   | --   |
| **ASL** | 0A   | 0E   | 1E   | --   | --   | --   | --   | --   | --   | --   | 06   | 16   | --   |
| **BCC** | --   | --   | --   | --   | --   | --   | --   | --   | --   | 90   | --   | --   | --   |
| **BCS** | --   | --   | --   | --   | --   | --   | --   | --   | --   | B0   | --   | --   | --   |
| **BEQ** | --   | --   | --   | --   | --   | --   | --   | --   | --   | F0   | --   | --   | --   |
| **BIT** | --   | 2C   | --   | --   | --   | --   | --   | --   | --   | --   | 24   | --   | --   |
| **BMI** | --   | --   | --   | --   | --   | --   | --   | --   | --   | 30   | --   | --   | --   |
| **BNE** | --   | --   | --   | --   | --   | --   | --   | --   | --   | D0   | --   | --   | --   |
| **BPL** | --   | --   | --   | --   | --   | --   | --   | --   | --   | 10   | --   | --   | --   |
| **BRK** | --   | --   | --   | --   | --   | 00   | --   | --   | --   | --   | --   | --   | --   |
| **BVC** | --   | --   | --   | --   | --   | --   | --   | --   | --   | 50   | --   | --   | --   |
| **BVS** | --   | --   | --   | --   | --   | --   | --   | --   | --   | 70   | --   | --   | --   |
| **CLC** | --   | --   | --   | --   | --   | 18   | --   | --   | --   | --   | --   | --   | --   |
| **CLD** | --   | --   | --   | --   | --   | D8   | --   | --   | --   | --   | --   | --   | --   |
| **CLI** | --   | --   | --   | --   | --   | 58   | --   | --   | --   | --   | --   | --   | --   |
| **CLV** | --   | --   | --   | --   | --   | B8   | --   | --   | --   | --   | --   | --   | --   |
| **CMP** | --   | CD   | DD   | D9   | C9   | --   | --   | C1   | D1   | --   | C5   | D5   | --   |
| **CPX** | --   | EC   | --   | --   | E0   | --   | --   | --   | --   | --   | E4   | --   | --   |
| **CPY** | --   | CC   | --   | --   | C0   | --   | --   | --   | --   | --   | C4   | --   | --   |
| **DEC** | --   | CE   | DE   | --   | --   | --   | --   | --   | --   | --   | C6   | D6   | --   |
| **DEX** | --   | --   | --   | --   | --   | CA   | --   | --   | --   | --   | --   | --   | --   |
| **DEY** | --   | --   | --   | --   | --   | 88   | --   | --   | --   | --   | --   | --   | --   |
| **EOR** | --   | 4D   | 5D   | 59   | 49   | --   | --   | 41   | 51   | --   | 45   | 55   | --   |
| **INC** | --   | EE   | FE   | --   | --   | --   | --   | --   | --   | --   | E6   | F6   | --   |
| **INX** | --   | --   | --   | --   | --   | E8   | --   | --   | --   | --   | --   | --   | --   |
| **INY** | --   | --   | --   | --   | --   | C8   | --   | --   | --   | --   | --   | --   | --   |
| **JMP** | --   | 4C   | --   | --   | --   | --   | 6C   | --   | --   | --   | --   | --   | --   |
| **JSR** | --   | 20   | --   | --   | --   | --   | --   | --   | --   | --   | --   | --   | --   |
| **LDA** | --   | AD   | BD   | B9   | A9   | --   | --   | A1   | B1   | --   | A5   | B5   | --   |
| **LDX** | --   | AE   | --   | BE   | A2   | --   | --   | --   | --   | --   | A6   | --   | B6   |
| **LDY** | --   | AC   | BC   | --   | A0   | --   | --   | --   | --   | --   | A4   | B4   | --   |
| **LSR** | 4A   | 4E   | 5E   | --   | --   | --   | --   | --   | --   | --   | 46   | 56   | --   |
| **NOP** | --   | --   | --   | --   | --   | EA   | --   | --   | --   | --   | --   | --   | --   |
| **ORA** | --   | 0D   | 1D   | 19   | 09   | --   | --   | 01   | 11   | --   | 05   | 15   | --   |
| **PHA** | --   | --   | --   | --   | --   | 48   | --   | --   | --   | --   | --   | --   | --   |
| **PHP** | --   | --   | --   | --   | --   | 08   | --   | --   | --   | --   | --   | --   | --   |
| **PLA** | --   | --   | --   | --   | --   | 68   | --   | --   | --   | --   | --   | --   | --   |
| **PLP** | --   | --   | --   | --   | --   | 28   | --   | --   | --   | --   | --   | --   | --   |
| **ROL** | 2A   | 2E   | 3E   | --   | --   | --   | --   | --   | --   | --   | 26   | 36   | --   |
| **ROR** | 6A   | 6E   | 7E   | --   | --   | --   | --   | --   | --   | --   | 66   | 76   | --   |
| **RTI** | --   | --   | --   | --   | --   | 40   | --   | --   | --   | --   | --   | --   | --   |
| **RTS** | --   | --   | --   | --   | --   | 60   | --   | --   | --   | --   | --   | --   | --   |
| **SBC** | --   | ED   | FD   | F9   | E9   | --   | --   | E1   | F1   | --   | E5   | F5   | --   |
| **SEC** | --   | --   | --   | --   | --   | 38   | --   | --   | --   | --   | --   | --   | --   |
| **SED** | --   | --   | --   | --   | --   | F8   | --   | --   | --   | --   | --   | --   | --   |
| **SEI** | --   | --   | --   | --   | --   | 78   | --   | --   | --   | --   | --   | --   | --   |
| **STA** | --   | 8D   | 9D   | 99   | --   | --   | --   | 81   | 91   | --   | 85   | 95   | --   |
| **STX** | --   | 8E   | --   | --   | --   | --   | --   | --   | --   | --   | 86   | --   | 96   |
| **STY** | --   | 8C   | --   | --   | --   | --   | --   | --   | --   | --   | 84   | 94   | --   |
| **TAX** | --   | --   | --   | --   | --   | AA   | --   | --   | --   | --   | --   | --   | --   |
| **TAY** | --   | --   | --   | --   | --   | A8   | --   | --   | --   | --   | --   | --   | --   |
| **TSX** | --   | --   | --   | --   | --   | BA   | --   | --   | --   | --   | --   | --   | --   |
| **TXA** | --   | --   | --   | --   | --   | 8A   | --   | --   | --   | --   | --   | --   | --   |
| **TXS** | --   | --   | --   | --   | --   | 9A   | --   | --   | --   | --   | --   | --   | --   |
| **TYA** | --   | --   | --   | --   | --   | 98   | --   | --   | --   | --   | --   | --   | --   |

