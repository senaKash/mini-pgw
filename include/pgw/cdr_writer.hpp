// include/pgw/cdr_write.hpp
#pragma once

#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

namespace pgw {

// Структура для представления одной записи CDR (Call Data Record)
struct CdrRecord {
    std::string timestamp;  // Временная метка записи CDR
    std::string imsi;       // IMSI абонента
    std::string action;     // Действие ("created" - создана сессия, "expired" - сессия завершена)
};

class CdrWriter {
public:
    // Конструктор, который инициализирует путь к файлу для записи CDR
    explicit CdrWriter(std::string file_path);
    
    // Деструктор, который завершает работу потока записи и очищает ресурсы
    ~CdrWriter();

    // Метод для помещения записи в очередь для асинхронной записи в файл
    void write(const CdrRecord& rec);

private:
    // Метод, который выполняет запись в файл в отдельном потоке
    void writer_loop();

    std::string                         path_;  // Путь к файлу для записи CDR
    std::thread                         writer_thread_;  // Поток для асинхронной записи в файл
    std::mutex                          mtx_;  // Мьютекс для синхронизации доступа к очереди
    std::condition_variable             cv_;   // Условная переменная для ожидания новых записей
    std::queue<CdrRecord>               queue_;  // Очередь для хранения записей CDR до их записи
    bool                                stop_{false};  // Флаг для завершения работы потока
};

} // namespace pgw
