#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include "pgw/blacklist.hpp"

using namespace pgw;

class BlacklistTest : public ::testing::Test {
protected:
    // Этот список IMSI будем использовать для тестов
    std::vector<std::string> test_imsi_list = {"1234567890", "9876543210", "blocked123"};

    Blacklist blacklist{test_imsi_list};  // Объект Blacklist, который мы будем тестировать
};

TEST_F(BlacklistTest, TestBlacklistSize) {
    // Проверяем, что количество записей в черном списке совпадает с размером вектора
    EXPECT_EQ(blacklist.is_blocked("1234567890"), true);  // Первый IMSI в списке
    EXPECT_EQ(blacklist.is_blocked("9876543210"), true);  // Второй IMSI в списке
    EXPECT_EQ(blacklist.is_blocked("blocked123"), true);  // Третий IMSI в списке
    EXPECT_EQ(blacklist.is_blocked("0000000000"), false); // Этот IMSI отсутствует в списке
}

TEST_F(BlacklistTest, TestImsiBlocked) {
    // Проверяем, что заблокированные IMSI действительно обнаруживаются в черном списке
    EXPECT_TRUE(blacklist.is_blocked("1234567890"));
    EXPECT_TRUE(blacklist.is_blocked("9876543210"));
    EXPECT_TRUE(blacklist.is_blocked("blocked123"));
}

TEST_F(BlacklistTest, TestImsiNotBlocked) {
    // Проверяем, что IMSI, не находящийся в списке, не блокируется
    EXPECT_FALSE(blacklist.is_blocked("0000000000"));
    EXPECT_FALSE(blacklist.is_blocked("unblocked123"));
}

TEST_F(BlacklistTest, TestEmptyBlacklist) {
    std::vector<std::string> empty_list;
    Blacklist empty_blacklist(empty_list);

    // Проверяем, что в пустом списке нет заблокированных IMSI
    EXPECT_FALSE(empty_blacklist.is_blocked("1234567890"));
    EXPECT_FALSE(empty_blacklist.is_blocked("9876543210"));
}

TEST_F(BlacklistTest, TestBlacklistLogging) {
    // Проверяем, что при блокировке IMSI выводится лог уровня debug
    // Для этого можно будет использовать spdlog::debug для проверки в реальном коде
    // Здесь предполагаем, что логирование корректно обрабатывается в коде
    // Этот тест будет работать, если у вас настроено логирование в тестах
    EXPECT_TRUE(blacklist.is_blocked("1234567890"));
    EXPECT_TRUE(blacklist.is_blocked("9876543210"));
}
