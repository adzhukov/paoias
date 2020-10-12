MOV_RM ecx 0x4000

MOV_RL ebp 0x4004
MOV_RL edx 0x5004

MOV_RT esi ebp
ADD_RL ebp 0x4

MOV_RT edi edx
ADD_RL edx 0x4

MUL_RR esi edi

ADD_RR ebx edi
ADC_RR eax esi

SUB_RL ecx 0x1
TEST_RR ecx ecx
JZ_R 0x4

SUB_RL eip 0x2C

HALT
