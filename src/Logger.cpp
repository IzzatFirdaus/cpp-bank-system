// Implementation of bank::Logger — append-only, thread-safe, failure-reporting.
#include "bank/Logger.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <system_error>
#include <utility>

namespace bank {

Logger& Logger::instance() {
    static Logger logger("log.txt");
    return logger;
}

Logger::Logger(std::filesystem::path path)
    : path_(std::move(path)),
      logStream_(path_, std::ios::app) {
    if (!logStream_.is_open()) {
        std::cerr << "bank::Logger: failed to open " << path_
                  << ": " << std::error_code{}.message() << "\n";
    }
}

void Logger::log(const std::string& accountNumber,
                 const std::string& action,
                 double amount) {
    std::lock_guard<std::mutex> lock(ioMutex_);
    if (!logStream_.is_open()) {
        return;
    }

    const auto now = std::chrono::system_clock::now();
    const auto tt  = std::chrono::system_clock::to_time_t(now);
    std::tm    tmBuf{};
    localtime_s(&tmBuf, &tt);

    logStream_ << std::put_time(&tmBuf, "%Y-%m-%dT%H:%M:%S")
               << " Account: " << accountNumber
               << " | Action: " << action
               << " | Amount: $"
               << std::fixed << std::setprecision(2) << amount << "\n";
    logStream_.flush();

    if (logStream_.fail()) {
        std::cerr << "bank::Logger: write failed for " << path_
                  << ": " << std::error_code{}.message() << "\n";
    }
}

} // namespace bank