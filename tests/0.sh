#!/bin/bash

cd ../tests/web/

# Функция для завершения Docker и обработки ошибок
cleanup() {
  echo "Останавливаю контейнеры..."
  docker-compose down
}

# Установка обработчика завершения
trap cleanup EXIT

# Запуск Docker
echo "Запускаю контейнеры..."
docker-compose up --build -d || exit 1

# Задержка для инициализации контейнеров
echo "Ожидание инициализации контейнеров..."
sleep 5

echo "Тестируем сайт 1..."
curl --proxy1.0 http://localhost:9000 -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8081/largefile.zip | grep -q "200" || exit 1

echo "Тестируем сайт 2..."
curl --proxy1.0 http://localhost:9000 -s http://127.0.0.1:8082/ | grep -q "Welcome to the text-only site!" || exit 1

echo "Тестируем сайт 3..."
curl --proxy1.0 http://localhost:9000 -s -o /dev/null -w "%{http_code}" -L http://127.0.0.1:8083/ | grep -q "200" || exit 1

echo "Все тесты пройдены успешно."
exit 0

