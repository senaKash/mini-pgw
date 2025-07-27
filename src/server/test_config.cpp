#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "pgw/config.hpp"
#include <fstream>
#include <nlohmann/json.hpp>

using namespace pgw;

// Мок для spdlog (чтобы предотвратить реальное логирование)
class MockLogger {
public:
    MOCK_METHOD(void, info, (const std::string& msg), ());
    MOCK_METHOD(void, warn, (const std::string& msg), ());
    MOCK_METHOD(void, error, (const std::string& msg), ());
};

// Тест для загрузки конфигурации
TEST(ConfigTest, LoadConfigFromFile_Success) {
    // Путь к тестовому файлу конфигурации
    std::string config_file = "test_config.json";

    // Создаем тестовый JSON-файл
    nlohmann::json j;
    j["udp_ip"] = "127.0.0.1";
    j["udp_port"] = 1234;
    j["session_timeout_sec"] = 300;
    j["cdr_file"] = "test_cdr.txt";
    j["http_port"] = 8080;
    j["graceful_shutdown_rate"] = 10;
    j["log_file"] = "test_log.txt";
    j["log_level"] = "info";
    j["blacklist"] = {"1234567890", "9876543210"};
    j["session_store"] = "in_memory";
    j["sqlite_db_path"] = "sessions.db";

    // Сохраняем JSON в файл
    std::ofstream out(config_file);
    out << j.dump(4);  // Записываем в файл с отступами для читаемости
    out.close();

    // Загружаем конфигурацию
    Config config = Config::load_from_file(config_file);

    // Проверка значений после загрузки
    EXPECT_EQ(config.udp_ip, "127.0.0.1");
    EXPECT_EQ(config.udp_port, 1234);
    EXPECT_EQ(config.session_timeout_sec, 300);
    EXPECT_EQ(config.cdr_file, "test_cdr.txt");
    EXPECT_EQ(config.http_port, 8080);
    EXPECT_EQ(config.graceful_shutdown_rate, 10);
    EXPECT_EQ(config.log_file, "test_log.txt");
    EXPECT_EQ(config.log_level, "info");
    EXPECT_EQ(config.blacklist.size(), 2);
    EXPECT_EQ(config.session_store, "in_memory");
    EXPECT_EQ(config.sqlite_db_path, "sessions.db");

    // Удаляем тестовый файл конфигурации после теста
    std::remove(config_file.c_str());
}

// Тест для обработки ошибки при открытии файла
TEST(ConfigTest, LoadConfigFromFile_FileNotFound) {
    std::string config_file = "non_existent_config.json";

    EXPECT_THROW(Config::load_from_file(config_file), std::runtime_error);
}

// Тест для обработки ошибки парсинга JSON
TEST(ConfigTest, LoadConfigFromFile_ParseError) {
    // Создаем некорректный JSON
    std::string config_file = "invalid_config.json";
    std::ofstream out(config_file);
    out << "{ udp_ip: 127.0.0.1, udp_port: 1234 }";  // Неправильный формат JSON
    out.close();

    EXPECT_THROW(Config::load_from_file(config_file), std::runtime_error);

    std::remove(config_file.c_str());
}

// Тест для отсутствующих обязательных полей
TEST(ConfigTest, LoadConfigFromFile_MissingFields) {
    // Создаем JSON без обязательных полей
    std::string config_file = "missing_fields_config.json";
    nlohmann::json j;
    j["udp_ip"] = "127.0.0.1";
    j["udp_port"] = 1234;

    std::ofstream out(config_file);
    out << j.dump(4);  // Записываем неполный JSON
    out.close();

    EXPECT_THROW(Config::load_from_file(config_file), std::runtime_error);

    std::remove(config_file.c_str());
}
