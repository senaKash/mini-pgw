// src/server/sqlite_session_store.cpp
#include "pgw/sqlite_session_store.hpp"
#include <spdlog/spdlog.h>

namespace pgw {

SqliteSessionStore::SqliteSessionStore(const std::string& db_path) {
    if (sqlite3_open(db_path.c_str(), &db_) != SQLITE_OK) {
        spdlog::error("Failed to open session database: {}", sqlite3_errmsg(db_));
        db_ = nullptr;
    } else {
        init_schema();
    }
}

SqliteSessionStore::~SqliteSessionStore() {
    if (db_) sqlite3_close(db_);
}

void SqliteSessionStore::init_schema() {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS sessions ("
        "imsi TEXT PRIMARY KEY,"
        "created_at TEXT,"
        "expires_at TEXT);";
    char* err_msg = nullptr;
    if (sqlite3_exec(db_, sql, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        spdlog::error("Failed to create schema: {}", err_msg);
        sqlite3_free(err_msg);
    }
}

std::vector<StoredSession> SqliteSessionStore::load_expired_sessions(const std::string& now) {
    std::lock_guard<std::mutex> lock(mtx_);
    std::vector<StoredSession> result;
    const char* sql =
        "SELECT imsi, created_at, expires_at "
        "FROM sessions "
        "WHERE expires_at <= ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return result;
    sqlite3_bind_text(stmt, 1, now.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        StoredSession s;
        s.imsi       = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        s.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        s.expires_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        result.push_back(std::move(s));
    }
    sqlite3_finalize(stmt);
    return result;
}


bool SqliteSessionStore::save_session(const StoredSession& s) {
    return save_session(s.imsi, s.created_at, s.expires_at);
}

bool SqliteSessionStore::save_session(const std::string& imsi,
                                      const std::string& created_at,
                                      const std::string& expires_at) {
    std::lock_guard<std::mutex> lock(mtx_);
    const char* sql = "REPLACE INTO sessions (imsi, created_at, expires_at) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, imsi.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, created_at.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, expires_at.c_str(), -1, SQLITE_TRANSIENT);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool SqliteSessionStore::delete_session(const std::string& imsi) {
    std::lock_guard<std::mutex> lock(mtx_);
    const char* sql = "DELETE FROM sessions WHERE imsi = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, imsi.c_str(), -1, SQLITE_TRANSIENT);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool SqliteSessionStore::session_exists(const std::string& imsi) {
    std::lock_guard<std::mutex> lock(mtx_);
    const char* sql = "SELECT 1 FROM sessions WHERE imsi = ? LIMIT 1;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, imsi.c_str(), -1, SQLITE_TRANSIENT);

    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return exists;
}

void SqliteSessionStore::cleanup_expired_sessions(const std::string& now) {
    std::lock_guard<std::mutex> lock(mtx_);
    const char* sql = "DELETE FROM sessions WHERE expires_at <= ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return;

    sqlite3_bind_text(stmt, 1, now.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::vector<StoredSession> SqliteSessionStore::load_sessions(const std::string& now) {
    std::lock_guard<std::mutex> lock(mtx_);
    std::vector<StoredSession> result;
    const char* sql = "SELECT imsi, created_at, expires_at FROM sessions WHERE expires_at > ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return result;
    }

    sqlite3_bind_text(stmt, 1, now.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        StoredSession session;
        session.imsi = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        session.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        session.expires_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        result.push_back(std::move(session));
    }

    sqlite3_finalize(stmt);
    return result;
}

} // namespace pgw
