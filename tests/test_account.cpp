// Reserved. Use header-only assertions until a framework is chosen.
// See ARCHITECTURE.md §1 and AGENTS.md §8.

// #include "bank/Account.h"
// static_assert exercises:
//   - default-constructibility is NOT required (no default ctor)
//   - deposit(amount <= 0) is a no-op
//   - withdraw(amount > balance) returns false