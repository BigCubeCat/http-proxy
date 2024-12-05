#pragma once
#include <vector>

#include <sys/epoll.h>

#include "const.hpp"

/*!
 * Объект прокси
 */

class http_proxy_t {
private:
    int m_epoll_fd      = -1;
    int m_listen_fd     = -1;
    int m_port          = 8080;
    int m_server_socket = -1;
    bool m_is_running   = false;

    std::vector<epoll_event> m_events;

    /*!
     * Переключает файловый дискриптор в неблокирующий режим
     * \param[in] fd файловый дескриптор
     */
    static void set_not_blocking(int fd);

    void accept_client() const;

    void process_client_fd(int i);

public:
    explicit http_proxy_t(int port);

    void run();
};
