# CLAUDE.md — CLI Quickstart for LLM Tools

> For LLM-driven shells (Claude Code, OpenHands, etc.). Humans may also find it useful as a cheat sheet.

## 0. Operating principles (do not skip)

1. Read `ARCHITECTURE-ESSENTIALS.md` first, then `ARCHITECTURE.md`. Do not start coding without them in context.
2. Honor `AGENTS.md`. If a rule conflicts with a user request, raise the conflict before coding.
3. Never modify `PRD.md`, `ARCHITECTURE.md`, `ARCHITECTURE-ESSENTIALS.md`, `AGENTS.md`, `CLAUDE.md`, or `README.md` without explicit permission.
4. Append-only logging. Never rewrite `log.txt`.
5. C++20 only. No external dependencies.

## 1. Common commands

> The shell is PowerShell 5.1 on Windows unless noted. Use `;` to chain, never `&&`.

### 1.1 Build

Console demo (single-file build):

```powershell
g++ -std=c++20 src/main.cpp -Iinclude -o bank_app.exe
```

If you add new `.cpp` files, glob them:

```powershell
g++ -std=c++20 src/*.cpp -Iinclude -o bank_app.exe
```

REST API server (requires `cpp-httplib`, add `-lws2_32` on Windows):

```powershell
g++ -std=c++20 src/server.cpp src/Account.cpp src/Logger.cpp -Iinclude -lws2_32 -o bank_server.exe
```

### 1.2 Run

Console demo:

```powershell
.\bank_app.exe
```

REST API server:

```powershell
.\bank_server.exe
```

Expected output:

```
Bank API server listening on http://localhost:8080
Dashboard: http://localhost:8080/index.html
Press Ctrl+C to stop.
```

### 1.3 Use the Web Dashboard

With the server running, open **http://localhost:8080/index.html** in a browser. The dashboard provides:
- Account summary (ID, balance, interest rate)
- Deposit / Withdraw / Apply Interest actions
- Real-time audit log from `log.txt`
- Toast notifications for success/error feedback

### 1.4 Test the API

```powershell
curl.exe -s http://localhost:8080/api/account
```

### 1.5 Inspect the log

```powershell
Get-Content .\log.txt
```

### 1.4 Reset the log (manual, demo only)

```powershell
Remove-Item .\log.txt -ErrorAction SilentlyContinue
```

`log.txt` is `.gitignore`d; removing it has no source-control cost.

### 1.5 Lint / format (optional)

No formatter is mandated. If `clang-format` is available:

```powershell
clang-format -i src/*.cpp include/bank/*.h
```

### 1.6 Clean build artifacts

```powershell
Remove-Item .\bank_app.exe -ErrorAction SilentlyContinue
Remove-Item .\*.o -ErrorAction SilentlyContinue
```

## 2. Key path aliases

| Alias | Maps to | Purpose |
| --- | --- | --- |
| `${workspaceFolder}` | repo root | VS Code task variable |
| `include/` | `include/bank/*.h` | Public headers |
| `src/` | `src/*.cpp`, `src/*.h` | Implementation |
| `tests/` | `tests/*.cpp` | Reserved |
| `docs/` | `docs/*.md` | Governance documents |

## 3. Repository map

```
cpp-bank-system/
├── Account.h                 # v1: root-level header (unchanged public API)
├── SavingsAccount.h          # v1: root-level header
├── main.cpp                  # v1: deterministic demo driver
├── README.md                 # user-facing docs
├── PRD.md                    # product requirements
├── ARCHITECTURE.md           # full technical spec
├── ARCHITECTURE-ESSENTIALS.md# fast-load summary + stress-test audit
├── AGENTS.md                 # rules for AI contributors
├── CLAUDE.md                 # this file
├── include/
│   ├── httplib.h             # cpp-httplib single-header HTTP library (vendored)
│   └── bank/
│       ├── Account.h         # canonical header (post-migration)
│       ├── SavingsAccount.h
│       ├── CheckingAccount.h # v1.1 stub
│       ├── Logger.h          # append-only sink
│       └── Bank.h            # account-collection facade
├── src/
│   ├── main.cpp              # console demo driver
│   ├── server.cpp            # REST API server + static file serving
│   ├── Account.cpp           # Account implementation
│   ├── Logger.cpp            # Logger implementation
│   └── Bank.cpp              # Bank facade implementation
├── public/
│   └── index.html            # web dashboard (served by the HTTP server)
├── tests/                    # reserved
└── .vscode/
    └── tasks.json
```

## 4. VS Code integration

- Default task: `C/C++: g++ build active project` (Ctrl+Shift+B) — builds the console demo.
- Server task: `Build REST Server` — builds the REST API server.
- The task glob `src/*.cpp` only — root-level `main.cpp` is intentionally excluded so the two do not collide. Use the explicit build command above if both exist.

## 5. Adding a new account type — recipe

1. Create `include/bank/CheckingAccount.h`.
2. Inherit from `bank::Account`.
3. Override `withdraw(double)` if behavior differs.
4. Do **not** edit `Account.h`.
5. Build, run, inspect `log.txt`.

## 6. Adding a new CLI command — recipe

1. Edit `src/main.cpp` (or `src/Menu.h` once it has content).
2. Keep the deterministic demo flow intact if you only add an optional branch.
3. Update `README.md` if the user-facing output changes.

## 7. Failure triage

| Symptom | Likely cause | First check |
| --- | --- | --- |
| Linker error: undefined `Account::saveTransaction` | Logger extraction in progress; include path missing | Confirm `-Iinclude` |
| Linker error: undefined `__imp_setsockopt` etc. | Missing Winsock library on Windows | Add `-lws2_32` to the link flags |
| `log.txt` missing | Working directory changed | Run from repo root |
| Balance drifts unexpectedly | Floating-point in `applyInterest` | Document; do not "fix" with rounding in v1 |
| New header not picked up | Build task globs `src/*.cpp` only | Use the explicit build command |
| Server port 8080 in use | Another process is occupying the port | Kill the other process or pass a different port (`.\bank_server.exe 9090`) |
| Dashboard shows "Server unreachable" | Server not running or CORS issue | Confirm `bank_server.exe` is running on port 8080 |

## 8. Quick reference

| Need | Look at |
|---|---|
| What to build | `PRD.md` |
| How to build it | `ARCHITECTURE.md` |
| The non-negotiables | `ARCHITECTURE-ESSENTIALS.md` |
| How to behave | `AGENTS.md` |
| How to run | this file, §1 |
| Where files live | this file, §3 |