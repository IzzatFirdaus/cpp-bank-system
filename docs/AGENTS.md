# AGENTS.md — Operational Rules for AI Contributors

> Read `ARCHITECTURE-ESSENTIALS.md` first, then `ARCHITECTURE.md`. This file governs *how* you change code.

## 1. Scope

This file applies to **all AI coding agents** (Copilot, Claude, Gemini, etc.) editing this repository. Human contributors are encouraged to follow it too.

## 2. Read-before-write

Before changing anything:

1. Read `PRD.md` to confirm the change serves a documented requirement.
2. Read `ARCHITECTURE-ESSENTIALS.md` for non-negotiable rules.
3. Read `ARCHITECTURE.md` for the technical detail behind the essentials.
4. Read `CLAUDE.md` for the exact build/run commands.
5. Use `read_file` / `grep_search` to inspect the file you intend to change. Never edit a file you have not read in full.

If a requested change conflicts with any of the above, **stop and ask the user** before proceeding.

## 3. Hard rules (do not violate)

1. **C++20 only.** Do not introduce C++23-only syntax.
2. **No external dependencies.** If you need a library, justify it in writing and add it to `ARCHITECTURE.md` first.
3. **No raw owning pointers.** Use `std::unique_ptr` / `std::shared_ptr`.
4. **`balance >= 0` is an invariant.** Never bypass it, even "temporarily" in a debug branch.
5. **Do not truncate `log.txt`.** Append mode only.
6. **Do not throw exceptions for control flow.** Use return values / no-ops.
7. **Do not swallow I/O errors silently.** Warn to `stderr` with the path and the OS error.
8. **Public API is additive within v1.x.** Removing or renaming a public method requires a version bump discussion.
9. **Do not edit `PRD.md`, `ARCHITECTURE.md`, `ARCHITECTURE-ESSENTIALS.md`, `AGENTS.md`, `CLAUDE.md`** without explicit user instruction. These are governance documents.
10. **Do not regenerate `log.txt`.** It is gitignored runtime output.

## 4. File modification safety

| File / area | Allowed for AI agents? | Notes |
| --- | --- | --- |
| `Account.h` | Yes, additive only | New virtuals allowed with `override`; existing signatures frozen |
| `SavingsAccount.h` | Yes, additive only | |
| `main.cpp` | Yes | The demo driver is free to evolve, but must still satisfy PRD §6.1 |
| `include/bank/*.h` | Yes | New headers welcome; mirror the namespace |
| `src/*.cpp` / `src/*.h` | Yes | New modules only; do not duplicate `Account` logic |
| `tests/` | Yes | Use header-only assertions until a real framework is chosen |
| `.vscode/tasks.json` | Yes, additive only | Don't remove the existing build task |
| `README.md` | Yes, factual corrections | Keep the build/run instructions accurate |
| `PRD.md`, `ARCHITECTURE*.md`, `AGENTS.md`, `CLAUDE.md` | **No**, requires user | |

## 5. Preferred design patterns

- **Ownership:** `std::unique_ptr` for single-owner; `std::shared_ptr` only when lifetime is genuinely shared; raw pointers as non-owning observers.
- **Interfaces:** prefer a non-virtual public interface (`deposit`, `withdraw`) with `virtual` overridable behavior. Keep the virtual surface small.
- **Validation:** at the boundary (constructor, public method entry). Never inside private helpers.
- **Constants:** `constexpr` over `const` where possible. Magic numbers become named constants.
- **Headers:** include what you use; avoid "kitchen sink" `<bits/stdc++.h>`.
- **Namespaces:** everything under `namespace bank`. Do not pollute the global namespace.
- **Error reporting:** prefer a return value for predictable failures (`bool withdraw`), `stderr` for unexpected I/O failures, exceptions only for genuinely exceptional conditions (and never for validation).

## 6. Step-by-step: implementing a new feature

Use this checklist for any non-trivial change.

1. **Confirm scope.** Open `PRD.md`. Does the feature appear, or does it need a PRD amendment? If the latter, stop and ask.
2. **Confirm architecture.** Open `ARCHITECTURE.md`. Is the feature already designed? If not, propose a delta and get user sign-off before coding.
3. **Read existing code.** Always. Especially the header you plan to extend.
4. **Plan minimal changes.**
   - New product variant → new header under `include/bank/`, no edits to `Account.h`.
   - New I/O target → extend `Logger`.
   - New driver behavior → edit `main.cpp` (or add a new file under `src/`).
5. **Write the change.** Keep functions small, names descriptive, comments rare (only where intent is non-obvious).
6. **Run the build.** `g++ -std=c++20 src/main.cpp -Iinclude -o bank_app.exe` or use the VS Code task. Fix all warnings.
7. **Run the program.** `./bank_app.exe`. Inspect `log.txt` and confirm it matches the documented format.
8. **Self-review.** Re-read the diff. Apply the hard rules checklist (§3). Verify no invariant was weakened.
9. **Report.** Summarize what changed, what was tested, and anything that still needs the user.

## 7. Comment hygiene

- Prefer self-explanatory names over comments.
- A comment is justified only when it explains **why**, not **what**.
- Remove AI-slop comments ("This function does X") before finishing.
- Never edit working code just to add or remove a comment.

## 8. Testing expectations

- For a demo project, a clean build + a single happy-path run is the minimum bar.
- For any new validation logic, add at least one negative test in `tests/`.
- Do not add a test framework dependency without user approval.

## 9. Git hygiene

- Commit messages start with a verb: "Add CheckingAccount stub", "Extract Logger from Account".
- One logical change per commit.
- Do not commit `bank_app.exe`, `*.o`, or `log.txt` — `.gitignore` already covers them.

## 10. When in doubt

Ask the user. Cite the file and rule you are unsure about. Do not guess on governance questions.