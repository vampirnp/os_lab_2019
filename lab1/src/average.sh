#!/bin/bash

# Проверяем, что были переданы аргументы
if [ $# -eq 0 ]; then
  echo "Нет входных аргументов."
  exit 1
fi

# Инициализируем переменные
sum=0
count=0

# Вычисляем сумму и количество аргументов
for arg in "$@"; do
  sum=$((sum + arg))
  count=$((count + 1))
done

# Вычисляем среднее арифметическое
average=$(echo "scale=2; $sum / $count" | bc)

# Выводим результаты
echo "Количество аргументов: $count"
echo "Среднее арифметическое: $average"
