#pragma once

#include <cstddef>
#include <cstdint>

struct proxy_config_t {
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
};