# Product Requirements Document — cpp-bank-system

## 1. Purpose

`cpp-bank-system` is a small, portable C++ console application that demonstrates how a real-world banking ledger could be modeled with object-oriented design, modern memory management, and durable transaction logging. It exists as both:

1. A **learning artifact** — clean, readable code that teaches inheritance, polymorphism, smart pointers, and file I/O.
2. A **seed system** — a deliberately small surface that can grow toward a multi-account, multi-product ledger without rewrites.

This document defines *what* the system does and *why*. Technical decisions live in `ARCHITECTURE.md`.

## 2. Target Audience

| Audience | Why they care |
| --- | --- |
| C++ learners | A short, idiomatic codebase that exercises RAII, virtual dispatch, and STL containers. |
| Interview candidates | A reference for explaining OOP design choices in technical interviews. |
| Course instructors / TAs | A reproducible example of class hierarchies and file-based persistence. |
| Hobbyist contributors | A starting point to add checking accounts, transfers, authentication, or a CLI menu. |

Out of scope for the current release: retail banking customers, regulators, mobile/web clients.

## 3. Personas

- **Lina, CS student.** Opens the repo, expects a one-command build, runs the program, and inspects `log.txt` to confirm the demo behaves as the README claims.
- **Marco, interview prep.** Reads the headers first to see the inheritance structure; wants the public API to be small and the invariants obvious.
- **Priya, hobbyist.** Wants to add a `CheckingAccount` without breaking existing behavior; expects additive, not invasive, changes.

## 4. Functional Requirements

### 4.1 Account lifecycle
- **FR-1.** Create an account with a unique identifier and an opening balance.
- **FR-2.** Retrieve the current balance and the account identifier.
- **FR-3.** Support deposits and withdrawals with positive-amount and sufficient-funds validation.
- **FR-4.** Persist a human-readable record of every accepted transaction to `log.txt` (append mode).

### 4.2 Product variants
- **FR-5.** `SavingsAccount` extends `Account` and adds an `interestRate` plus a one-shot `applyInterest()` action that credits accrued interest and logs it.

### 4.3 Program behavior
- **FR-6.** The executable prints a startup banner, the account number, the initial balance, performs a deterministic sequence of operations, prints the final balance, and confirms that logging occurred.
- **FR-7.** All operations complete deterministically for the demo seed; no interactive prompts in the default executable.

## 5. Non-Functional Requirements

| Category | Requirement |
| --- | --- |
| Language standard | C++20 |
| Compiler | `g++` (GCC) — same toolchain as the existing VS Code task |
| Memory | No raw owning pointers; use `std::unique_ptr` / `std::shared_ptr` where ownership is shared |
| Determinism | Same input → same log output |
| Portability | Build and run on Windows (primary), Linux, macOS without source changes |
| Build | Single command, no external dependencies beyond the standard library |
| Logging | Plain-text, line-oriented, safe to open with any editor |
| Failure safety | A failed I/O operation must not corrupt the in-memory balance |

## 6. User Flows

### 6.1 Demo flow (current scope)
1. User invokes the built executable.
2. Program constructs a single `SavingsAccount` ("ACC-1001", balance $500, rate 2.5%).
3. Program prints banner and initial state.
4. Program performs: deposit $150 → withdraw $50 → apply interest.
5. Program prints final balance and a "logged" confirmation.
6. `log.txt` now contains four lines corresponding to the actions above.

### 6.2 Future flow (CLI menu, out of scope for v1, referenced for context)
1. User launches the executable with no arguments.
2. A menu lists: create account, deposit, withdraw, apply interest, list accounts, quit.
3. Each action echoes the result and writes a log line.

## 7. Edge-Case Behavior

| Case | Required behavior |
| --- | --- |
| Deposit of zero or negative amount | Reject silently (no balance change, no log line) |
| Withdrawal of zero or negative amount | Reject and return `false`; no log line |
| Withdrawal exceeding balance | Reject and return `false`; no log line |
| `log.txt` not writable | Print a warning to `stderr`; in-memory state remains correct |
| `log.txt` missing | Create on first write (append mode creates the file) |
| Multiple program runs | Each run appends to `log.txt`; no truncation |
| Empty account number | Constructor accepts; current scope does not enforce uniqueness |

## 8. Success Metrics

The PRD is considered met when **all** of the following hold:

- A clean checkout builds with the documented single command and produces `bank_app.exe` (or platform equivalent).
- Running the executable matches the demo flow in §6.1 and produces the expected `log.txt` content.
- Adding a new account type (e.g., `CheckingAccount`) requires changes only in new files plus a one-line registration; no existing source file must be modified.
- A reviewer can read `Account.h` end-to-end in under 90 seconds and state the invariants.

## 9. Out of Scope (v1)

- Interactive CLI, password / PIN authentication, multi-user accounts.
- Network APIs, databases, persistent account storage beyond `log.txt`.
- Currency localization, multi-currency, overdraft fees.
- Threading or concurrent transactions.
- Internationalization.

## 10. Open Questions

- Should the program treat `log.txt` as authoritative state, or as an audit trail only? (Current scope: audit trail only.)
- Should `applyInterest()` be idempotent within a day? (Deferred — single-shot semantics for v1.)