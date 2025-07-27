// src/server/session_manager.cpp
#include "pgw/session_manager.hpp"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <thread>

namespace pgw {

using clock = std::chrono::steady_clock;

// helper: текущее время как "YYYY-MM-DD HH:MM:SS"
static std::string now_str() {
    auto t = std::time(nullptr);
    std::ostringstream ts;
    ts << std::put_time(std::localtime(&t), "%F %T");
    return ts.str();
}

// helper: вычисляем время истечения: now + timeout → строка
static std::string expires_str(std::chrono::seconds timeout) {
    auto tp = std::chrono::system_clock::now() + timeout;
    std::time_t t = std::chrono::system_clock::to_time_t(tp);
    std::ostringstream ts;
    ts << std::put_time(std::localtime(&t), "%F %T");
    return ts.str();
}

SessionManager::SessionManager(std::chrono::seconds session_timeout,
                               std::unique_ptr<ISessionStore> store,
                               CdrWriter& cdr_writer)
  : timeout_(session_timeout)
  , store_(std::move(store))
  , cdr_(cdr_writer)
{
    // Восстанавливаем существующие сессии (непросроченные)
    auto existing = store_->load_sessions(now_str());
    for (auto& s : existing) {
        spdlog::info("Restored session {} (expires at {})", s.imsi, s.expires_at);
    }
    // Запускаем фоновую очистку
    cleaner_thread_ = std::thread(&SessionManager::cleaner_loop, this);
}

SessionManager::~SessionManager() {
    stop_ = true;
    cv_.notify_all();
    if (cleaner_thread_.joinable())
        cleaner_thread_.join();
}

bool SessionManager::touch_session(const std::string& imsi) {
    auto now     = now_str();
    auto expires = expires_str(timeout_);

    std::lock_guard<std::mutex> lk(mtx_);

    // Если сессия уже есть — пролонгируем
    if (store_->session_exists(imsi)) {
        StoredSession s{ imsi, now, expires };
        store_->save_session(s);
        spdlog::info("Session {} refreshed, expires at {}", imsi, expires);
        return true;
    }

    // Проверяем лимит
    //auto all = store_->load_sessions(now);
    //if (all.size() >= max_sessions_) {
    //    spdlog::warn("Session limit reached ({}) — rejecting {}", max_sessions_, imsi);
    //    return false;
    //}

    // Создаём новую сессию
    StoredSession new_s{ imsi, now, expires };
    store_->save_session(new_s);
    cdr_.write({ now, imsi, "created" });
    spdlog::info("Session created for IMSI {}, expires at {}", imsi, expires);

    // Уведомляем очистку (чтобы не ждать полного интервала)
    cv_.notify_one();
    return true;
}

bool SessionManager::is_active(const std::string& imsi) const {
    std::lock_guard<std::mutex> lk(mtx_);
    auto all = store_->load_sessions(now_str());
    return std::any_of(all.begin(), all.end(),
                       [&](auto& s){ return s.imsi == imsi; });
}

void SessionManager::offload_rate(size_t sessions_per_sec) {
    size_t removed = 0;
    while (removed < sessions_per_sec) {
        StoredSession s;
        {
            std::lock_guard<std::mutex> lk(mtx_);
            auto all = store_->load_sessions(now_str());
            if (all.empty()) break;
            s = all.front();
        }
        {
            std::lock_guard<std::mutex> lk(mtx_);
            expire_session_locked(s.imsi);
        }
        ++removed;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / sessions_per_sec));
    }
}

void SessionManager::graceful_stop(size_t rate) {
    spdlog::info("Graceful shutdown: offloading all sessions at {} per sec", rate);
    while (true) {
        std::vector<StoredSession> all;
        {
            std::lock_guard<std::mutex> lk(mtx_);
            all = store_->load_sessions(now_str());
        }
        if (all.empty()) break;
        offload_rate(rate);
    }
}

void SessionManager::expire_session_locked(const std::string& imsi) {
    store_->delete_session(imsi);
    cdr_.write({ now_str(), imsi, "expired" });
    spdlog::info("Session expired for IMSI {}", imsi);
}

void SessionManager::cleaner_loop() {
    while (!stop_) {
        auto now = now_str();

        // 1) Сначала получаем список строго просроченных — чтобы написать CDR
        auto expired = store_->load_expired_sessions(now);
        for (auto& s : expired) {
            cdr_.write({ now, s.imsi, "expired" });
            spdlog::info("Session expired for IMSI {}", s.imsi);
        }

        // 2) Потом удаляем их из хранилища одним запросом/итерацией
        store_->cleanup_expired_sessions(now);

        // 3) Ждём до следующей итерации
        std::unique_lock<std::mutex> lk(mtx_);
        cv_.wait_for(lk, std::chrono::seconds(1));
    }
}


} // namespace pgw
