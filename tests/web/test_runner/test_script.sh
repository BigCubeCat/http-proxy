#!/bin/bash

# Тест сайта 1: Проверка скачивания файла
echo "Тестируем сайт 1..."
curl --proxy1.0 http://proxy:9000/ -s -o /dev/null -w "%{http_code}" http://site1:8081/ | grep -q "200" || exit 1

# Тест сайта 2: Проверка текстового ответа
echo "Тестируем сайт 2..."
curl --proxy1.0 http://proxy:9000/ -s http://site2:8082/ | grep -q "Welcome to the text-only site!" || exit 1

# Тест сайта 3: Проверка перенаправления
echo "Тестируем сайт 3..."
curl --proxy1.0 http://proxy:9000/ -s -o /dev/null -w "%{http_code}" -L http://site3:8083/ | grep -q "200" || exit 1

# Если все тесты прошли успешно
echo "Все тесты пройдены успешно."
exit 0

