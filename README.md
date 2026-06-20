# Secure Login and Registration System (C++)

A robust, menu-driven command-line Login and Registration System written in C++. The system prioritizes security by salting and hashing credentials before writing them to the database.

---

## 🚀 Key Features

* **Cryptographic Security:** Passwords are never stored in plaintext. The system generates a unique **16-character random salt** for each user and hashes the combined password + salt using **SHA-256**.
* **Input Validation & Constraints:**
  * **Usernames:** Must be alphanumeric only, between 4 and 20 characters, with no special symbols or spaces.
  * **Passwords:** Must be between 8 and 30 characters and include at least one uppercase letter (A-Z), one lowercase letter (a-z), one digit (0-9), and one special character (e.g. `!`, `@`, `#`, `$`, etc.).
* **Duplicate Username Checking:** Scans the database file (`users.dat`) to ensure usernames are unique.
* **Premium UX Features:** 
  * Interactive **masked password input** (displays `*` as characters are typed) with backspace support.
  * Fallback to standard input reading if run in a non-interactive pipe or automated test suite, preventing infinite loops.

---

## 📂 File Structure

* `main.cpp` - Contains the CLI menu loop, input validation routines, and login/registration flows.
* `sha256.h` & `sha256.cpp` - Self-contained SHA-256 cryptographic hashing implementation.
* `.gitignore` - Prevents tracked compiled binaries (`.exe`, `.obj`, etc.) and the user database (`users.dat`) from being committed.

---

## 🛠️ Compilation and Execution

### Prerequisites
Make sure you have a standard C++11 compiler installed on your system (e.g., GCC/MinGW).

### 1. Compile the Source Code
Open your terminal/PowerShell in the repository directory and run:
```powershell
g++ -std=c++11 main.cpp sha256.cpp -o login_system.exe
```

*Note: If you are using LLVM-MinGW UCRT on Windows, run:*
```powershell
& "C:\Users\iamsh\AppData\Local\Microsoft\WinGet\Packages\MartinStorsjo.LLVM-MinGW.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\llvm-mingw-20260602-ucrt-x86_64\bin\g++.exe" -std=c++11 main.cpp sha256.cpp -o login_system.exe
```

### 2. Run the Application
Start the compiled executable:
```powershell
.\login_system.exe
```

---

## 🔒 Storage Format

All credentials are saved in `users.dat` with the following schema:
```text
<username> <salt> <sha256_hash>
```

Example database record:
```text
alice cOQb7wNiVBQEiL5v 73d53f1496d6c045a7938ef12a19f994cc454f5e13350cbb378bb81730bf03d6
```