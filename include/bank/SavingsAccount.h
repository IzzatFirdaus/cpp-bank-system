#ifndef BANK_SAVINGS_ACCOUNT_H
#define BANK_SAVINGS_ACCOUNT_H

#include "bank/Account.h"

namespace bank {

// Stub: signatures mirror the root-level SavingsAccount.h (v1).
// Full logic intentionally lives in the root header for v1.
class SavingsAccount : public Account {
private:
    double interestRate;

public:
    SavingsAccount(std::string accNum, double initialBalance, double rate);
    void   applyInterest();
};

} // namespace bank

#endif