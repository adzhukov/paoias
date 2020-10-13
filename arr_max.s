MOV_RM ecx 0x4000
MOV_RL ebx 0x4004

MOV_RT edx ebx
CMP_RR eax edx
CMOV_GT_RR eax edx
ADD_RL ebx 0x4
SUB_RL ecx 0x1
TEST_RR ecx ecx
JZ_R 0x4

SUB_RL eip 0x20

HALT
