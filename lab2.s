movl 0x1000
mem eax
movto ecx
mov eex
cmp ecx
je 0x13
add 0x1
movto eex
add 0x1000
mem eax
movto ebx
mov eex
add 0x2000
mem eax
mul ebx
addto egx
mov ebx
adc efx
jmp 0x3
exit 0x0

// Результат в регистрах efx и egx
