mem ecx
movto ecx
mov ebx
cmp ecx
je 0x13
movl 0x1
addto ebx
mem ebx
movto egx
mov ebx
movto efx
mov ecx
addto efx
mem efx
mul egx
addto eex
mov egx
adc edx
jmp 0x2
exit 0x0
