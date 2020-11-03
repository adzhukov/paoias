movl eax 0x1000
memr ecx eax
cmp eex ecx
jmp eq 0xf
add eex 0x1
movr eax eex
add eax 0x1000
memr ebx eax
movr eax eex
add eax 0x2000
memr eax eax
mul eax ebx
addr egx eax
adc efx ebx
jmp any 2
exit 0x0 0x0

// Результат в регистрах efx и egx
