// src/server/main.cpp
#include "pgw/config.hpp"
#include "pgw/cdr_writer.hpp"
#include "pgw/blacklist.hpp"
#include "pgw/session_manager.hpp"
#include "pgw/in_memory_session_store.hpp"
#include "pgw/sqlite_session_store.hpp"
#include "pgw/session_store.hpp"
#include "pgw/udp_server.hpp"
#include "pgw/http_api.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <filesystem>
#include <vector>
#include <memory>
#include <chrono>

int main(int argc, char* argv[]) {
    // 1. Загрузка конфигурации
    pgw::Config cfg;
    try {
        cfg = pgw::Config::load_from_file("config/server_config.json");
    } catch (const std::exception& ex) {
        spdlog::critical("Failed to load config: {}", ex.what());
        return EXIT_FAILURE;
    }

    // 2. Настройка консольного и файлового логгера
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
    console_sink->set_level(spdlog::level::from_str(cfg.log_level));  // Устанавливаем уровень логирования для консоли

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_st>(cfg.log_file, true);
    file_sink->set_level(spdlog::level::from_str(cfg.log_level));  // Устанавливаем уровень логирования для файла

    std::vector<spdlog::sink_ptr> sinks{ console_sink, file_sink };
    auto logger = std::make_shared<spdlog::logger>("pgw", sinks.begin(), sinks.end());
    spdlog::set_default_logger(logger);

    // Устанавливаем глобальный уровень логирования
    spdlog::set_level(spdlog::level::from_str(cfg.log_level));  // Устанавливаем глобальный уровень логирования

    spdlog::info("Logging to console and file: {}", std::filesystem::absolute(cfg.log_file).string());
    spdlog::info("PGW server starting...");

    // 3. Инициализация CDR и Blacklist
    pgw::CdrWriter cdr{ cfg.cdr_file };
    pgw::Blacklist blacklist{ cfg.blacklist };

    // 4. Инициализация хранилища сессий
    std::unique_ptr<pgw::ISessionStore> store;

    if (cfg.session_store == "sqlite") {
        spdlog::info("Using SQLite session store: {}", cfg.sqlite_db_path);
        store = std::make_unique<pgw::SqliteSessionStore>(cfg.sqlite_db_path);
    } else {
        spdlog::info("Using in-memory session store");
        store = std::make_unique<pgw::InMemorySessionStore>();
    }

    // 5. Создание SessionManager
    pgw::SessionManager sessions{
        std::chrono::seconds(cfg.session_timeout_sec),
        std::move(store),
        cdr
    };

    // 6. Инициализация и запуск серверов
    pgw::UdpServer udp{ cfg.udp_ip, cfg.udp_port, blacklist, sessions };

    // Передаем в HttpApi порт, SessionManager, callback для UDP‑stop и скорость graceful‑shutdown
    pgw::HttpApi http{
        cfg.http_port,
        sessions,
        // callback: остановит UDP‑сервер
        [&udp]() {
            spdlog::info("Stopping UDP server from HTTP /stop");
            udp.stop();
        },
        // скорость offload из конфига
        cfg.graceful_shutdown_rate
    };

    udp.start();
    http.start();

    // 7. Ожидание завершения: сначала HTTP (after /stop), затем UDP
    http.join();
    udp.join();

    spdlog::info("PGW server stopped");
    return 0;
}
