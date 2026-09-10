# Product

<!-- impeccable:product-schema 1 -->

## Platform

native

## Stack

C++20 with g++ (GCC), standard library only. Single-command build (`g++ -std=c++20 src/main.cpp -Iinclude -o bank_app.exe`). No build system, no external dependencies.

## Users

**Primary:** The developer, building for personal learning and portfolio. Deepening C++ skills in OOP, smart pointers, and transaction logging through a hands-on project.

**Secondary audiences (PRD-documented):**
- C++ learners exploring idiomatic code with RAII, virtual dispatch, and STL containers.
- Interview candidates needing a clean codebase to discuss OOP design choices.
- Course instructors / TAs needing a reproducible class-hierarchy example.
- Hobbyist contributors extending the system with new account types or features.

## Product Purpose

A small, portable C++ console application demonstrating how a real-world banking ledger can be modeled with object-oriented design, modern memory management, and durable transaction logging. Exists as both a learning artifact and a seed system designed to grow toward a multi-account, multi-product ledger without rewrites.

Success means: a clean build in one command, a deterministic demo that matches documented output, and the ability to add a new account type in new files only.

## Positioning

A deliberately minimal, zero-dependency C++ banking system that teaches OOP through production-grade patterns (smart pointers, append-only logging, invariant enforcement) rather than toy examples. Distinguished by its commitment to extensibility without rewrites — adding a new product variant requires no edits to existing source files.

## Operating Context

- **Build:** Single `g++` command or VS Code build task (Ctrl+Shift+B).
- **Run:** Execute the binary; it runs a deterministic demo (deposit → withdraw → apply interest → print results).
- **Output:** Console output (banner, balances, confirmation) and `log.txt` (append-only audit trail).
- **Verification:** Inspect `log.txt` to confirm transaction records match expected format.
- **Iteration:** Edit source, rebuild, run. No hot-reload, no interactive prompts in v1.

## Capabilities and Constraints

**Current capabilities (v1):**
- Create an account with unique ID and opening balance.
- Deposit and withdraw with positive-amount and sufficient-funds validation.
- SavingsAccount with annual interest rate and one-shot `applyInterest()`.
- Append-only transaction logging to `log.txt` with human-readable format.
- Deterministic demo flow — same input produces same output every run.

**Technical constraints:**
- C++20 only; no C++23 syntax.
- No external dependencies; standard library only.
- No raw owning pointers; `std::unique_ptr` / `std::shared_ptr` required.
- `balance >= 0` is a hard invariant; never bypassed.
- `log.txt` is append-only; never truncated or regenerated.
- No exceptions for control flow; validation returns `bool`, I/O failures warn to `stderr`.
- Public API is additive within v1.x; removing or renaming requires version bump discussion.

**Planned growth (beyond v1):**
- CheckingAccount with overdraft limit (v1.1).
- Interactive CLI menu for account operations (v1.1+).
- Timestamped log lines, currency formatting, input validation hardening (v1.1).
- Authentication, transfers, multi-account management, database persistence (v2+).

## Brand Commitments

Open to new identity. Current working name is "Bank Management System" (from README). No logo, voice, or visual identity established. The product operates in a terminal — branding lives in naming, documentation tone, and code quality rather than visual design.

## Evidence on Hand

- **Working codebase:** `Account.h`, `SavingsAccount.h`, `main.cpp` at root; `include/bank/` and `src/` with full implementations (`Account.cpp`, `Bank.cpp`, `Logger.cpp`).
- **Documentation:** `PRD.md` (full requirements), `ARCHITECTURE.md` (technical spec), `ARCHITECTURE-ESSENTIALS.md` (fast-load + stress test), `README.md` (user-facing docs).
- **Demo output:** `log.txt` shows three transaction lines (DEPOSIT $150, WITHDRAW $50, INTEREST_ADD $15).
- **Build infrastructure:** `.vscode/tasks.json` with g++ build task.
- **Absences:** No tests implemented (only reserved `tests/` directory with a stub). No CI/CD. No packaging or distribution setup.

## Product Principles

1. **Learning through clean code.** Every design choice should be explainable in under 90 seconds. If a reader can't state the invariants after reading a header, the code isn't done.
2. **Extensibility without surgery.** Adding a new account type or feature must never require editing existing source files. New behavior lives in new files.
3. **Zero dependencies.** The standard library is the ceiling. External dependencies require explicit justification and documentation.
4. **Deterministic demo.** The same build, run on any platform, must produce identical console output and `log.txt` content.
5. **Balance is sacred.** The `balance >= 0` invariant is non-negotiable. No temporary bypasses, no debug-mode exceptions.
