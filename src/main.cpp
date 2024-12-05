#include <iostream>

#include <spdlog/spdlog-inl.h>

#include "config.hpp"
#include "proxy.hpp"

const std::string USAGE_MESSAGE =
    "\t--help - вывод сообщения о том как запускать прокси, возможных флагах и "
    "их описания\n"
    "\t--port порт, на котором слушает прокси. По умолчанию 8080\n"
    "\t--max-client-threads - максимальное кол-во работающих "
    "потоков\n\t\t(размер "
    "пула клиентских подключений).  По умолчанию 4\n"
    "\t--cache-ttl - время жизни кэш записи\n"
    "\t--cache-size - максимальный размер кэша\n";

int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::trace);
    http_proxy_t proxy(8080);
    std::cout << "hello world!" << "\n";
    auto conf = load_config(argc, argv);
    if (conf.help) {
        std::cout << USAGE_MESSAGE;
    }

    proxy.run();

    return 0;
}
