// test/cdr_writer_test.cpp
#include <gtest/gtest.h>
#include "pgw/cdr_writer.hpp"
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>

namespace fs = std::filesystem;

class CdrWriterTest : public ::testing::Test {
protected:
    std::string test_file = "test_cdr_file.csv";

    void SetUp() override {
        // Удаляем файл, если он существует, перед началом теста
        if (fs::exists(test_file)) {
            fs::remove(test_file);
        }
    }

    void TearDown() override {
        // Удаляем файл после теста
        if (fs::exists(test_file)) {
            fs::remove(test_file);
        }
    }
};

// Теперь используем TEST_F вместо TEST
TEST_F(CdrWriterTest, WriteCdrRecord) {
    pgw::CdrWriter writer(test_file);

    pgw::CdrRecord record = {"2025-07-27 12:00:00", "1234567890", "created"};
    writer.write(record);

    // Даем время на запись в файл
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Проверяем, что файл существует и содержит запись
    ASSERT_TRUE(fs::exists(test_file));

    std::ifstream file(test_file);
    std::string line;
    std::getline(file, line);

    // Проверяем формат записи
    ASSERT_EQ(line, "2025-07-27 12:00:00,1234567890,created");
}

TEST_F(CdrWriterTest, WriterThreadShutdown) {
    {
        pgw::CdrWriter writer(test_file);
        pgw::CdrRecord record = {"2025-07-27 12:01:00", "0987654321", "expired"};
        writer.write(record);

        // Даем время на запись
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Проверяем, что файл существует и содержит запись после завершения работы объекта
    ASSERT_TRUE(fs::exists(test_file));

    std::ifstream file(test_file);
    std::string line;
    std::getline(file, line);

    // Проверяем корректность записи
    ASSERT_EQ(line, "2025-07-27 12:01:00,0987654321,expired");
}

// Тест на пустой путь
TEST_F(CdrWriterTest, NoFilePath) {
    // Проверяем, что выбрасывается исключение при пустом пути
    EXPECT_THROW({
        pgw::CdrWriter writer(""); // передаем пустую строку вместо пути
    }, std::invalid_argument);  // ожидаем исключение std::invalid_argument
}
