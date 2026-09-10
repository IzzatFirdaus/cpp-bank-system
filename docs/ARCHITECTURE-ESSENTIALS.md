# Architecture Essentials — cpp-bank-system

> Load this first. For full design context, see `ARCHITECTURE.md`. On conflict, `ARCHITECTURE.md` wins.

## 1. Non-negotiable rules

1. **Language is C++20.** New code compiles with `-std=c++20`.
2. **No external dependencies** except `cpp-httplib` (single-header, vendored at `include/httplib.h`). Standard library only for all other code.
3. **No raw owning pointers.** Use `std::unique_ptr` (single owner) or `std::shared_ptr` (shared owner). Raw pointers are allowed only as non-owning observers.
4. **`balance` is non-negative.** Any operation that would make it negative is rejected at the boundary.
5. **`log.txt` is an audit trail, not a database.** The program never reads it back into state in v1.
6. **Append-only logging.** Opening mode is always `std::ios::app`. Never truncate.
7. **No exceptions for control flow.** Validation failures return `false` or no-op; I/O failures warn on `stderr`.
8. **Additive API changes only within v1.x.** Removing a public method requires a major version bump.

## 2. Core data structures

```cpp
namespace bank {

class Account {
protected:
    std::string accountNumber;   // read-only after construction
    double      balance;         // invariant: >= 0.0
public:
    explicit Account(std::string accNum, double initialBalance);
    virtual ~Account() = default;
    virtual void  deposit(double amount);          // rejects amount <= 0
    virtual bool  withdraw(double amount);         // rejects amount <= 0 or amount > balance
    double        getBalance()      const noexcept;
    std::string  getAccountNumber() const;
protected:
    void saveTransaction(const std::string& type, double amount);
};

class SavingsAccount : public Account {
public:
    SavingsAccount(std::string accNum, double initialBalance, double rate);  // rate is annual %
    double getInterestRate() const noexcept;
    void applyInterest();   // credits balance * (rate / 100) and logs INTEREST_ADD
};

class Logger { /* append-only sink for log.txt */ };
class Bank   { /* std::vector<std::unique_ptr<Account>> facade, v1.1 */ };
class CheckingAccount : public Account { /* overdraft, v1.1 */ };

} // namespace bank
```

## 3. Primary routes / entry points

| Surface | Path | Purpose |
| --- | --- | --- |
| Build (demo) | `g++ -std=c++20 src/main.cpp -Iinclude -o bank_app.exe` | Compile the console demo binary |
| Build (server) | `g++ -std=c++20 src/server.cpp src/Account.cpp src/Logger.cpp -Iinclude -lws2_32 -o bank_server.exe` | Compile the REST API server (add `-lws2_32` on Windows) |
| VS Code build task | `.vscode/tasks.json` → `C/C++: g++ build active project` | One-click demo build |
| VS Code server task | `.vscode/tasks.json` → `Build REST Server` | One-click server build |
| Run (demo) | `.\bank_app.exe` | Execute the deterministic demo |
| Run (server) | `.\bank_server.exe` | Start HTTP server on port 8080 |
| Dashboard | `http://localhost:8080/index.html` | Web UI (served by the HTTP server) |
| Log output | `log.txt` | Append-only audit trail |

## 4. Logging contract (one-liner)

`Account: <id> | Action: <TYPE> | Amount: $<value>\n` appended to `log.txt`. Failures warn on `stderr` and continue.

## 5. Layered module map

```
+----------------------------+
| main.cpp (console demo)    |  deterministic demo driver
| server.cpp (HTTP server)   |  REST API + static file serving
+----------------------------+
            |
            v
+----------------------------+
| bank::Bank (v1.1 facade)   |  owns std::vector<std::unique_ptr<Account>>
+----------------------------+
            |
            v
+----------------------------+
| bank::Account              |  base, validates deposit/withdraw
|   bank::SavingsAccount     |  adds applyInterest(), getInterestRate()
|   bank::CheckingAccount    |  (v1.1 stub)
+----------------------------+
            |
            v
+----------------------------+
| bank::Logger               |  append-only sink for log.txt
+----------------------------+

+----------------------------+
| cpp-httplib (vendored)     |  single-header HTTP library
+----------------------------+
            |
            v
+----------------------------+
| public/index.html          |  web dashboard (HTML5/CSS3/JS)
+----------------------------+
```

## 6. Quick mental model

- One binary, one demo run, one append-only file — OR — one HTTP server, one browser tab, same append-only file.
- Add a product → new header, no edits to `Account.h`.
- Add an account type with overdraft → override `withdraw`, keep invariants.
- The HTTP server (`server.cpp`) is a thin adapter: it parses requests, calls `Account` methods, and returns JSON. It does not contain business logic.

---

## 7. Stress-test self-audit

> Hard questions asked after the design was drafted. Anything not answered becomes a TODO in `ARCHITECTURE.md`.

### 7.1 What will break first under load or edge cases?

- **`Logger` reopens the file per line.** Every `saveTransaction` call constructs and destructs an `std::ofstream`. For the demo this is fine; under thousands of transactions it will dominate wall time. The first real bottleneck will be here, not in `Account`.
- **No account-number uniqueness.** Two accounts with the same id would interleave indistinguishably in `log.txt`. Anyone parsing the log would silently corrupt any future analytics.
- **No timestamp on log lines.** Reconstructing the order of events across processes (or even across rapid same-second writes) becomes guesswork.
- **`log.txt` is co-located with the executable.** Any working-directory change (CI, packaged run) writes to the wrong place silently.
- **`interestRate` is double.** Floating-point drift compounds across many `applyInterest()` calls; rounding to cents on display will eventually show `$X.0000001`.

### 7.2 What failure modes or edge cases are currently unaccounted for?

- **Concurrent writers.** Two processes appending to `log.txt` can interleave lines because there is no file lock. Acceptable for v1, dangerous the moment a second client exists.
- **Read-only file system / sandboxed directory.** The current code would emit a stream failure without explaining why. `Logger` should report the path and the OS error.
- **Disk full / quota exceeded.** Same shape as above: silent stream failure.
- **Negative opening balance.** The constructor accepts it. The "balance >= 0" invariant is documented but not enforced at construction.
- **Account-number injection.** Free-form strings end up in `log.txt` unescaped. A newline in the id would forge a log entry.
- **Locale-dependent number formatting.** `$150.00` is fine in `en_US`; a different locale prints `$150,00` and breaks downstream parsers.
- **`SavingsAccount::applyInterest()` with `interestRate < 0`.** A "negative interest" feature is plausible in some jurisdictions but undocumented; current code happily credits a negative amount, which then drains the balance.
- **Multiple `applyInterest()` calls in one run.** No idempotency; each call credits again.

### 7.3 Which parts of this design are over-engineered and can be simplified?

- **`bank::Bank` facade in v1** is speculative. Remove from the scaffold; reintroduce the day a second account is added. **Decision:** keep as a *stub only* with no behavior.
- **`bank::CheckingAccount`** is speculative. Same call. **Decision:** keep as a *header-only stub*; no `.cpp`, no behavior.
- **Forwarding re-exports under `include/bank/*.h`** add files without value until the root headers are actually deprecated. **Decision:** in the scaffold pass, the `include/bank/` files directly *contain* the implementation (single source of truth, no re-export indirection).
- **`BANK_LOG_PATH` env var** is reserved but unused. **Decision:** remove from the spec until `Logger` actually reads it; do not reserve undocumented knobs.
- **Namespace `bank::`** is fine, but introducing it before there are multiple translation units is decorative. **Decision:** keep the namespace — it costs nothing and signals intent.

### 7.4 Audit outcome → concrete TODOs

| TODO | Owner | Target version |
| --- | --- | --- |
| Open `log.txt` once per process in `Logger` | v1.1 | v1.1 |
| Validate `initialBalance >= 0` in `Account` ctor | v1.1 | v1.1 |
| Reject account numbers containing `\n` or `\|` | v1.1 | v1.1 |
| Add ISO-8601 timestamp to every log line | v1.1 | v1.1 |
| Use `std::fixed` / `setprecision(2)` for currency formatting | v1.1 | v1.1 |
| Document `interestRate` sign policy (>= 0) | v1.1 | v1.1 |
| File-lock or single-writer contract for `log.txt` | v2 | v2 |