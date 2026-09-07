#ifndef BANK_BANK_H
#define BANK_BANK_H

#include <memory>
#include <string>
#include <vector>

#include "bank/Account.h"

namespace bank {

// Facade owning a collection of accounts. Stub: signatures only.
// Scheduled for v1.1 alongside the CLI menu work.
class Bank {
public:
    Bank() = default;

    void                  addAccount(std::unique_ptr<Account> a);
    Account*              find(const std::string& accountNumber) const;
    std::size_t           size() const noexcept;

private:
    std::vector<std::unique_ptr<Account>> accounts_;
};

} // namespace bank

#endif