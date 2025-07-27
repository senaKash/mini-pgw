// include/pgw/session_manager.hpp
#pragma once

#include "pgw/session_store.hpp"  // Подключение интерфейса ISessionStore и структуры StoredSession
#include "pgw/cdr_writer.hpp"  // Подключение CdrWriter для записи данных CDR
#include <mutex>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <memory>
#include <atomic>

namespace pgw {

// Класс, управляющий сессиями, проверяющий их активность и поддерживающий "graceful shutdown"
class SessionManager {
public:
    // Конструктор, инициализирующий параметры для управления сессиями
    // session_timeout — таймаут для сессии
    // store — хранилище сессий
    // cdr_writer — объект для записи CDR
    SessionManager(std::chrono::seconds session_timeout,
                   std::unique_ptr<ISessionStore> store,
                   CdrWriter& cdr_writer);

    // Деструктор
    ~SessionManager();

    // Метод для создания новой сессии или продления существующей
    // Возвращает true, если сессия была только что создана
    bool touch_session(const std::string& imsi);

    // Метод для проверки, активна ли сессия с данным IMSI
    bool is_active(const std::string& imsi) const;

    // Метод для дозированного удаления сессий в секунду (для graceful stop)
    void offload_rate(size_t sessions_per_sec);

    // Метод для graceful shutdown: дозированное удаление сессий
    // Удаляет сессии с заданной частотой
    void graceful_stop(size_t sessions_per_sec);

private:
    // Метод для цикла очистки сессий по таймауту
    void cleaner_loop();

    // Метод для истечения срока действия сессии с заданным IMSI
    void expire_session_locked(const std::string& imsi);

    std::chrono::seconds                    timeout_;  // Таймаут для сессий
    std::unique_ptr<ISessionStore>          store_;    // Хранилище сессий
    CdrWriter&                              cdr_;      // Объект для записи CDR
    mutable std::mutex                      mtx_;      // Мьютекс для синхронизации доступа
    std::condition_variable                 cv_;       // Условная переменная для синхронизации touch_session
    std::thread                             cleaner_thread_;  // Поток для очистки сессий
    std::atomic<bool>                       stop_{false};  // Флаг остановки работы менеджера сессий
};

} // namespace pgw
