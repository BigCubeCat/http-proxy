# HTTP1.0 Proxy

## Сборка

Для сборки необходим spdlog и gtest.

```bash
cmake -S . -B build && cmake --build build
```

## Исользование

```bash
./build/http_proxy --help
hello world!
        --help - вывод сообщения о том как запускать прокси, возможных флагах и их описания
        --port порт, на котором слушает прокси. По умолчанию 8080
        --max-client-threads - максимальное кол-во работающих потоков
                (размер пула клиентских подключений).  По умолчанию 4
        --cache-ttl - время жизни кэш записи
        --cache-size - максимальный размер кэша
```

## Запуск

```bash
./build/http_proxy
```

## Тестирование

для некоторых тестов необходим docker compose

- [ ] basic
- [x] sequential
- [x] concurrent-batches
- [x] cache-invalidation
- [ ] parallel-clients
- [x] incremental-interrupt

0. (Собирите)[#Сборка] проект в директорию `build`
1. Запустите прокси (на порту 9000)[#Использование]
2. Параллельно запустите `ctest` (можно использовать, например, tmux)

```bash
ctest --output-on-failure --test-dir ./build
```

## TODO

- [x] Функционал кэша
- [x] Инвалидация кэша
- [x] Обычные HTML страницы
- [x] Файлы
- [x] redirect
- [ ] оптимизация
