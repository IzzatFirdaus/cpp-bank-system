// Implementation of bank::Account.
#include "bank/Account.h"

#include <cmath>
#include <utility>

#include "bank/Logger.h"

namespace bank {

namespace {

// Strip newlines and the log delimiter from an account number.
// Falls back to "UNKNOWN_ACC" if the result is empty.
std::string sanitizeAccountNumber(std::string accNum) {
    std::string clean;
    clean.reserve(accNum.size());
    for (char c : accNum) {
        if (c != '\n' && c != '\r' && c != '|') {
            clean.push_back(c);
        }
    }
    return clean.empty() ? "UNKNOWN_ACC" : clean;
}

} // namespace

Account::Account(std::string accNum, double initialBalance)
    : accountNumber(sanitizeAccountNumber(std::move(accNum))),
      balance(std::isfinite(initialBalance) && initialBalance > 0.0
                  ? initialBalance
                  : 0.0) {}

void Account::deposit(double amount) {
    if (std::isfinite(amount) && amount > 0.0) {
        balance += amount;
        saveTransaction("DEPOSIT", amount);
    }
}

bool Account::withdraw(double amount) {
    if (std::isfinite(amount) && amount > 0.0 && amount <= balance) {
        balance -= amount;
        saveTransaction("WITHDRAW", amount);
        return true;
    }
    return false;
}

double Account::getBalance() const noexcept {
    return balance;
}

std::string Account::getAccountNumber() const {
    return accountNumber;
}

void Account::saveTransaction(const std::string& type, double amount) {
    Logger::instance().log(accountNumber, type, amount);
}

} // namespace bank