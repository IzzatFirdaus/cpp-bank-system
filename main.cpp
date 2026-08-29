#include <iostream>
#include <memory>
#include "SavingsAccount.h"

int main() {
    std::unique_ptr<SavingsAccount> myAcc = std::make_unique<SavingsAccount>("ACC-1001", 500.00, 2.5);

    std::cout << "--- Smart Banking System ---\n";
    std::cout << "Account: " << myAcc->getAccountNumber() << "\n";
    std::cout << "Initial Balance: $" << myAcc->getBalance() << "\n";

    myAcc->deposit(150.00);
    myAcc->saveTransaction("DEPOSIT", 150.00);

    myAcc->withdraw(50.00);
    myAcc->saveTransaction("WITHDRAW", 50.00);

    myAcc->applyInterest();

    std::cout << "Final Balance after interest: $" << myAcc->getBalance() << "\n";
    std::cout << "Transactions logged to log.txt.\n";

    return 0;
}