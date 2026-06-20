#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <cctype>
#include <algorithm>
#include <conio.h>
#include <io.h>
#include <stdio.h>
#include "sha256.h"

// Helper function to trim whitespace
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// Generate a random salt of given length
std::string generateSalt(size_t length = 16) {
    const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, chars.size() - 1);
    
    std::string salt = "";
    for (size_t i = 0; i < length; ++i) {
        salt += chars[distribution(generator)];
    }
    return salt;
}

// Validate Username: Alphanumeric and length check
bool isValidUsername(const std::string& username, std::string& errorMsg) {
    if (username.length() < 4 || username.length() > 20) {
        errorMsg = "Username length must be between 4 and 20 characters.";
        return false;
    }
    for (char c : username) {
        if (!std::isalnum(static_cast<unsigned char>(c))) {
            errorMsg = "Username must contain only alphanumeric characters (no spaces or symbols).";
            return false;
        }
    }
    return true;
}

// Validate Password: length check and complexity criteria
bool isValidPassword(const std::string& password, std::string& errorMsg) {
    if (password.length() < 8 || password.length() > 30) {
        errorMsg = "Password length must be between 8 and 30 characters.";
        return false;
    }
    
    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;
    bool hasSpecial = false;
    const std::string specialChars = "!@#$%^&*()_+-=[]{}|;':\",./<>?";
    
    for (char c : password) {
        if (std::isupper(static_cast<unsigned char>(c))) hasUpper = true;
        else if (std::islower(static_cast<unsigned char>(c))) hasLower = true;
        else if (std::isdigit(static_cast<unsigned char>(c))) hasDigit = true;
        else if (specialChars.find(c) != std::string::npos) hasSpecial = true;
    }
    
    if (!hasUpper) {
        errorMsg = "Password must contain at least one uppercase letter (A-Z).";
        return false;
    }
    if (!hasLower) {
        errorMsg = "Password must contain at least one lowercase letter (a-z).";
        return false;
    }
    if (!hasDigit) {
        errorMsg = "Password must contain at least one digit (0-9).";
        return false;
    }
    if (!hasSpecial) {
        errorMsg = "Password must contain at least one special character (e.g. !, @, #, $, etc.).";
        return false;
    }
    return true;
}

// Check if username already exists in users.dat
bool isUsernameDuplicate(const std::string& username) {
    std::ifstream file("users.dat");
    if (!file.is_open()) return false;
    
    std::string u, s, h;
    while (file >> u >> s >> h) {
        if (u == username) {
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

// Read masked input for password
std::string getMaskedInput(const std::string& prompt) {
    std::cout << prompt;
    std::string input = "";
    
    // Check if input stream is a terminal/TTY
    if (!_isatty(_fileno(stdin))) {
        if (!std::getline(std::cin, input)) {
            return "";
        }
        return trim(input);
    }
    
    char ch;
    while (true) {
        ch = _getch();
        if (ch == '\r' || ch == '\n') {
            std::cout << std::endl;
            break;
        } else if (ch == '\b') { // Backspace
            if (!input.empty()) {
                input.pop_back();
                std::cout << "\b \b";
            }
        } else if (ch == 3) { // Ctrl+C
            std::cout << std::endl;
            exit(0);
        } else if (ch == 0 || static_cast<unsigned char>(ch) == 224) { // Special keys (arrows, fn)
            _getch(); // consume extra byte
        } else {
            input += ch;
            std::cout << '*';
        }
    }
    return input;
}

// Register user
void registerUser() {
    std::cout << "\n============================================\n";
    std::cout << "             USER REGISTRATION              \n";
    std::cout << "============================================\n";
    
    std::string username, password, confirmPassword;
    std::string errorMsg;
    
    // Get & validate username
    while (true) {
        std::cout << "Enter Username: ";
        if (!std::getline(std::cin, username)) {
            std::cout << "\nInput stream terminated. Registration aborted.\n";
            return;
        }
        username = trim(username);
        
        if (!isValidUsername(username, errorMsg)) {
            std::cout << "[ERROR] " << errorMsg << "\n\n";
            continue;
        }
        
        if (isUsernameDuplicate(username)) {
            std::cout << "[ERROR] Username already exists. Please choose another one.\n\n";
            continue;
        }
        break;
    }
    
    // Get & validate password
    while (true) {
        password = getMaskedInput("Enter Password: ");
        if (std::cin.eof() || std::cin.fail()) {
            std::cout << "\nInput stream terminated. Registration aborted.\n";
            return;
        }
        if (!isValidPassword(password, errorMsg)) {
            std::cout << "[ERROR] " << errorMsg << "\n\n";
            continue;
        }
        
        confirmPassword = getMaskedInput("Confirm Password: ");
        if (std::cin.eof() || std::cin.fail()) {
            std::cout << "\nInput stream terminated. Registration aborted.\n";
            return;
        }
        if (password != confirmPassword) {
            std::cout << "[ERROR] Passwords do not match. Please try again.\n\n";
            continue;
        }
        break;
    }
    
    // Generate salt, hash password, and store
    std::string salt = generateSalt();
    std::string hashedPassword = sha256(password + salt);
    
    std::ofstream file("users.dat", std::ios::app);
    if (!file.is_open()) {
        std::cout << "[FATAL] Failed to open credentials file for storage.\n";
        return;
    }
    
    file << username << " " << salt << " " << hashedPassword << "\n";
    file.close();
    
    std::cout << "\n[SUCCESS] Registration completed successfully!\n";
}

// Login verification
bool verifyLogin(const std::string& username, const std::string& password) {
    std::ifstream file("users.dat");
    if (!file.is_open()) return false;
    
    std::string u, s, h;
    while (file >> u >> s >> h) {
        if (u == username) {
            file.close();
            std::string calculatedHash = sha256(password + s);
            return calculatedHash == h;
        }
    }
    file.close();
    return false;
}

// Login user
void loginUser() {
    std::cout << "\n============================================\n";
    std::cout << "                 USER LOGIN                 \n";
    std::cout << "============================================\n";
    
    std::string username, password;
    std::cout << "Enter Username: ";
    if (!std::getline(std::cin, username)) {
        std::cout << "\nInput stream terminated. Login aborted.\n";
        return;
    }
    username = trim(username);
    
    password = getMaskedInput("Enter Password: ");
    if (std::cin.eof() || std::cin.fail()) {
        std::cout << "\nInput stream terminated. Login aborted.\n";
        return;
    }
    
    if (verifyLogin(username, password)) {
        std::cout << "\n[SUCCESS] Login successful! Welcome back, " << username << ".\n";
    } else {
        std::cout << "\n[ERROR] Invalid username or password.\n";
    }
}

int main() {
    std::string choice;
    while (true) {
        std::cout << "\n============================================\n";
        std::cout << "        SECURE LOGIN & REGISTRATION         \n";
        std::cout << "============================================\n";
        std::cout << " 1. Register Account\n";
        std::cout << " 2. Login\n";
        std::cout << " 3. Exit\n";
        std::cout << "--------------------------------------------\n";
        std::cout << "Enter Choice (1-3): ";
        if (!std::getline(std::cin, choice)) {
            std::cout << "\nInput stream terminated. Exiting.\n";
            break;
        }
        choice = trim(choice);
        
        if (choice == "1") {
            registerUser();
        } else if (choice == "2") {
            loginUser();
        } else if (choice == "3") {
            std::cout << "\nThank you for using our system. Goodbye!\n";
            break;
        } else {
            std::cout << "[ERROR] Invalid choice. Please select 1, 2, or 3.\n";
        }
    }
    return 0;
}
