// mini-pgw/include/pgw/udp_server.hpp
#pragma once

#include "blacklist.hpp"  // Подключение чёрного списка для проверки IMSI
#include "session_manager.hpp"  // Подключение менеджера сессий для работы с сессиями
#include <string>
#include <thread>
#include <atomic>

namespace pgw {

// Класс для работы с UDP сервером
// Обрабатывает прием UDP пакетов и взаимодействует с черным списком и менеджером сессий
class UdpServer {
public:
    // Конструктор для инициализации сервера с указанием параметров IP, порта, чёрного списка и менеджера сессий
    UdpServer(const std::string& ip,  // IP-адрес для прослушивания
              uint16_t port,         // Порт для прослушивания
              Blacklist& blacklist,  // Чёрный список для проверки IMSI
              SessionManager& sessions);  // Менеджер сессий для управления активными сессиями

    // Метод для запуска UDP сервера, который будет работать в фоновом потоке
    virtual void start();

    // Метод для ожидания завершения работы потока
    virtual void join();

    // Метод для остановки приёма новых пакетов
    virtual void stop();

private:
    // Основной цикл обработки UDP пакетов
    void run_loop();

    std::string ip_;  // IP-адрес для прослушивания UDP пакетов
    uint16_t port_;   // Порт для прослушивания UDP пакетов
    Blacklist& blacklist_;  // Ссылка на объект чёрного списка
    SessionManager& sessions_;  // Ссылка на объект менеджера сессий

    std::thread      thread_;  // Поток для выполнения приёма данных
    std::atomic<bool> running_{false};  // Флаг, указывающий на состояние сервера (работает или нет)
};

} // namespace pgw
