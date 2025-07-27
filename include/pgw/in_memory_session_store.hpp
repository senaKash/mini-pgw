// include/pgw/in_memory_session_store.hpp
#pragma once

#include "pgw/session_store.hpp"
#include <mutex>
#include <unordered_map>

namespace pgw {

// Класс для хранения сессий в памяти, реализующий интерфейс ISessionStore
// Этот класс управляет сессиями в оперативной памяти (в отличие от других возможных вариантов, таких как база данных)
class InMemorySessionStore : public ISessionStore {
public:
    InMemorySessionStore() = default;  // Конструктор по умолчанию для инициализации объекта
    ~InMemorySessionStore() override = default;  // Деструктор для правильного освобождения ресурсов

    // Метод для загрузки активных сессий
    // Этот метод возвращает все сессии, которые считаются активными на данный момент
    // Параметр 'now' может быть использован для фильтрации сессий по времени
    std::vector<StoredSession> load_sessions(const std::string& now) override;

    // Метод для сохранения или обновления сессии
    // Сохраняет новую сессию или обновляет существующую в контейнере
    bool save_session(const StoredSession& s) override;

    // Метод для удаления сессии
    // Удаляет сессию из контейнера по уникальному IMSI
    bool delete_session(const std::string& imsi) override;

    // Метод для проверки существования сессии
    // Проверяет, есть ли сессия с данным IMSI в хранилище
    bool session_exists(const std::string& imsi) override;

    // Метод для удаления просроченных сессий
    // Удаляет сессии, которые истекли, но не возвращает их для дальнейшей обработки
    void cleanup_expired_sessions(const std::string& now) override;

    // Метод для загрузки всех просроченных сессий
    // Возвращает все сессии, которые просрочены, для последующего логирования
    std::vector<StoredSession> load_expired_sessions(const std::string& now) override;

private:
    std::mutex mtx_;  // Мьютекс для синхронизации доступа к данным (сессиям) между потоками
    // Контейнер для хранения сессий в памяти
    // Ключом является IMSI абонента, а значением — структура StoredSession, представляющая саму сессию
    std::unordered_map<std::string, StoredSession> sessions_;
};

} // namespace pgw
