// src/server/config.cpp
#include "pgw/config.hpp"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

namespace pgw {

Config Config::load_from_file(const std::string& path) {
    // Читаем весь файл
    std::ifstream in(path);
    if (!in.is_open()) {
        throw std::runtime_error("Cannot open config file: " + path);
    }

    json j;
    try {
        in >> j;
    } catch (const json::parse_error& e) {
        throw std::runtime_error(std::string("JSON parse error: ") + e.what());
    }

    Config cfg;

    // Обязательные поля
    try {
        cfg.udp_ip                  = j.at("udp_ip").get<std::string>();
        cfg.udp_port                = j.at("udp_port").get<uint16_t>();
        cfg.session_timeout_sec     = j.at("session_timeout_sec").get<uint32_t>();
        cfg.cdr_file                = j.at("cdr_file").get<std::string>();
        cfg.http_port               = j.at("http_port").get<uint16_t>();
        cfg.graceful_shutdown_rate  = j.at("graceful_shutdown_rate").get<uint32_t>();
        cfg.log_file                = j.at("log_file").get<std::string>();
        cfg.log_level               = j.at("log_level").get<std::string>();
        cfg.blacklist               = j.at("blacklist").get<std::vector<std::string>>();
        cfg.session_store          = j.value("session_store", std::string("in_memory"));
        cfg.sqlite_db_path         = j.value("sqlite_db_path", std::string("sessions.db"));

    } catch (const json::type_error& e) {
        throw std::runtime_error(std::string("Config type error: ") + e.what());
    } catch (const json::out_of_range& e) {
        throw std::runtime_error(std::string("Missing config field: ") + e.what());
    }

    // Логирование успешной загрузки
    spdlog::info("Config loaded from {}", path);
    spdlog::info(" UDP: {}:{}", cfg.udp_ip, cfg.udp_port);
    spdlog::info(" Session timeout: {} sec", cfg.session_timeout_sec);
    spdlog::info(" Session store: {}", cfg.session_store);
    if (cfg.session_store == "sqlite") {
        spdlog::info(" SQLite DB path: {}", cfg.sqlite_db_path);
    }
    spdlog::info(" HTTP port: {}", cfg.http_port);
    spdlog::info(" Graceful shutdown rate: {} sessions/sec", cfg.graceful_shutdown_rate);
    spdlog::info(" CDR file: {}", cfg.cdr_file);
    spdlog::info(" Log file: {}, level: {}", cfg.log_file, cfg.log_level);
    spdlog::info(" Blacklist count: {}", cfg.blacklist.size());

    return cfg;
}

} // namespace pgw
