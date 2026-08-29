#ifndef SAVINGS_ACCOUNT_H
#define SAVINGS_ACCOUNT_H

#include "Account.h"

class SavingsAccount : public Account {
private:
    double interestRate;

public:
    SavingsAccount(std::string accNum, double initialBalance, double rate)
        : Account(accNum, initialBalance), interestRate(rate) {}

    void applyInterest() {
        double interest = balance * (interestRate / 100.0);
        deposit(interest);
        saveTransaction("INTEREST_ADD", interest);
    }
};

#endif