// mini-pgw/include/pgw/session.hpp
#pragma once
#include <string>
#include <chrono>
#include <nlohmann/json.hpp>

// Структура для представления сессии
// Содержит информацию о сессии пользователя, включая идентификатор, IP-адрес, время начала и последней активности
struct Session {
    std::string session_id;  // Идентификатор сессии
    std::string user_ip;     // IP-адрес пользователя, связанный с сессией
    std::chrono::system_clock::time_point start_time;  // Время начала сессии
    std::chrono::system_clock::time_point last_activity;  // Время последней активности пользователя
};

// Функция для сериализации объекта Session в формат JSON
inline void to_json(nlohmann::json& j, const Session& s) {
    j = {
        {"session_id", s.session_id},  // Сериализуем идентификатор сессии
        {"user_ip", s.user_ip},        // Сериализуем IP-адрес пользователя
        // Сериализуем время начала сессии как количество секунд с эпохи
        {"start_time", std::chrono::duration_cast<std::chrono::seconds>(s.start_time.time_since_epoch()).count()},
        // Сериализуем время последней активности как количество секунд с эпохи
        {"last_activity", std::chrono::duration_cast<std::chrono::seconds>(s.last_activity.time_since_epoch()).count()}
    };
}

// Функция для десериализации данных JSON в объект Session
inline void from_json(const nlohmann::json& j, Session& s) {
    s.session_id = j.at("session_id").get<std::string>();  // Извлекаем идентификатор сессии
    s.user_ip = j.at("user_ip").get<std::string>();        // Извлекаем IP-адрес пользователя
    auto epoch_start = std::chrono::system_clock::time_point{};  // Начало эпохи (1970-01-01)
    // Восстанавливаем время начала сессии, преобразуя количество секунд из JSON в time_point
    s.start_time = epoch_start + std::chrono::seconds(j.at("start_time").get<int64_t>());
    // Восстанавливаем время последней активности
    s.last_activity = epoch_start + std::chrono::seconds(j.at("last_activity").get<int64_t>());
}
