#include <csignal>
#include <iostream>
#include <memory>

#include <spdlog/spdlog-inl.h>
#include <spdlog/spdlog.h>

#include "cache.hpp"
#include "config.hpp"
#include "proxy.hpp"


void sigpipe_handler(int _unused) {
    spdlog::warn("SIGPIPE recieved");
}


const std::string USAGE_MESSAGE =
    "\t--help - вывод сообщения о том как запускать прокси, возможных флагах и "
    "их описания\n"
    "\t--port порт, на котором слушает прокси. По умолчанию 8080\n"
    "\t--max-client-threads - максимальное кол-во работающих "
    "потоков\n\t\t(размер "
    "пула клиентских подключений).  По умолчанию 4\n"
    "\t--cache-ttl - время жизни кэш записи\n"
    "\t--cache-size - максимальный размер кэша\n";


std::shared_ptr<http_proxy_t> proxy_inst;

void sigint_handler(int status) {
    spdlog::info("exiting");
    proxy_inst->stop(status);
}

int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::trace);

    signal(SIGPIPE, sigpipe_handler);

    auto conf = load_config(argc, argv);
    if (conf.help) {
        std::cout << USAGE_MESSAGE;
        return 0;
    }
    auto cache =
        std::make_shared<lru_cache_t<std::string>>(conf.cache_size, conf.ttl);

    spdlog::trace("client threads = {}", conf.max_client_threads);

    proxy_inst = std::make_shared<http_proxy_t>(
        cache.get(), conf.proxy_port, static_cast<int>(conf.max_client_threads)
    );

    signal(SIGINT, sigint_handler);

    proxy_inst->run();

    return 0;
}
