# Используем базовый образ с инструментами для сборки
FROM gcc:latest

# Устанавливаем необходимые зависимости
RUN apt-get update && apt-get install -y cmake make git

# Копируем исходный код
WORKDIR /app
COPY . .

# Сборка проекта с использованием CMake
RUN cd /app && cmake -S . -B build && cd build && make

# Указываем порт для прослушивания
EXPOSE 8080

# Запуск прокси
CMD ["./build/proxy"]

