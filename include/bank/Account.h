#ifndef BANK_ACCOUNT_H
#define BANK_ACCOUNT_H

#include <string>

namespace bank {

// Base account: identity, balance, deposit, withdraw, transactional logging.
// Invariants:
//   - balance >= 0.0 after every accepted operation
//   - accountNumber contains no '\n', '\r', or '|' (sanitized at construction)
//   - deposit/withdraw log ONLY on accepted state changes
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