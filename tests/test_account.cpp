// Zero-dependency test suite for cpp-bank-system (AGENTS.md §8).
// Build: g++ -std=c++20 tests/test_account.cpp src/Account.cpp src/Logger.cpp -Iinclude -o tests/test_account.exe
// Run:   .\tests\test_account.exe
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

#include "bank/Account.h"
#include "bank/SavingsAccount.h"

namespace {

// --- helpers -------------------------------------------------------------

int failures = 0;

void check(bool cond, const char* what) {
    if (!cond) {
        ++failures;
        std::cerr << "FAIL: " << what << "\n";
    }
}

// Count occurrences of a substring in log.txt.
std::size_t countInLog(const std::string& needle) {
    std::ifstream in("log.txt");
    std::size_t   count = 0;
    std::string   line;
    while (std::getline(in, line)) {
        if (line.find(needle) != std::string::npos) {
            ++count;
        }
    }
    return count;
}

} // namespace

int main() {
    // --- construction: negative / non-finite opening balance clamps to 0 ---
    {
        bank::Account neg("NEG", -100.0);
        check(neg.getBalance() == 0.0, "negative opening balance clamps to 0");

        bank::Account nan("NAN", std::numeric_limits<double>::quiet_NaN());
        check(nan.getBalance() == 0.0, "NaN opening balance clamps to 0");

        bank::Account inf("INF", std::numeric_limits<double>::infinity());
        check(inf.getBalance() == 0.0, "+Inf opening balance clamps to 0");
    }

    // --- deposit: rejects NaN / +Inf / non-positive, logs nothing ---
    {
        bank::Account a("DEP", 100.0);
        a.deposit(std::numeric_limits<double>::quiet_NaN());
        check(a.getBalance() == 100.0, "deposit(NaN) is a no-op");

        a.deposit(std::numeric_limits<double>::infinity());
        check(a.getBalance() == 100.0, "deposit(+Inf) is a no-op");

        a.deposit(0.0);
        check(a.getBalance() == 100.0, "deposit(0) is a no-op");

        a.deposit(-50.0);
        check(a.getBalance() == 100.0, "deposit(negative) is a no-op");

        check(countInLog("Action: DEPOSIT") == 0,
              "rejected deposits generate NO log entries");
    }

    // --- withdraw: rejects NaN / +Inf / overdraw, logs nothing ---
    {
        bank::Account a("WD", 100.0);
        check(!a.withdraw(std::numeric_limits<double>::quiet_NaN()),
              "withdraw(NaN) returns false");
        check(!a.withdraw(std::numeric_limits<double>::infinity()),
              "withdraw(+Inf) returns false");
        check(!a.withdraw(0.0), "withdraw(0) returns false");
        check(!a.withdraw(-10.0), "withdraw(negative) returns false");
        check(!a.withdraw(200.0), "withdraw(over balance) returns false");
        check(a.getBalance() == 100.0, "balance unchanged after rejected withdrawals");

        check(countInLog("Action: WITHDRAW") == 0,
              "rejected withdrawals generate NO log entries");
    }

    // --- accepted operations log exactly once ---
    {
        bank::Account a("OK", 100.0);
        a.deposit(50.0);
        check(a.getBalance() == 150.0, "accepted deposit mutates balance");
        check(countInLog("Account: OK | Action: DEPOSIT") == 1,
              "accepted deposit logs exactly once");

        check(a.withdraw(30.0), "accepted withdrawal returns true");
        check(a.getBalance() == 120.0, "accepted withdrawal mutates balance");
        check(countInLog("Account: OK | Action: WITHDRAW") == 1,
              "accepted withdrawal logs exactly once");
    }

    // --- account number sanitization ---
    {
        bank::Account dirty("bad\nid|with\rchars", 10.0);
        check(dirty.getAccountNumber() == "badidwithchars",
              "newlines and pipes stripped from account number");

        bank::Account empty("", 10.0);
        check(empty.getAccountNumber() == "UNKNOWN_ACC",
              "empty account number falls back to UNKNOWN_ACC");
    }

    // --- SavingsAccount: negative / non-finite rate clamps to 0 ---
    {
        bank::SavingsAccount neg("SR", 100.0, -5.0);
        neg.applyInterest();
        check(neg.getBalance() == 100.0,
              "negative rate clamps to 0; applyInterest is a no-op");

        bank::SavingsAccount nan("SN", 100.0, std::numeric_limits<double>::quiet_NaN());
        nan.applyInterest();
        check(nan.getBalance() == 100.0,
              "NaN rate clamps to 0; applyInterest is a no-op");

        bank::SavingsAccount zero("SZ", 100.0, 0.0);
        zero.applyInterest();
        check(zero.getBalance() == 100.0,
              "zero rate produces no interest and no log entry");
        check(countInLog("Action: INTEREST_ADD") == 0,
              "no INTEREST_ADD logged for zero/negative rates");
    }

    // --- SavingsAccount: positive rate credits and logs once ---
    {
        bank::SavingsAccount s("SI", 500.0, 2.5);
        s.applyInterest();
        check(s.getBalance() == 512.5, "2.5% interest on 500 credits 12.5");
        check(countInLog("Account: SI | Action: INTEREST_ADD") == 1,
              "accepted interest logs exactly once");
    }

    if (failures == 0) {
        std::cout << "All tests passed.\n";
        return 0;
    }
    std::cerr << failures << " test(s) failed.\n";
    return 1;
}