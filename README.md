# HTTP1.0 Proxy

## Usage

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
./build/http_proxy --help
```

## Using docker

```bash
docker-compose up --build
```
