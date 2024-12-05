# Используем Python как базу
FROM python:3.9-slim

# Устанавливаем зависимости
RUN pip install requests

# Копируем клиентский скрипт
WORKDIR /app
COPY client.py .

# Запуск клиента
CMD ["python", "client.py"]

