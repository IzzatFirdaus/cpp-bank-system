#ifndef BANK_LOGGER_H
#define BANK_LOGGER_H

#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>

namespace bank {

// Append-only sink for log.txt. Process-wide singleton: the file is opened
// once (std::ios::app) on first use and never truncated. Writes are
// serialized with a mutex; failures are reported to stderr, never thrown.
class Logger {
public:
    static Logger& instance();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // Format: <ISO-8601 timestamp> Account: <id> | Action: <TYPE> | Amount: $<value>
    void log(const std::string& accountNumber,
             const std::string& action,
             double amount);

private:
    explicit Logger(std::filesystem::path path);

    std::filesystem::path path_;
    std::ofstream         logStream_;
    std::mutex            ioMutex_;
};

} // namespace bank

#endif