// include/pgw/sqlite_session_store.hpp
#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <sqlite3.h>
#include "pgw/session_store.hpp"  // Интерфейс для хранения сессий
#include "pgw/session.hpp"  // Структуры данных для сессий

namespace pgw {

// Класс для хранения сессий в базе данных SQLite, реализующий интерфейс ISessionStore
class SqliteSessionStore : public ISessionStore {
public:
    // Конструктор, принимающий путь к файлу базы данных
    explicit SqliteSessionStore(const std::string& db_path);
    
    // Деструктор для очистки ресурсов
    ~SqliteSessionStore();

    // Реализация интерфейса ISessionStore
    // Метод для загрузки всех сессий, актуальных на данный момент
    std::vector<StoredSession> load_sessions(const std::string& now) override;

    // Метод для сохранения сессии в базе данных
    bool save_session(const StoredSession& s) override;

    // Метод для удаления сессии по IMSI
    bool delete_session(const std::string& imsi) override;

    // Метод для проверки существования сессии в базе данных
    bool session_exists(const std::string& imsi) override;

    // Метод для удаления просроченных сессий
    void cleanup_expired_sessions(const std::string& now) override;

    // Новый метод для загрузки всех просроченных сессий, чье время истекло
    std::vector<StoredSession> load_expired_sessions(const std::string& now);

private:
    // Метод для сохранения сессии с указанием времени создания и истечения
    bool save_session(const std::string& imsi, const std::string& created_at, const std::string& expires_at);

    // Метод для инициализации схемы таблицы в базе данных (если она ещё не существует)
    void init_schema();

    sqlite3* db_;  // Указатель на объект базы данных SQLite
    std::mutex mtx_;  // Мьютекс для синхронизации доступа к базе данных
};

} // namespace pgw
