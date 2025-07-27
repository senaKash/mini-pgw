// include/pgw/config.hpp
#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace pgw {

// Класс, представляющий конфигурацию для PGW сервера
class Config {
public:
    // Параметры для UDP
    std::string            udp_ip;           // IP-адрес для прослушивания UDP пакетов
    uint16_t               udp_port;         // Порт для UDP соединений

    // Параметры сессий
    uint32_t               session_timeout_sec;  // Таймаут сессии в секундах

    // Параметры для записи CDR и логирования
    std::string            cdr_file;         // Путь к файлу для записи CDR
    std::string            log_file;         // Путь к лог-файлу
    std::string            log_level;        // Уровень логирования (например, "INFO", "DEBUG")

    // Параметры для HTTP API
    uint16_t               http_port;        // Порт для HTTP API
    uint32_t               graceful_shutdown_rate;  // Скорость завершения работы сессий

    // Чёрный список IMSI
    std::vector<std::string> blacklist;      // Список IMSI, для которых запросы отклоняются

    // Выбор хранилища сессий
    //   "in_memory" - для хранения в памяти или "sqlite" - для использования SQLite базы данных
    std::string            session_store;    // Тип хранилища сессий (например, "in_memory" или "sqlite")

    // Путь к базе данных SQLite (используется, если session_store == "sqlite")
    std::string            sqlite_db_path;   // Путь к файлу SQLite базы данных

    // Статический метод для загрузки конфигурации из файла
    static Config load_from_file(const std::string& path);  // Загрузка конфигурации из JSON файла
};

} // namespace pgw
