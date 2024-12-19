#pragma once

#include <string>

// Максимальное количество клиентов
constexpr int MAX_EVENTS         = 4096;
constexpr int BUFFER_SIZE        = 256;
constexpr int CACHE_TTL          = 300;    // Время жизни кеша в секундах
constexpr int EPOLL_WAIT_TIMEOUT = 100;

const std::string USAGE_MESSAGE =
    "\t--help - вывод сообщения о том как запускать прокси, возможных флагах и "
    "их описания\n"
    "\t--port порт, на котором слушает прокси. По умолчанию 8080\n"
    "\t--max-client-threads - максимальное кол-во работающих "
    "потоков\n\t\t(размер "
    "пула клиентских подключений).  По умолчанию 4\n"
    "\t--cache-ttl - время жизни кэш записи\n"
    "\t--cache-size - максимальный размер кэша\n";
