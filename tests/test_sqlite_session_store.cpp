#include <gtest/gtest.h>
#include "pgw/sqlite_session_store.hpp"
#include "pgw/session.hpp"
#include <filesystem>

namespace fs = std::filesystem;
using namespace pgw;

class SqliteSessionStoreTest : public ::testing::Test {
protected:
    std::string db_path = "test.db";
    std::unique_ptr<SqliteSessionStore> store_;

    // Запуск перед каждым тестом
    void SetUp() override {
        // Удаляем файл базы данных перед каждым тестом, чтобы обеспечить чистоту теста
        if (fs::exists(db_path)) {
            fs::remove(db_path);
        }
        store_ = std::make_unique<SqliteSessionStore>(db_path);
    }

    // Завершение работы после каждого теста
    void TearDown() override {
        store_.reset();
        if (fs::exists(db_path)) {
            fs::remove(db_path);  // Удаляем базу данных после теста
        }
    }

    // Создание тестовой сессии
    StoredSession create_session(const std::string& imsi, const std::string& created_at, const std::string& expires_at) {
        StoredSession session;
        session.imsi = imsi;
        session.created_at = created_at;
        session.expires_at = expires_at;
        return session;
    }
};

// Тест 1: Проверка сохранения и загрузки сессий
TEST_F(SqliteSessionStoreTest, SaveAndLoadSessions) {
    StoredSession session = create_session("123456789012345", "2023-01-01 00:00:00", "2023-12-31 23:59:59");

    // Сохраняем сессию
    ASSERT_TRUE(store_->save_session(session));

    // Загружаем сессии с условием, что они не истекли
    auto loaded_sessions = store_->load_sessions("2023-06-01 00:00:00");

    // Проверяем, что сессия была загружена
    ASSERT_EQ(loaded_sessions.size(), 1);
    EXPECT_EQ(loaded_sessions[0].imsi, "123456789012345");
}

// Тест 2: Проверка загрузки просроченных сессий
TEST_F(SqliteSessionStoreTest, LoadExpiredSessions) {
    // Создаем сессии с разными сроками действия
    store_->save_session(create_session("123456789012345", "2023-01-01 00:00:00", "2023-12-31 23:59:59"));
    store_->save_session(create_session("987654321012345", "2022-01-01 00:00:00", "2022-12-31 23:59:59"));

    // Загружаем просроченные сессии на 2023-01-01
    auto expired_sessions = store_->load_expired_sessions("2023-01-01 00:00:00");

    // Проверяем, что загружена только одна просроченная сессия
    ASSERT_EQ(expired_sessions.size(), 1);
    EXPECT_EQ(expired_sessions[0].imsi, "987654321012345");
}

// Тест 3: Проверка удаления сессий
TEST_F(SqliteSessionStoreTest, DeleteSession) {
    // Создаем сессии
    store_->save_session(create_session("123456789012345", "2023-01-01 00:00:00", "2023-12-31 23:59:59"));
    store_->save_session(create_session("987654321012345", "2022-01-01 00:00:00", "2022-12-31 23:59:59"));

    // Удаляем одну сессию
    ASSERT_TRUE(store_->delete_session("123456789012345"));

    // Проверяем, что сессия была удалена
    EXPECT_FALSE(store_->session_exists("123456789012345"));
    EXPECT_TRUE(store_->session_exists("987654321012345"));
}

// Тест 4: Проверка очистки просроченных сессий
TEST_F(SqliteSessionStoreTest, CleanupExpiredSessions) {
    // Создаем сессии
    store_->save_session(create_session("123456789012345", "2023-01-01 00:00:00", "2023-12-31 23:59:59"));
    store_->save_session(create_session("987654321012345", "2022-01-01 00:00:00", "2022-12-31 23:59:59"));

    // Очистка просроченных сессий на 2023-01-01
    store_->cleanup_expired_sessions("2023-01-01 00:00:00");

    // Проверяем, что осталась только актуальная сессия
    EXPECT_TRUE(store_->session_exists("123456789012345"));
    EXPECT_FALSE(store_->session_exists("987654321012345"));
}

