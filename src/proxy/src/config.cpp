#include "config.hpp"

#include <cstdlib>
#include <unordered_map>

#include <spdlog/spdlog.h>


namespace {
std::unordered_map<std::string, std::string> arg_parse(int argc, char **argv) {
    std::unordered_map<std::string, std::string> options;
    std::string flag;
    for (int i = 1; i < argc; ++i) {
        std::string word = argv[i];
        if (word.starts_with("--")) {
            flag = word;
            if (flag == "--help") {
                options[flag] = "true";
            }
            continue;
        }
        if (flag != "") {
            options[flag] = word;
            flag          = "";
        }
    }
    return options;
}
std::string get_arg(
    const std::unordered_map<std::string, std::string> &args,
    const std::string &arg1
) {
    auto it = args.find(arg1);
    if (it == args.end()) {
        return "";
    }
    return it->second;
}
};    // namespace

proxy_config_t load_config(int argc, char **argv) {
    proxy_config_t config;
    auto arguments = arg_parse(argc, argv);
    if (get_arg(arguments, "--help") == "true") {
        config.help = true;
    }
    auto port = get_arg(arguments, "--port");
    if (port != "") {
        config.proxy_port = std::atoi(port.c_str());
    }
    auto max_client_threads = get_arg(arguments, "--max-client-threads");
    if (max_client_threads != "") {
        config.max_client_threads = std::atoi(max_client_threads.c_str());
    }
    auto ttl = get_arg(arguments, "--cache-ttl");
    if (ttl != "") {
        config.ttl = std::atoi(ttl.c_str());
    }
    auto size = get_arg(arguments, "--cache-size");
    if (size != "") {
        config.cache_size = std::atoi(size.c_str());
    }
    auto log_level = get_arg(arguments, "--log-level");
    if (log_level != "") {
        if (log_level == "error")
            config.logging_level = spdlog::level::err;
        else if (log_level == "debug")
            config.logging_level = spdlog::level::debug;
        else if (log_level == "trace")
            config.logging_level = spdlog::level::trace;
    }
    return config;
}

void debug(const proxy_config_t &conf) {
    spdlog::info(
        "help={}\nport={}\nthreads={}\nttl={}\nsize={}\n",
        conf.help,
        conf.proxy_port,
        conf.max_client_threads,
        conf.ttl,
        conf.cache_size
    );
}
