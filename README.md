# HTTP1.0 Proxy

## Исользование

```bash
➜  http-proxy git:(v0.1) ✗ ./build/http_proxy --help
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
cmake -S . -B build && cmake --build build
./build/http_proxy
```

## Запуск через docker

```bash
docker-compose up --build
```

## Тестирование

0. Собирите проект в директорию `build`

```bash
cmake -S . -B build  && cmake --build buildc
```

1. Запустите прокси на 9000 порту

```bash
./http_proxy --port 9000
```

2. Параллельно запустите `ctest`

```bash
ctest --output-on-failure --test-dir ./build
```

## TODO

### Thread Pool & Cache

- [x] Функционал кэша
- [x] Инвалидация кэша
- [ ] Частичная загрузка файла в кэш

### Функционал

- [x] Обычные HTML страницы
- [x] Файлы
- [ ] redirect

### HTTP Методы

- [x] GET
- [ ] POST
- [ ] PUT
- [ ] UPDATE
- [ ] DELETE
- [ ] PATCH
