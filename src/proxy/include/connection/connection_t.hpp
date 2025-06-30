#pragma once

#include "proxy_server_iface.hpp"

/*!
 * Интерфейс процессинга соединения
 */
class connection_t {
public:
    connection_t() = default;

    virtual bool process_input([[maybe_unused]] proxy_server_iface *server
    ) = 0;    // if res is true then connection should be deleted

    virtual bool process_output([[maybe_unused]] proxy_server_iface *server
    ) = 0;    // if res is true then connection should be deleted

    virtual ~connection_t() = default;
};
