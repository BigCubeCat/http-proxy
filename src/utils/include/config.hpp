#pragma once

#include <cstddef>
#include <cstdint>

#include <spdlog/spdlog.h>

struct proxy_config_t {
    explicit proxy_config_t() = default;
    /*! порт, на котором слушает прокси. По умолчанию 8080
     */
    uint16_t proxy_port = 8080;
    /*! максимальное кол-во работающих потоков (размер пула клиентских
     * подключений).  По умолчанию 4
     */
    size_t max_client_threads = 4;
    /*!
     *  вывод сообщения о том как запускать прокси, возможных флагах и их
     * описания
     */
    bool help = false;
    /*!
     * время жизни кэш записи. По умолчанию 5с
     */
    long ttl = 5;
    /*!
     * Максимальный размер кэша
     */
    size_t cache_size = 10000;

    spdlog::level::level_enum logging_level = spdlog::level::info;
};

proxy_config_t load_config(int argc, char **argv);

void debug(const proxy_config_t &conf);
