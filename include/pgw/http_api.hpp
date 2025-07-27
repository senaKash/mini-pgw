// include/pgw/http_api.hpp
#pragma once

#include <cstdint>
#include <thread>
#include <functional>
#include "pgw/session_manager.hpp"

namespace pgw {

// Класс, представляющий HTTP API для взаимодействия с сервером PGW
class HttpApi {
public:
    // Конструктор класса HttpApi, инициализирующий параметры сервера HTTP
    // Добавлен callback для остановки UDP-сервера
    HttpApi(uint16_t port,  // Порт для запуска HTTP API сервера
            SessionManager& sessions,  // Менеджер сессий, с которым будет работать API
            std::function<void()> udp_stop_cb,  // Callback для остановки UDP сервера
            size_t graceful_rate);  // Скорость завершения сессий при остановке

    // Метод для запуска HTTP сервера
    void start();

    // Метод для остановки HTTP сервера
    void stop();

    // Метод для ожидания завершения работы потока сервера
    void join();

private:
    // Вспомогательный метод для запуска HTTP сервера в отдельном потоке
    void run_server();

    uint16_t              port_;  // Порт для прослушивания HTTP запросов
    SessionManager&       sessions_;  // Ссылка на объект менеджера сессий
    size_t                graceful_rate_;  // Скорость завершения сессий при остановке
    std::function<void()> udp_stop_cb_;  // Callback-функция для остановки UDP сервера
    std::thread           thread_;  // Поток для работы HTTP сервера
    bool                  running_ = false;  // Флаг, показывающий, что сервер работает
};

} // namespace pgw
