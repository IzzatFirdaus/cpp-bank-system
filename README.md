# Bank Management System (C++)

A lightweight, Object-Oriented C++ banking system featuring a console demo, a REST API server, and a modern web dashboard. Demonstrates OOP design, modern smart pointers, file-based transaction logging, and HTTP service architecture.

## Core Concepts & Features

- **Object-Oriented Design (OOP):** Polymorphic class hierarchy with a base `Account` class and derived `SavingsAccount`.
- **Modern C++ Memory Management:** `std::unique_ptr` for strict, leak-free ownership. No raw owning pointers.
- **File I/O Persistence:** Every transaction (deposits, withdrawals, interest) is append-logged to `log.txt` with ISO-8601 timestamps.
- **REST API Server:** Lightweight HTTP server (`cpp-httplib`) exposing account operations as JSON endpoints on port 8080.
- **Web Dashboard:** Responsive HTML5/CSS3/JavaScript frontend with real-time balance display, transaction actions, and audit log.
- **Encapsulation:** Internal balance data protected by validated transaction logic and boundary invariants.

## Project Architecture

```
cpp-bank-system/
├── include/
│   ├── httplib.h                   # cpp-httplib single-header HTTP library
│   └── bank/
│       ├── Account.h               # Base class: identity, balance, deposit, withdraw
│       ├── SavingsAccount.h        # Derived class: interest rate, applyInterest()
│       ├── CheckingAccount.h       # v1.1 stub
│       ├── Logger.h                # Append-only sink for log.txt
│       └── Bank.h                  # Account collection facade
├── src/
│   ├── main.cpp                    # Console demo driver
│   ├── server.cpp                  # REST API server + static file serving
│   ├── Account.cpp                 # Account implementation
│   ├── Logger.cpp                  # Logger implementation
│   └── Bank.cpp                    # Bank facade implementation
├── public/
│   └── index.html                  # Web dashboard (served by the HTTP server)
├── tests/                          # Reserved for future tests
├── main.cpp                        # v1: root-level demo driver (legacy)
├── Account.h                       # v1: root-level header (legacy)
├── SavingsAccount.h                # v1: root-level header (legacy)
└── .vscode/tasks.json              # Build tasks (console + server)
```

## Prerequisites & Toolchain

- **Compiler:** `g++` (GCC) with C++20 support.
- **Platform:** Windows (primary), Linux, macOS.
- **No external package managers** — `cpp-httplib` is vendored as a single header under `include/httplib.h`.

## Getting Started

### 1. Clone the Repository

```bash
git clone https://github.com/IzzatFirdaus/cpp-bank-system.git
cd cpp-bank-system
```

### 2. Choose a Build Target

#### Option A: Console Demo (deterministic, single-run)

```powershell
g++ -std=c++20 src/main.cpp -Iinclude -o bank_app.exe
.\bank_app.exe
```

Expected output:

```
--- Smart Banking System ---
Account: ACC-1001
Initial Balance: $500
Final Balance after interest: $615
Transactions logged to log.txt.
```

#### Option B: REST API Server + Web Dashboard (interactive)

```powershell
g++ -std=c++20 src/server.cpp src/Account.cpp src/Logger.cpp -Iinclude -lws2_32 -o bank_server.exe
.\bank_server.exe
```

Then open **http://localhost:8080/index.html** in your browser.

> **Note:** On Windows, `-lws2_32` links the Winsock library required by `cpp-httplib`. On Linux/macOS, omit this flag.

### 3. REST API Endpoints

| Endpoint | Method | Request Body | Response |
| --- | --- | --- | --- |
| `/api/account` | GET | — | `{"accountNumber":"ACC-1001","balance":600.00,"interestRate":2.50}` |
| `/api/deposit` | POST | `{"amount": 100.00}` | `{"success":true,"balance":700.00}` |
| `/api/withdraw` | POST | `{"amount": 50.00}` | `{"success":true,"balance":650.00}` |
| `/api/apply-interest` | POST | — | `{"success":true,"balance":666.25,"credited":16.25}` |
| `/api/logs` | GET | — | `["2026-09-11T00:29:31 Account: ACC-1001 \| Action: INTEREST_ADD \| Amount: $13.75", ...]` |

**Error responses:**

- `400 Bad Request` — invalid or non-positive amount.
- `422 Unprocessable Entity` — withdrawal exceeds balance.

### 4. Web Dashboard Features

- **Account Summary Card** — displays account ID, current balance, and interest rate.
- **Transaction Panel** — input fields and buttons for deposit, withdraw, and apply interest.
- **Real-Time Audit Log** — live feed from `log.txt` with color-coded status badges (`DEPOSIT`, `WITHDRAW`, `INTEREST_ADD`).
- **Toast Notifications** — immediate feedback on success or rejection (e.g., "Insufficient Funds", "Invalid Amount").
- **CORS-enabled** — works with any local browser origin.

### 5. Verify Transaction Logging

After any transaction, inspect `log.txt`:

```
2026-09-11T00:29:15 Account: ACC-1001 | Action: DEPOSIT | Amount: $100.00
2026-09-11T00:29:22 Account: ACC-1001 | Action: WITHDRAW | Amount: $50.00
2026-09-11T00:29:31 Account: ACC-1001 | Action: INTEREST_ADD | Amount: $13.75
```

## VS Code Integration

Two build tasks are available:

| Task | Shortcut | Description |
| --- | --- | --- |
| `C/C++: g++ build active project` | Ctrl+Shift+B | Builds the console demo (`bank_app.exe`) |
| `Build REST Server` | Run Task menu | Builds the REST server (`bank_server.exe`) |

## Design Principles

- **Balance Invariant:** `balance >= 0.0` is enforced at the boundary. No operation can make it negative.
- **Append-Only Logging:** `log.txt` is opened in append mode. Never truncated.
- **No Exceptions for Control Flow:** Validation failures return `false` or no-op. I/O failures warn to `stderr`.
- **Additive Public API:** New methods are added; existing signatures are frozen within v1.x.