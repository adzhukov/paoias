movl 0x1000
movto ebx
mem ebx
movto ecx
movl 0x0
cmp ecx
je 0xf
movl 0x1
addto ebx
subfrom ecx
mem ebx
cmp edx
jl 0x4
movto edx
jmp 0x4
exit 0x0

// Результат в регистре edx
// Максимальный элемент
