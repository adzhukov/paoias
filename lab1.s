mem ebx // Получить количество элементов
movto ecx
movl 0x0
cmp ecx // Сравнение количества оставшихся с 0
je 0xb // Выход
movl 0x1
addto ebx // Увеличить номер текущего элемента
subfrom ecx
mem ebx // Получит текущий элемент
addto edx // Прибавить к сумме
jmp 0x2 // Переход в начало цикла
exit 0x0
