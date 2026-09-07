#ifndef BANK_MENU_H
#define BANK_MENU_H

#include "bank/Bank.h"

namespace bank {

// Interactive CLI menu. Stub: signatures only.
// Scheduled for v1.1; v1 keeps the deterministic demo in main.cpp.
class Menu {
public:
    explicit Menu(Bank& bank);

    int run();

private:
    Bank& bank_;
};

} // namespace bank

#endif