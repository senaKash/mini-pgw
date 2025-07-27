// src/server/udp_server.cpp

#include "pgw/udp_server.hpp"
#include <spdlog/spdlog.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <chrono>
#include <vector>

namespace pgw {

UdpServer::UdpServer(const std::string& ip,
                     uint16_t port,
                     Blacklist& blacklist,
                     SessionManager& sessions)
    : ip_(ip)
    , port_(port)
    , blacklist_(blacklist)
    , sessions_(sessions)
    , running_(false)
{}

void UdpServer::start() {
    running_ = true;
    thread_ = std::thread(&UdpServer::run_loop, this);
    spdlog::info("UDP server listening on {}:{}", ip_, port_);
}

void UdpServer::stop() {
    running_ = false;
}

void UdpServer::join() {
    if (thread_.joinable())
        thread_.join();
}

void UdpServer::run_loop() {
    int sock = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        spdlog::critical("Failed to create UDP socket: {}", std::strerror(errno));
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port_);
    inet_pton(AF_INET, ip_.c_str(), &addr.sin_addr);

    if (::bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        spdlog::critical("UDP bind failed: {}", std::strerror(errno));
        ::close(sock);
        return;
    }

    std::vector<uint8_t> buf(16);
    sockaddr_in client_addr{};
    socklen_t client_addr_len = sizeof(client_addr);

    while (running_) {
        ssize_t len = ::recvfrom(sock,
                                 buf.data(), buf.size(),
                                 0,
                                 reinterpret_cast<sockaddr*>(&client_addr),
                                 &client_addr_len);

        if (len <= 0) {
            // При EINTR или таймауте просто делаем паузу
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // Декодируем BCD в строку IMSI (low‑nibble → первая цифра, high‑nibble → вторая)
        std::string imsi;
        for (ssize_t i = 0; i < len; ++i) {
            uint8_t b   = buf[i];
            uint8_t low =  b        & 0x0F;
            uint8_t high= (b >> 4)  & 0x0F;
            if (low  != 0xF) imsi.push_back(char('0' + low));
            if (high != 0xF) imsi.push_back(char('0' + high));
        }
        spdlog::info("Received IMSI {}", imsi);

        // Проверяем чёрный список и создаём сессию при необходимости
        bool accepted = false;
        if (blacklist_.is_blocked(imsi)) {
            spdlog::warn("IMSI {} is blacklisted, rejecting", imsi);
        } else {
            accepted = sessions_.touch_session(imsi);
            spdlog::info("{} session for IMSI {}", accepted ? "Created" : "Rejected", imsi);
        }

        const char* resp = accepted ? "created" : "rejected";
        ssize_t sent = ::sendto(sock,
                                resp, std::strlen(resp),
                                0,
                                reinterpret_cast<sockaddr*>(&client_addr),
                                client_addr_len);

        if (sent < 0) {
            spdlog::error("Failed to send '{}' to client: {}", resp, std::strerror(errno));
        } else {
            spdlog::info("Sent {} bytes: '{}'", sent, resp);
        }
    }

    ::close(sock);
    spdlog::info("UDP server stopped");
}

} // namespace pgw
