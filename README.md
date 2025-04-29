# CS-112 Semester Project: AI-Powered Expense Tracker

## Description
A secure C++ application that:
-> Automatically categorizes expenses using AI (OpenAI API)  
-> Encrypts user data with military-grade AES-256 encryption  
-> Generates detailed spending reports with ASCII visualizations  
-> Stores data locally in organized CSV files  

## Installation
1. Prerequisites:
   - C++ compiler (g++/MinGW)
   - OpenSSL libraries
   - cURL (for API calls)

2. Compile & Run:
# Linux/macOS
   g++ main.cpp -o expense_tracker -lssl -lcrypto -lcurl
   ./expense_tracker

# Windows (MinGW)
   g++ main.cpp -o expense_tracker -lssl -lcrypto -lcurl -lws2_32
   expense_tracker.exe

# Security System (Phase 1)
Key Features:
1. SHA-256 Password Hashing:
// Converts passwords to irreversible 64-character hashes
string hashedPwd = sha256("user_password");
 
2. AES-256 Encryption:
// 32-byte key encrypts usernames/emails
const string AES_KEY = "ThisIsA32ByteLongSecretKeyForAES!"; 
string encrypted = aes25_encrypt(text, AES_KEY, iv_source);
- Encrypted data stored in users.txt 

# Smart Categorization (Phase 2)
Steps:
1. User inputs raw expense:
"500 for dinner at KFC yesterday"
2. AI extracts:
Amount: 500
Category: food
Description: dinner at KFC
3. Saves to /expenses/food.csv with the timestamp

# Data Storage & Management (Phase 3)
Key Features:
1. Automated CSV Handling:
Creates /expenses/ folder on first run
Generates new category files (e.g., food.csv) when needed
Appends entries with timestamps:
2024-05-01 14:30:00,500,KFC dinner

2. Category Management:
Tracks all used categories in categories.txt
Prevents duplicates with SaveCategoryIfNotExists()

3. Fallback manual entry system:
void ManualEntry() { // If AI categorization fails
  cout << "Enter Amount: ";  
  getline(cin, amount); // Handles spaces in input
}
4. Error Handling:
Checks file permissions before writing
Validates directory creation with std::filesystem

# Reporting Features (Phase 4)
Sample Outputs:
1. Category Report:

---------------------------------------------------
| Timestamp           | Amount | Description      |
---------------------------------------------------
| 2024-05-01 12:30:45 | 500    | dinner at KFC    |
---------------------------------------------------
2. Summary Report:

---------------------------
| Category | Total Amount |
---------------------------
| food     | 1500         |  
| transport| 800          |
---------------------------

Important Notes
1. Replace OpenAI API key in main.cpp (Line 290)
2. Keep cacert.pem in project directory
3. Change hardcoded AES key for production use

Team 
Phase	Component	Contributor  
1	Authentication	Irab Zara  
2	AI Integration	Reyan Kashif  
3	File Management	Ali Kamran  
4	Reporting	Reyan Kashif  
