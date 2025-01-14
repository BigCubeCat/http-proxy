#include <csignal>
#include <iostream>
#include <memory>

#include <unistd.h>

#include <spdlog/spdlog-inl.h>
#include <spdlog/spdlog.h>

#include "config.hpp"
#include "const.hpp"
#include "proxy.hpp"


void sigpipe_handler(int _unused) {
    spdlog::warn("SIGPIPE recieved");
}

std::shared_ptr<http_proxy_t> proxy_inst;

void sigint_handler(int status) {
    spdlog::info("exiting");
    proxy_inst->stop(status);
    std::cout << "terminating\n";
    sleep(2);
}

int main(int argc, char *argv[]) {
    spdlog::set_pattern("[%^%l%$] %v");

    std::signal(SIGPIPE, sigpipe_handler);

    auto conf = load_config(argc, argv);
    if (conf.help) {
        std::cout << USAGE_MESSAGE;
        return 0;
    }
    spdlog::set_level(conf.logging_level);
    auto cache = std::make_shared<cache_t>(conf.cache_size, conf.ttl);

    proxy_inst = std::make_shared<http_proxy_t>(
        cache.get(), conf.proxy_port, static_cast<int>(conf.max_client_threads)
    );

    std::signal(SIGINT, sigint_handler);

    proxy_inst->run();

    return 0;
}
