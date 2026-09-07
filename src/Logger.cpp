// Stub translation unit for bank::Logger. Full implementation in v1.1.
#include "bank/Logger.h"

namespace bank {

Logger::Logger(std::filesystem::path path) : path_(std::move(path)) {}

void Logger::log(const std::string& accountNumber,
                 const std::string& action,
                 double amount) {
    // Intentionally empty in v1; see ARCHITECTURE.md §4.4.
    (void)accountNumber;
    (void)action;
    (void)amount;
}

} // namespace bank