#pragma once


class http_proxy_t {
private:
    int m_port;
    int m_server_socket = -1;
    bool m_is_running   = false;

public:
    explicit http_proxy_t(int port);

    void run();

    int accept_client();
};
