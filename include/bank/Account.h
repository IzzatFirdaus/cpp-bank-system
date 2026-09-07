#ifndef BANK_ACCOUNT_H
#define BANK_ACCOUNT_H

#include <string>

namespace bank {

// Base account. Stub: signatures mirror the root-level Account.h (v1).
// Full logic intentionally lives in the root header for v1; this file
// exists to anchor the bank:: include path for future migration.
class Account {
protected:
    std::string accountNumber;
    double      balance;

public:
    explicit Account(std::string accNum, double initialBalance);
    virtual ~Account() = default;

    virtual void deposit(double amount);
    virtual bool withdraw(double amount);

    double       getBalance()      const noexcept;
    std::string  getAccountNumber() const;

protected:
    void saveTransaction(const std::string& type, double amount);
};

} // namespace bank

#endif