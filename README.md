#  Регистры

- EAX (32 bit) – General-Purpose
- EBX (32 bit) – General-Purpose
- ECX (32 bit) – General-Purpose
- EDX (32 bit) – General-Purpose
- EIP (32 bit) – Instruction Pointer

# Флаги

- ZF (1 bit) – Zero Flag
- SF (1 bit) – Sign Flag

# Инструкции

| Command  | Op. 1 (Dest.)   | Op. 2 (Source)  | Description                                               |
|----------|-----------------|-----------------|-----------------------------------------------------------|
| MOV_RR   | Register        | Register        | Copy value of `Op2` register to `Op1` register            |
| MOV_RM   | Register        | Address literal | Copy value at `Op2` memory address to `Op1` register      |
| MOV_MR   | Address literal | Register        | Copy value of `Op2` register to `Op1` memory address      |
| MOV_RL   | Register        | Literal         | Copy `Op2` literal to `Op1` register                      |
| MOV_TL   | Address register| Literal         | Copy `Op2` literal to address at `Op1` register           |
| MOV_RT   | Register        | Address register| Copy value of address at `Op2` register to `Op1` register |
| CMP_RR   | Register        | Register        | Compare `Op2` register with `Op1` register                |
| ADD_RR   | Register        | Register        | Adds value of `Op2` register to `Op1` register            |
| ADD_RL   | Register        | Literal         | Adds `Op2` literal to `Op1` register                      |
| SUB_RR   | Register        | Register        | Subtracts value of `Op2` register from `Op1` register     |
| SUB_RL   | Register        | Literal         | Subtracts `Op2` literal from `Op1` register               |
| JZ_R     | Offset          | None            | Relative jump (same as add or sub with op1 = eip)         |
| TEST_RR  | Register        | Register        | Bitwise AND, only flags affected, result discarded        |
|CMOV_GT_RR| Register        | Register        | Conditional move (Greater)                                |
| HALT     | None            | None            | Print VM state and exit                                   |

# Структура команд

```
XXXX XXXX YYYY YYYY ZZZZ ZZZZ LLLL LLLL

XXXX XXXX  – код команды
YYYY YYYY – первый операнд (при наличии) 
ZZZZ ZZZZ – второй операнд (при наличии)
ZZZZ ZZZZ LLLL LLLL – литерал (при наличии)
```

