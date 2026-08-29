#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>
#include <fstream>
#include <iostream>

class Account {
protected:
    std::string accountNumber;
    double balance;

public:
    Account(std::string accNum, double initialBalance) 
        : accountNumber(accNum), balance(initialBalance) {}

    virtual ~Account() = default;

    virtual void deposit(double amount) {
        if (amount > 0) balance += amount;
    }

    virtual bool withdraw(double amount) {
        if (amount > 0 && amount <= balance) {
            balance -= amount;
            return true;
        }
        return false;
    }

    void saveTransaction(const std::string& type, double amount) {
        std::ofstream file("log.txt", std::ios::app);
        if (file.is_open()) {
            file << "Account: " << accountNumber << " | Action: " << type << " | Amount: $" << amount << "\n";
        }
    }

    double getBalance() const { return balance; }
    std::string getAccountNumber() const { return accountNumber; }
};

#endif