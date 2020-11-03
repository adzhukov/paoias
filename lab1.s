movl eax 0x1000
memr ecx eax
cmp ebx ecx
jmp eq 0xa
add ebx 0x1
movr eax ebx
add eax 0x1000
memr eax eax
addr edx eax
jmp any 2
exit 0x0 0x0

// Результат в регистре edx
// Сумма
