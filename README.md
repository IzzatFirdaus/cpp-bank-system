# Bank Management System (C++) 

A lightweight, Object-Oriented C++ console application designed to demonstrate fundamental software engineering principles, class inheritance, modern smart pointers, and file-based transaction logging.

## Core Concepts & Features

- **Object-Oriented Design (OOP):** Employs polymorphic class architecture using a base `Account` class and a derived `SavingsAccount` class.
- **Modern C++ Memory Management:** Utilizes `std::unique_ptr` for strict, leak-free object ownership without relying on raw pointers.
- **File I/O Persistence:** Automatically appends and logs every transaction (deposits, withdrawals, and interest additions) to a persistent `log.txt` file.
- **Encapsulation:** Protects internal balance data using standard getter methods and validated transaction logic.

## Project Architecture

- `Account.h`: Base abstract class handling account identifiers, current balances, core operations, and transaction logging routines.
- `SavingsAccount.h`: Derived class extending base functionalities with specific features like interest rate calculation.
- `main.cpp`: Executable entry point executing dynamic account interactions and file operations.

## Prerequisites & Toolchain

- **Compiler:** `g++` (GCC) with support for modern C++20 standard capabilities.
- **Build System:** Integrated via VS Code shell tasks (`g++`).

## Getting Started

### 1. Clone the Repository

```bash
git clone [https://github.com/IzzatFirdaus/cpp-bank-system.git](https://github.com/IzzatFirdaus/cpp-bank-system.git)
cd cpp-bank-system

```

### 2. Compile the Application

Run the GCC compiler using C++20 flag:

```powershell
g++ -std=c++20 main.cpp -o bank_app.exe

```

### 3. Run the Executable

```powershell
.\bank_app.exe

```

### 4. Verify Transaction Logging

After executing the binary, open `log.txt` generated in the root folder to inspect recorded account actions:

```text
Account: ACC-1001 | Action: DEPOSIT | Amount: $150
Account: ACC-1001 | Action: WITHDRAW | Amount: $50
Account: ACC-1001 | Action: INTEREST_ADD | Amount: $15

```