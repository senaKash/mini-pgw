#include <gtest/gtest.h>
#include "pgw/in_memory_session_store.hpp"
#include "pgw/session_store.hpp"

using namespace pgw;

// Создаем фиктивные данные сессий
StoredSession make_session(const std::string& imsi, const std::string& expires_at) {
    StoredSession session;
    session.imsi = imsi;  // Прямо передаем строку
    session.expires_at = expires_at;
    return session;
}

// Тестируем добавление и удаление сессий в памяти
TEST(InMemorySessionStoreTest_MEMORY, SaveAndDeleteSession_MEMORY) {
    InMemorySessionStore store;
    
    // Сессия с некоторым IMSI и датой истечения
    StoredSession session1 = make_session("123456789012345", "2023-12-31");
    StoredSession session2 = make_session("987654321012345", "2022-01-01");

    // Сохраняем сессии
    EXPECT_TRUE(store.save_session(session1));
    EXPECT_TRUE(store.save_session(session2));

    // Проверяем, что сессии существуют
    EXPECT_TRUE(store.session_exists("123456789012345"));
    EXPECT_TRUE(store.session_exists("987654321012345"));

    // Удаляем одну сессию и проверяем
    EXPECT_TRUE(store.delete_session("123456789012345"));
    EXPECT_FALSE(store.session_exists("123456789012345"));
    EXPECT_TRUE(store.session_exists("987654321012345"));

    // Удаляем последнюю сессию и проверяем
    EXPECT_TRUE(store.delete_session("987654321012345"));
    EXPECT_FALSE(store.session_exists("987654321012345"));
}

TEST(InMemorySessionStoreTest_MEMORY, LoadActiveSessions_MEMORY) {
    InMemorySessionStore store;

    // Добавляем несколько сессий с разными сроками действия
    store.save_session(make_session("123456789012345", "2023-12-31"));
    store.save_session(make_session("987654321012345", "2022-01-01"));
    store.save_session(make_session("111223344556677", "2024-01-01"));

    // Загружаем активные сессии для даты "2023-01-01"
    auto active_sessions = store.load_sessions("2023-01-01");

    // Проверяем, что в списке активных сессий только те, что не просрочены
    ASSERT_EQ(active_sessions.size(), 2);
    
    // Порядок не гарантируется, поэтому проверяем только содержимое
    std::vector<std::string> expected_imsi = {"123456789012345", "111223344556677"};
    std::vector<std::string> actual_imsi;
    for (const auto& session : active_sessions) {
        actual_imsi.push_back(session.imsi);  // Теперь это строка
    }

    // Сравниваем наборы имси (без учета порядка)
    std::sort(expected_imsi.begin(), expected_imsi.end());
    std::sort(actual_imsi.begin(), actual_imsi.end());
    
    EXPECT_EQ(expected_imsi, actual_imsi);
}

// Тестируем загрузку просроченных сессий в памяти
TEST(InMemorySessionStoreTest_MEMORY, LoadExpiredSessions_MEMORY) {
    InMemorySessionStore store;

    // Добавляем несколько сессий с разными сроками действия
    store.save_session(make_session("123456789012345", "2023-12-31"));
    store.save_session(make_session("987654321012345", "2022-01-01"));
    store.save_session(make_session("111223344556677", "2024-01-01"));

    // Загружаем просроченные сессии для даты "2023-01-01"
    auto expired_sessions = store.load_expired_sessions("2023-01-01");

    // Проверяем, что в списке просроченных сессий только те, что просрочены
    ASSERT_EQ(expired_sessions.size(), 1);
    EXPECT_EQ(expired_sessions[0].imsi, "987654321012345");
}

// Тестируем удаление просроченных сессий в памяти
TEST(InMemorySessionStoreTest_MEMORY, CleanupExpiredSessions_MEMORY) {
    InMemorySessionStore store;

    // Добавляем несколько сессий с разными сроками действия
    store.save_session(make_session("123456789012345", "2023-12-31"));
    store.save_session(make_session("987654321012345", "2022-01-01"));
    store.save_session(make_session("111223344556677", "2024-01-01"));

    // Убираем просроченные сессии для даты "2023-01-01"
    store.cleanup_expired_sessions("2023-01-01");

    // Проверяем, что остались только актуальные сессии
    EXPECT_TRUE(store.session_exists("123456789012345"));
    EXPECT_FALSE(store.session_exists("987654321012345"));
    EXPECT_TRUE(store.session_exists("111223344556677"));
}
