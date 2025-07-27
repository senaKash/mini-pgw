#include "client.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

// Структура для хранения конфигурации
// Хранит данные, необходимые для подключения к серверу и количество клиентов
struct Config {
    std::string server_ip;  // IP-адрес сервера
    uint16_t server_port;   // Порт сервера
    int num_clients;        // Количество клиентов для запуска
    std::string default_imsi; // По умолчанию IMSI, если не передан через командную строку
};

// Функция для загрузки конфигурации из JSON файла
// Читает файл конфигурации, парсит его и извлекает нужные параметры
Config load_config(const std::string& config_file) {
    std::ifstream config_stream(config_file);
    if (!config_stream.is_open()) {
        spdlog::error("Cannot open config file: {}", config_file);  // Логирование ошибки при открытии файла
        throw std::runtime_error("Cannot open config file");  // Выбрасываем исключение при ошибке открытия файла
    }

    nlohmann::json config_json;
    config_stream >> config_json;  // Чтение JSON данных из файла

    Config config;
    try {
        // Извлечение значений из JSON
        config.server_ip = config_json.at("server_ip").get<std::string>();  // Извлекаем серверный IP
        config.server_port = config_json.at("server_port").get<uint16_t>();  // Извлекаем серверный порт
        config.num_clients = config_json.at("num_clients").get<int>();  // Извлекаем количество клиентов
        config.default_imsi = config_json.value("default_imsi", "");  // Извлекаем IMSI по умолчанию, если есть
    } catch (const nlohmann::json::exception& e) {
        spdlog::error("Error parsing config file: {}", e.what());  // Логируем ошибку при парсинге
        throw std::runtime_error("Error parsing config file");  // Выбрасываем исключение, если возникла ошибка парсинга
    }

    return config;  // Возвращаем полученную конфигурацию
}

// Точка входа программы
int main(int argc, char* argv[]) {
    std::string config_file = "config/client_config.json"; // Путь к конфигурационному файлу

    // Загружаем конфигурацию из файла
    Config config;
    try {
        config = load_config(config_file);  // Загружаем конфигурацию
    } catch (const std::exception& e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;  // Выводим ошибку в консоль, если не удалось загрузить конфигурацию
        return EXIT_FAILURE;  // Завершаем выполнение программы с ошибкой
    }

    std::string imsi;

    // Проверяем, был ли передан IMSI через командную строку
    if (argc > 1) {
        imsi = argv[1];  // Если IMSI передан, используем его
        spdlog::info("Using IMSI from command line: {}", imsi);
    } else {
        // Если IMSI не передан, используем IMSI из конфигурации или генерируем случайный
        if (config.default_imsi.empty()) {
            imsi = generate_imsi();  // Генерируем случайный IMSI, если не указано в конфиге
            spdlog::info("Using generated IMSI: {}", imsi);
        } else {
            imsi = config.default_imsi;  // Используем IMSI из конфигурации
            spdlog::info("Using IMSI from config: {}", imsi);
        }
    }

    // Запускаем множество клиентов, используя параметры из конфигурации
    run_multiple_clients(config.server_ip, config.server_port, config.num_clients);

    return 0;  // Завершаем выполнение программы с успешным результатом
}
