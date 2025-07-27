// src/server/cdr_writer.cpp
#include "pgw/cdr_writer.hpp"
#include <fstream>
#include <spdlog/spdlog.h>
#include <stdexcept>  // для std::invalid_argument

namespace pgw {

CdrWriter::CdrWriter(std::string file_path)
{
    if (file_path.empty()) {
        throw std::invalid_argument("File path cannot be empty");
    }
    path_ = std::move(file_path);
    writer_thread_ = std::thread(&CdrWriter::writer_loop, this);
}

CdrWriter::~CdrWriter() {
    {
        std::lock_guard<std::mutex> lk(mtx_);
        stop_ = true;
        cv_.notify_all();
    }
    writer_thread_.join();
}

void CdrWriter::write(const CdrRecord& rec) {
    {
        std::lock_guard<std::mutex> lk(mtx_);
        queue_.push(rec);
    }
    cv_.notify_one();
}

void CdrWriter::writer_loop() {
    std::ofstream out(path_, std::ios::app);
    if (!out.is_open()) {
        spdlog::critical("Failed to open CDR file: {}", path_);
        return;
    }
    while (true) {
        std::unique_lock<std::mutex> lk(mtx_);
        cv_.wait(lk, [&]{ return stop_ || !queue_.empty(); });
        if (stop_ && queue_.empty())
            break;

        CdrRecord rec = std::move(queue_.front());
        queue_.pop();
        lk.unlock();

        // Запись строки: timestamp,imsi,action\n
        out << rec.timestamp << "," 
            << rec.imsi      << "," 
            << rec.action    << "\n";
        out.flush();
    }
    out.close();
}

} // namespace pgw
