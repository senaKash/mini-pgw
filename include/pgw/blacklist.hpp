// include/pgw/blacklist.hpp

#pragma once

#include <string>
#include <unordered_set>
#include <vector>

namespace pgw {

/**
 * Класс для управления чёрным списком IMSI.
 * Загружает список IMSI и предоставляет методы для проверки блокировки.
 */
class Blacklist {
public:
    /** 
     * Конструктор, инициализирующий чёрный список из вектора IMSI.
     * 
     * @param imsi_list Вектор строк, содержащий IMSI для добавления в чёрный список.
     */
    explicit Blacklist(const std::vector<std::string>& imsi_list);

    /** 
     * Проверяет, находится ли заданный IMSI в чёрном списке.
     * 
     * @param imsi IMSI, который нужно проверить.
     * @return true, если IMSI найден в чёрном списке, иначе false.
     */
    bool is_blocked(const std::string& imsi) const noexcept;

private:
    /** Множество для хранения IMSI, находящихся в чёрном списке. */
    std::unordered_set<std::string> blocked_;
};

} // namespace pgw
