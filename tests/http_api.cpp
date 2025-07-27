// tests/test_http_api.cpp

#include "http_api.hpp"
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <httplib.h>  // Подключаем httplib.h

class HttpApiTest : public ::testing::Test {
protected:
    uint16_t port = 8080;
    size_t graceful_rate = 10;
    bool udp_called = false;  // Используем флаг для проверки вызова UDP-стопа

    // Используем std::function для лямбды
    std::function<bool(const std::string&)> is_active_lambda = [](const std::string& imsi) {
        // Имитация проверки активности сессии
        if (imsi == "imsi_123") return true;
        return false;
    };

    void graceful_stop(size_t rate) {
        // Просто имитация завершения работы сессий
        spdlog::info("Gracefully stopping with rate {}", rate);
    }

    void udp_stop_cb() {
        udp_called = true;  // Проверка, был ли вызван callback
    }

    pgw::HttpApi* api = nullptr;

    void SetUp() override {
        // Используем лямбду вместо обычной функции-члена
        api = new pgw::HttpApi(port, is_active_lambda, udp_stop_cb, graceful_rate);
    }

    void TearDown() override {
        delete api;
    }
};

// Пример теста на проверку работы /check_subscriber
TEST_F(HttpApiTest, CheckSubscriber) {
    // Имитируем проверку активности
    httplib::Client cli("http://localhost:8080");
    httplib::Response res = cli.Get("/check_subscriber?imsi=imsi_123");

    EXPECT_EQ(res.body(), "active");
}

// Пример теста на проверку работы /stop
TEST_F(HttpApiTest, StopServer) {
    // Имитируем вызов stop API
    httplib::Client cli("http://localhost:8080");
    httplib::Response res = cli.Get("/stop");

    EXPECT_EQ(res.body(), "shutting down");
    EXPECT_TRUE(udp_called);  // Проверяем, был ли вызван callback
}
