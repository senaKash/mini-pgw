#include "pgw/in_memory_session_store.hpp"

namespace pgw {

// Функция, которая сравнивает строки с датами (если они в формате строк)
bool is_before(const std::string& time1, const std::string& time2) {
    return time1 < time2;
}

std::vector<StoredSession> InMemorySessionStore::load_sessions(const std::string& now) {
    std::lock_guard<std::mutex> lk(mtx_);
    std::vector<StoredSession> out;
    for (auto& [k, s] : sessions_) {
        if (is_before(now, s.expires_at)) {
            out.push_back(s);
        }
    }
    return out;
}

bool InMemorySessionStore::save_session(const StoredSession& s) {
    std::lock_guard<std::mutex> lk(mtx_);
    sessions_[s.imsi] = s;
    return true;
}

bool InMemorySessionStore::delete_session(const std::string& imsi) {
    std::lock_guard<std::mutex> lk(mtx_);
    return sessions_.erase(imsi) > 0;
}

bool InMemorySessionStore::session_exists(const std::string& imsi) {
    std::lock_guard<std::mutex> lk(mtx_);
    return sessions_.find(imsi) != sessions_.end();
}

void InMemorySessionStore::cleanup_expired_sessions(const std::string& now) {
    std::lock_guard<std::mutex> lk(mtx_);
    for (auto it = sessions_.begin(); it != sessions_.end(); ) {
        if (is_before(it->second.expires_at, now)) {
            it = sessions_.erase(it);
        } else {
            ++it;
        }
    }
}

std::vector<StoredSession> InMemorySessionStore::load_expired_sessions(const std::string& now) {
    std::lock_guard<std::mutex> lk(mtx_);
    std::vector<StoredSession> expired;
    for (auto& [k, s] : sessions_) {
        if (is_before(s.expires_at, now)) {
            expired.push_back(s);
        }
    }
    return expired;
}

} // namespace pgw
