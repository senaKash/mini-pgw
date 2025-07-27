#include "client.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <cstring>
#include <thread>
#include <sstream>
#include <random>

// Генерация случайного IMSI
// IMSI состоит из 15 цифр, где первые 12 цифр фиксированы, а последние 3 генерируются случайно
std::string generate_imsi() {
    std::random_device rd;
    std::uniform_int_distribution<int> dist(0, 9);
    std::ostringstream imsi;
    
    // Формируем IMSI как строку из 15 цифр
    imsi << "001010123456";  // постоянная часть IMSI
    for (int i = 0; i < 3; ++i) {  // Добавляем случайные цифры
        imsi << dist(rd);
    }
    return imsi.str();
}

// Функция для преобразования IMSI в BCD (Binary-Coded Decimal) формат
std::vector<uint8_t> imsi_to_bcd(const std::string& imsi) {
    std::vector<uint8_t> bcd;
    for (size_t i = 0; i < imsi.size(); i += 2) {
        uint8_t low  = imsi[i]   - '0';  // Низкий 4 бита
        uint8_t high = (i + 1 < imsi.size()) ? imsi[i + 1] - '0' : 0xF;  // Высокий 4 бита
        bcd.push_back((high << 4) | low);  // Объединяем 2 цифры в 1 байт
    }
    return bcd;
}

// Функция для отправки данных через сокет
// Отправляет данные на сервер по UDP
int send_data(int sock, const uint8_t* data, size_t size, const sockaddr_in& serv) {
    ssize_t sent = ::sendto(sock, data, size, 0, reinterpret_cast<const sockaddr*>(&serv), sizeof(serv));
    if (sent < 0) {
        spdlog::error("sendto failed: {}", std::strerror(errno));  // Логирование ошибки при отправке
        return EXIT_FAILURE;
    }
    spdlog::info("Sent {} bytes", sent);  // Логирование успешной отправки данных

    return EXIT_SUCCESS;
}

// Функция для отправки запроса на создание сессии и обработки ответа от сервера
int request_session(const std::string& imsi, const std::string& server_ip, uint16_t server_port) {
    // Создаём UDP-сокет
    int sock = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        spdlog::error("Failed to create socket: {}", std::strerror(errno));  // Логирование ошибки при создании сокета
        return EXIT_FAILURE;
    }

    // Настроим адрес сервера
    sockaddr_in serv{};  
    serv.sin_family = AF_INET;
    serv.sin_port   = htons(server_port);
    if (inet_pton(AF_INET, server_ip.c_str(), &serv.sin_addr) <= 0) {
        spdlog::error("Invalid server IP: {}", server_ip);  // Логирование ошибки при неверном IP
        ::close(sock);
        return EXIT_FAILURE;
    }

    // Отправляем IMSI
    auto payload = imsi_to_bcd(imsi);  // Закодированный IMSI в BCD
    if (send_data(sock, payload.data(), payload.size(), serv) != EXIT_SUCCESS) {
        ::close(sock);
        return EXIT_FAILURE;
    }

    // Получаем ответ от сервера
    char buf[64] {};
    ssize_t rec = ::recvfrom(sock, buf, sizeof(buf) - 1, 0, nullptr, nullptr);
    if (rec < 0) {
        spdlog::error("recvfrom timeout/error: {}", std::strerror(errno));  // Логирование ошибки при получении данных
        ::close(sock);
        return EXIT_FAILURE;
    }

    buf[rec] = '\0';  // Завершаем строку
    std::string response(buf);  // Ответ от сервера
    if (response == "created") {
        spdlog::info("IMSI {}: Session created", imsi);  // Логирование успешного создания сессии
    } else {
        spdlog::warn("IMSI {}: Unknown response: {}", imsi, response);  // Логирование неизвестного ответа от сервера
    }

    ::close(sock);
    return EXIT_SUCCESS;
}

// Функция для продления сессии несколько раз с вероятностью 20%
int extend_session_multiple_times(const std::string& imsi, const std::string& server_ip, uint16_t server_port) {
    // Добавляем случайность для вероятности 20%
    int num_extensions = (rand() % 5 == 0) ? 1 : 0;  // 20% вероятность продления

    if (num_extensions > 0) {
        // Если продление сессии будет происходить, запускаем продление
        spdlog::info("Extending session for IMSI: {}", imsi);  // Логируем попытку продления
        if (request_session(imsi, server_ip, server_port) != EXIT_SUCCESS) {
            return EXIT_FAILURE;  // Прерываем, если произошла ошибка при продлении сессии
        }
        std::this_thread::sleep_for(std::chrono::seconds(1 + rand() % 5));  // Задержка между продлениями
    } else {
        spdlog::info("No extension for IMSI: {}", imsi);  // Логируем отсутствие продления
    }

    return EXIT_SUCCESS;
}

// Функция для запуска множества клиентов
void run_multiple_clients(const std::string& server_ip, uint16_t server_port, int num_clients) {
    std::vector<std::thread> client_threads;  // Массив потоков для клиентов

    for (int i = 0; i < num_clients; ++i) {
        std::string imsi = generate_imsi();  // Генерируем случайный IMSI для клиента
        client_threads.push_back(std::thread([&server_ip, server_port, imsi]() {
            // Запускаем клиента с данным IMSI
            if (request_session(imsi, server_ip, server_port) == EXIT_SUCCESS) {
                extend_session_multiple_times(imsi, server_ip, server_port);  // Если сессия создана, пробуем её продлить
            }
        }));
    }

    // Дожидаемся завершения всех потоков
    for (auto& t : client_threads) {
        if (t.joinable()) {
            t.join();  // Ожидаем завершения работы каждого потока
        }
    }
}
