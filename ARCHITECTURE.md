# Architecture — cpp-bank-system

> Single source of truth for technical design. If `ARCHITECTURE.md` and `ARCHITECTURE-ESSENTIALS.md` disagree, this file wins.

## 1. Tech Stack

| Layer | Choice | Rationale |
| --- | --- | --- |
| Language | C++20 | Matches existing toolchain (`-std=c++20`); enables `std::unique_ptr`, `<filesystem>`, concepts if needed later |
| Compiler | `g++` (GCC) | Already wired into the VS Code build task |
| Build | Single `g++` invocation, no build system | Repo is intentionally simple: one command per target |
| Runtime deps | C++ standard library + `cpp-httplib` (single-header, vendored) | `cpp-httplib` provides HTTP server with zero external package managers |
| Storage | Plain-text `log.txt` (append-only) | Matches the PRD's audit-trail scope |
| Web frontend | HTML5, CSS3, vanilla JavaScript (ES6) | No framework dependencies; served as static files by the HTTP server |
| Tests (future) | Header-only assertion macros or `doctest` single-header | Keep zero-dep until the test suite actually grows |

## 2. Repository Layout

```
cpp-bank-system/
├── include/
│   ├── httplib.h                   # cpp-httplib single-header HTTP library (vendored)
│   └── bank/
│       ├── Account.h               # Base class: identity, balance, deposit, withdraw
│       ├── SavingsAccount.h        # Derived class: interest rate, applyInterest()
│       ├── CheckingAccount.h       # stub, future variant
│       ├── Logger.h                # append-only sink for log.txt
│       └── Bank.h                  # owns accounts (collection facade)
├── src/
│   ├── main.cpp                    # Console demo driver (deterministic flow)
│   ├── server.cpp                  # REST API server + static file serving
│   ├── Account.cpp                 # Account implementation
│   ├── Logger.cpp                  # Logger implementation
│   └── Bank.cpp                    # Bank facade implementation
├── public/
│   └── index.html                  # Web dashboard (served by the HTTP server)
├── tests/                          # Reserved for future tests
├── Account.h                       # v1: root-level header (legacy, unchanged public API)
├── SavingsAccount.h                # v1: root-level header (legacy)
├── main.cpp                        # v1: root-level demo driver (legacy)
├── PRD.md                          # Product requirements
├── ARCHITECTURE.md                 # This file — full technical spec
├── ARCHITECTURE-ESSENTIALS.md      # Fast-load summary + stress-test audit
├── AGENTS.md                       # Operational rules for AI contributors
├── CLAUDE.md                       # CLI quickstart for LLM tools
├── README.md                       # User-facing documentation
├── log.txt                         # Generated at runtime (gitignored)
└── .vscode/
    └── tasks.json                  # Build tasks (console + server)
```

The migration from root-level headers to `include/bank/` is staged: the root copies remain so existing README commands keep working. New code references the `bank/` namespace.

## 3. Data Model

### 3.1 In-memory types

```cpp
// bank::Account
std::string accountNumber;   // free-form identifier; uniqueness not enforced in v1
double       balance;        // >= 0 invariant

// bank::SavingsAccount : bank::Account
double       interestRate;   // annual percent, e.g. 2.5 means 2.5%
```

**Invariants**

- `balance >= 0.0` after every accepted operation.
- `interestRate` is read-only after construction (no setter in v1).
- `accountNumber` is read-only after construction.

### 3.2 Persistence model

| Aspect | Decision |
| --- | --- |
| Authoritative store | None — `log.txt` is an audit trail, not a database |
| Format | `Account: <id> \| Action: <TYPE> \| Amount: $<value>\n` |
| Mode | `std::ios::app` (creates if missing, never truncates) |
| Encoding | UTF-8, LF line endings |
| Rotation | None in v1 |

Because there is no authoritative store, the program reconstructs state from the constructor arguments on every run — by design for the demo.

## 4. Class & Component Design

### 4.1 `bank::Account` (base)

Responsibilities:
- Hold `accountNumber` and `balance`.
- Validate and apply `deposit(amount)` and `withdraw(amount)`.
- Emit a log line for each accepted action via `Logger`.

Public API (v1, unchanged from current code):

```cpp
explicit Account(std::string accNum, double initialBalance);
virtual ~Account() = default;
virtual void  deposit(double amount);          // returns void; logs accepted
virtual bool  withdraw(double amount);         // true on success
double        getBalance()      const noexcept;
std::string  getAccountNumber() const;
protected:
    void saveTransaction(const std::string& type, double amount);  // forwards to Logger
```

### 4.2 `bank::SavingsAccount : bank::Account`

Adds:
- `interestRate` (constructor argument, percentage).
- `getInterestRate()` — returns the annual interest rate (added for REST API exposure).
- `applyInterest()` — credits `balance * (rate / 100)` and logs `INTEREST_ADD`.

Does not override `deposit` / `withdraw`.

### 4.3 `bank::CheckingAccount` (scaffold-only stub)

Reserved for v1.1. Suggested shape (not implemented in this pass):

```cpp
class CheckingAccount : public Account {
public:
    CheckingAccount(std::string accNum, double initialBalance, double overdraftLimit);
    bool withdraw(double amount) override;     // allows negative up to -overdraftLimit
private:
    double overdraftLimit;
};
```

### 4.4 `bank::Logger`

A thin append-only sink extracted from `Account::saveTransaction`.

Responsibilities:
- Open `log.txt` once per process (singleton-ish, but explicit `Logger logger("log.txt");`).
- Provide `log(accountNumber, action, amount)`.
- On open failure, write a single warning to `stderr` and degrade to a no-op (no exception thrown — keeps the demo flow resilient).

Header sketch (full implementation lives in the stub during scaffold step):

```cpp
class Logger {
public:
    explicit Logger(std::filesystem::path path);
    void log(const std::string& accountNumber,
             const std::string& action,
             double amount);
private:
    std::filesystem::path path_;
};
```

### 4.5 `bank::Bank` (scaffold-only facade)

Owns a `std::vector<std::unique_ptr<Account>>`. Reserved for the multi-account flow referenced in PRD §6.2.

```cpp
class Bank {
public:
    void addAccount(std::unique_ptr<Account> a);
    Account* find(const std::string& accountNumber) const;
    std::size_t size() const noexcept;
private:
    std::vector<std::unique_ptr<Account>> accounts_;
};
```

### 4.6 `server.cpp` — REST API Server

A thin HTTP adapter that exposes the `bank::Account` domain engine as JSON endpoints. Built with `cpp-httplib` (single-header, vendored at `include/httplib.h`).

**Responsibilities:**
- Parse HTTP requests and extract JSON payloads.
- Call `Account` / `SavingsAccount` methods.
- Return JSON responses with appropriate HTTP status codes.
- Serve static files from `public/` (the web dashboard).
- Set CORS headers for local browser development.
- Listen on `http://0.0.0.0:8080` (port configurable via CLI argument).

**Does NOT contain business logic.** All validation and invariant enforcement lives in `Account` and `SavingsAccount`.

**Endpoints:**

| Method | Path | Purpose |
| --- | --- | --- |
| `GET` | `/api/account` | Returns account number, balance, interest rate as JSON |
| `POST` | `/api/deposit` | Accepts `{"amount": N}`, calls `deposit()`, returns updated balance |
| `POST` | `/api/withdraw` | Accepts `{"amount": N}`, calls `withdraw()`, returns success + balance |
| `POST` | `/api/apply-interest` | Calls `applyInterest()`, returns new balance + credited interest |
| `GET` | `/api/logs` | Reads last 50 lines of `log.txt`, returns as JSON array |

**Error handling:**
- Invalid/non-positive amounts → HTTP 400 with `{"success":false,"error":"..."}`.
- Withdrawal exceeds balance → HTTP 422 with `{"success":false,"error":"Insufficient funds.","balance":...}`.
- Server startup failure → non-zero exit code with stderr message.

**JSON helpers:** Minimal `json::escape()`, `json::fmt()`, and `json::get_number()` functions are defined locally in `server.cpp`. No external JSON library is used.

### 4.7 `main.cpp`

For v1, continues to run the deterministic demo (deposit / withdraw / applyInterest). For v1.1+, can be replaced by a thin driver that constructs a `Bank`, registers accounts, and either runs the demo or hands control to `Menu`.

## 5. Logging Contract

| Property | Value |
| --- | --- |
| File | `log.txt`, opened with `std::ios::app` |
| Line | `<ISO-like timestamp optional v1.1> Account: <id> \| Action: <TYPE> \| Amount: $<value>\n` |
| Failure mode | Warn on `stderr`; in-memory state stays correct |
| Concurrency | None in v1; process-serial only |

Example output (matches current `log.txt`):

```
Account: ACC-1001 | Action: DEPOSIT | Amount: $150
Account: ACC-1001 | Action: WITHDRAW | Amount: $50
Account: ACC-1001 | Action: INTEREST_ADD | Amount: $15
```

## 6. Build, Run, Tooling

### 6.1 Build

Console demo:

```powershell
g++ -std=c++20 src/main.cpp -Iinclude -o bank_app.exe
```

REST API server (add `-lws2_32` on Windows for Winsock):

```powershell
g++ -std=c++20 src/server.cpp src/Account.cpp src/Logger.cpp -Iinclude -lws2_32 -o bank_server.exe
```

Globbing all source files (builds both `main` and `server` — causes a linker error due to duplicate `main()`; use the explicit commands above):

```powershell
g++ -std=c++20 src/*.cpp -Iinclude -o bank_app.exe
```

The `-Iinclude` flag is optional for root-level headers in v1, but new code under `src/` must include like `#include "bank/Account.h"`.

### 6.2 Run

Console demo:

```powershell
.\bank_app.exe
```

REST API server:

```powershell
.\bank_server.exe
```

The server starts listening on `http://localhost:8080`. The web dashboard is served at `http://localhost:8080/index.html`.

### 6.3 VS Code tasks

`.vscode/tasks.json` defines two build tasks:

| Task | Label | Output |
| --- | --- | --- |
| Default build (Ctrl+Shift+B) | `C/C++: g++ build active project` | `bank_app.exe` (console demo) |
| Server build | `Build REST Server` | `bank_server.exe` (REST API) |

## 8. Environment Configuration

| Variable | Purpose | Notes |
| --- | --- | --- |
| `BANK_LOG_PATH` | Override log file location | Default: `./log.txt`. Reserved for v1.1, not consumed in v1 |
| Port 8080 | HTTP server listen address | Default for `bank_server.exe`; override via CLI arg (`.\bank_server.exe 9090`) |

No `.env`, no secrets, no external network configuration.

## 9. Error Handling Policy

- **Validation errors** (negative amount, insufficient funds): no exception, return value or no-op, no log line.
- **I/O errors**: warn on `stderr`, continue.
- **Invariant violations** (would-be negative balance): rejected at the boundary; never allowed to occur in memory.

The project does not use exceptions for control flow. STL exceptions (`std::ofstream` failure to open) are caught at the `Logger` boundary and reported.

## 10. Threading & Concurrency

Out of scope for v1. `Logger` is process-serial. If threading is introduced later, `Logger` becomes the synchronization owner (mutex inside) — `Account` stays lock-free.

## 11. Versioning & Compatibility

- v1 = current demo, single account, deterministic flow + REST API server with web dashboard.
- v1.1 = `Logger` extraction, `CheckingAccount`, optional CLI menu, multi-account `Bank` facade.
- v2 = Persistent state load on startup, authentication, database-backed storage.

API additions are additive; removals require a major bump.

## 12. Migration Plan from v0 → v1

1. Keep `Account.h` and `SavingsAccount.h` at the repo root (unchanged signatures).
2. Introduce `include/bank/*.h` as forwarding re-exports of the root headers, then progressively move logic.
3. Extract `Logger` from `Account::saveTransaction`.
4. Add `Bank` and `CheckingAccount` as empty stubs.
5. Update `.vscode/tasks.json` to pass `-Iinclude`.

Each step compiles and the demo still runs.