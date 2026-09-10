#ifndef BANK_SAVINGS_ACCOUNT_H
#define BANK_SAVINGS_ACCOUNT_H

#include <cmath>

#include "bank/Account.h"

namespace bank {

// Savings account: adds an annual interest rate and a one-shot
// applyInterest() action. Invariant: interestRate >= 0.0 (clamped).
class SavingsAccount : public Account {
private:
    double interestRate;

public:
    SavingsAccount(std::string accNum, double initialBalance, double rate)
        : Account(std::move(accNum), initialBalance),
          interestRate(std::isfinite(rate) && rate >= 0.0 ? rate : 0.0) {}

    double getInterestRate() const noexcept { return interestRate; }

    // Credits balance * (rate / 100) and logs INTEREST_ADD only when the
    // interest amount is finite and positive.
    void applyInterest() {
        const double interest = balance * (interestRate / 100.0);
        if (std::isfinite(interest) && interest > 0.0) {
            balance += interest;
            saveTransaction("INTEREST_ADD", interest);
        }
    }
};

} // namespace bank

#endif