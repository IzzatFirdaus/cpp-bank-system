#ifndef BANK_CHECKING_ACCOUNT_H
#define BANK_CHECKING_ACCOUNT_H

#include "bank/Account.h"

namespace bank {

// Stub only. Scheduled for v1.1.
// Invariant: balance may go down to -overdraftLimit, no further.
class CheckingAccount : public Account {
private:
    double overdraftLimit;

public:
    CheckingAccount(std::string accNum, double initialBalance, double overdraftLimit);
    bool   withdraw(double amount) override;
};

} // namespace bank

#endif