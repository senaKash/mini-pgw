// mini-pgw/include/pgw/session_store.hpp
#pragma once

#include <string>
#include <vector>

namespace pgw {

struct StoredSession {
    std::string imsi;
    std::string created_at;
    std::string expires_at;
};

class ISessionStore {
public:
    virtual ~ISessionStore() = default;

    /// Возвращает все сессии, у которых expires_at > now
    virtual std::vector<StoredSession> load_sessions(const std::string& now) = 0;
    /// Сохраняет новую или обновляет существующую сессию
    virtual bool save_session(const StoredSession& s) = 0;
    /// Удаляет сессию по IMSI
    virtual bool delete_session(const std::string& imsi) = 0;
    /// Проверяет, есть ли сессия с данным IMSI
    virtual bool session_exists(const std::string& imsi) = 0;
    /// Удаляет все «просроченные» сессии (expires_at <= now)
    virtual void cleanup_expired_sessions(const std::string& now) = 0;
    
    virtual std::vector<StoredSession> load_expired_sessions(const std::string& now) = 0;
};

} // namespace pgw
