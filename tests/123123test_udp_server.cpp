#include <gtest/gtest.h>
#include "pgw/udp_server.hpp"
#include <thread>
#include <chrono>

using namespace pgw;

class UdpServerTest : public ::testing::Test {
protected:
    SimpleBlacklist blacklist;
    SimpleSessionManager session_manager;
    UdpServer udp_server{"127.0.0.1", 9999, blacklist, session_manager};

    void SetUp() override {
        // Можно настроить начальные условия для тестов
    }

    void TearDown() override {
        udp_server.stop();
        udp_server.join();
    }
};

TEST_F(UdpServerTest, StartAndStop) {
    udp_server.start();

    // Подождем немного для того, чтобы сервер начал работать
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    udp_server.stop();
    udp_server.join();

    // Тестируем, что сервер остановлен (например, можно добавить флаг в самом классе)
    EXPECT_FALSE(udp_server.is_running());
}

TEST_F(UdpServerTest, ProcessNotBlockedImsi) {
    std::string imsi = "1234567890";  // IMSI, который не заблокирован

    udp_server.start();

    // Эмулируем получение IMSI (здесь можно было бы использовать специализированную функцию для теста)
    udp_server.receive_imsi(imsi);  // Пример вызова для эмуляции получения IMSI

    udp_server.stop();
    udp_server.join();

    // Проверка, что сессия была создана (можно проверять внутренние изменения, если нужно)
}

TEST_F(UdpServerTest, ProcessBlockedImsi) {
    std::string imsi = "blocked123";  // Заблокированный IMSI

    udp_server.start();

    // Эмулируем получение заблокированного IMSI
    udp_server.receive_imsi(imsi);  // Пример вызова для получения IMSI

    udp_server.stop();
    udp_server.join();

    // Проверка, что сессия не была создана (можно проверять внутренние изменения, если нужно)
}
