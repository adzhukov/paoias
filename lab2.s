push
0x0 // Результат первые 4 байта
push
0x0 // Результат вторые 4 байта
push
0x0 // Количество обработанных элементов массива
dup // Копировать количество обработанных элементов для сравнения
push
0x0 // Адрес начала массива
read // Получить количество элементов в массива
cmp // Сравнение количества обработанных элементов с общим
push
0x2c // Номер команды следующей после цикла
jz // Выход из цикла при равенстве (Переход к 44 (0x2c) команде)
dup
push
0x1 // Начало первого массива
add // Получение адреса текущего элемента в первом массиве
dup
push
0x0
read // Получение общего количества элементов
add // Получение адреса текущего элемента во втором массиве
read // Получение элемента второго массива по его адресу
swap
read // Получение элемента первого массива по его адресу
mul // Умножение полученных элементов
rol
push
0x1
write // Временно поместить количество обработанных элементов в адрес первого элемента первого массива
rol
add // Сложение младших 4х байт результата
ror
adc // Сложение с учетом переноса старших 4х байт результата
swap
push
0x1
dup
read
add // Получение количества обработанных элементов и прибавление 1
push
0x6 // Адрес начала цикла
jmp // Переход в начало цикла
pop // Убрать количество обработанных элементов
halt
