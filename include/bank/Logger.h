#ifndef BANK_LOGGER_H
#define BANK_LOGGER_H

#include <filesystem>
#include <mutex>
#include <string>

namespace bank {

// Append-only sink for log.txt. Stub: signatures only.
class Logger {
public:
    explicit Logger(std::filesystem::path path);

    void log(const std::string& accountNumber,
             const std::string& action,
             double amount);

private:
    std::filesystem::path path_;
    std::mutex            ioMutex_;
};

} // namespace bank

#endif