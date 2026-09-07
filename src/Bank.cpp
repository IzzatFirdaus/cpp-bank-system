// Stub translation unit for bank::Bank. Full implementation in v1.1.
#include "bank/Bank.h"

namespace bank {

void Bank::addAccount(std::unique_ptr<Account> a) {
    accounts_.push_back(std::move(a));
}

Account* Bank::find(const std::string& accountNumber) const {
    for (const auto& a : accounts_) {
        if (a->getAccountNumber() == accountNumber) return a.get();
    }
    return nullptr;
}

std::size_t Bank::size() const noexcept {
    return accounts_.size();
}

} // namespace bank