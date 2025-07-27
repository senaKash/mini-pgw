#include "pgw/http_api.hpp"
#include <httplib.h>
#include <spdlog/spdlog.h>
#include <future>  // Для использования std::async

namespace pgw {

HttpApi::HttpApi(uint16_t port,
                 SessionManager& sessions,
                 std::function<void()> udp_stop_cb,
                 size_t graceful_rate)
    : port_(port)
    , sessions_(sessions)
    , graceful_rate_(graceful_rate)
    , udp_stop_cb_(std::move(udp_stop_cb))
{ }

void HttpApi::start() {
    running_ = true;
    thread_ = std::thread(&HttpApi::run_server, this);
    spdlog::info("HTTP API listening on port {}", port_);
}

void HttpApi::stop() {
    running_ = false;

    // Останавливаем UDP-сервер, если он был запущен
    if (udp_stop_cb_) {
        udp_stop_cb_();  // Останавливаем UDP сервер
        spdlog::info("UDP server stop callback invoked");
    }
}

void HttpApi::join() {
    if (thread_.joinable())
        thread_.join();
}

void HttpApi::run_server() {
    httplib::Server server;

    // Флаг для остановки сервера
    bool stop_requested = false;

    // GET /check_subscriber?imsi=…
    server.Get("/check_subscriber", [this](const httplib::Request& req, httplib::Response& res) {
        auto imsi = req.get_param_value("imsi");
        bool active = sessions_.is_active(imsi);
        res.set_content(active ? "active" : "not active", "text/plain");
        spdlog::info("HTTP /check_subscriber imsi={} -> {}", imsi, active ? "active" : "not active");
    });

    // GET /stop
    server.Get("/stop", [this, &server, &stop_requested](const httplib::Request&, httplib::Response& res) {
        spdlog::info("HTTP /stop called via GET");

        // 1) Останавливаем UDP-сервер
        if (udp_stop_cb_) {
            udp_stop_cb_();
            spdlog::info("UDP server stop callback invoked");
        }

        // 2) Устанавливаем флаг на остановку
        stop_requested = true;
        spdlog::info("Stop flag set, server will stop after handling current requests.");

        // 3) Graceful offload сессий
        spdlog::info("Starting graceful stop of sessions");
        sessions_.graceful_stop(graceful_rate_);
        spdlog::info("All sessions offloaded, HTTP API shutting down");

        // 4) Останавливаем HTTP-сервер
        if (stop_requested) {
            spdlog::info("Stopping HTTP server...");
            server.stop();
        }

        res.set_content("shutting down", "text/plain");
    });

    // GET запрос на /test
    server.Get("/test", [](const httplib::Request& req, httplib::Response& res) {
        spdlog::info("Received GET request to /test");
        res.set_content("Test GET request successful", "text/plain");
    });

    // Ожидаем запросы, пока флаг не будет установлен
    while (!stop_requested) {
        try {
            server.set_error_handler([](const httplib::Request&, httplib::Response& res) {
                res.set_content("Internal Server Error", "text/plain");
            });

            spdlog::info("Listening for requests on port {}", port_);
            server.listen("0.0.0.0", port_);
        } catch (const std::exception& e) {
            spdlog::error("Error while listening: {}", e.what());
            break;
        }
    }

    spdlog::info("Server has been stopped.");

    // Завершаем программу
    std::exit(0); // Завершаем программу после остановки сервера
}


} // namespace pgw
