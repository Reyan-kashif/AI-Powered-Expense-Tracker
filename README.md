
# 🎓 CS-112 Semester Project: AI-Powered Expense Tracker

---

## 🧾 Description  
A secure **C++ application** that:  
✅ Automatically categorizes expenses using **AI (OpenAI API)**  
✅ Encrypts user data with **military-grade AES-256 encryption**  
✅ Generates detailed **spending reports with ASCII visualizations**  
✅ Stores data locally in **organized CSV files**  

---

## ⚙️ Installation  

### 📋 Prerequisites  
- C++ compiler (e.g., `g++`, MinGW)  
- OpenSSL libraries  
- cURL (for OpenAI API calls)  

### 🛠️ Compile & Run  

#### 🔸 Linux/macOS  
```bash
g++ main.cpp -o expense_tracker -lssl -lcrypto -lcurl  
./expense_tracker
```

#### 🔸 Windows (MinGW)  
```bash
g++ main.cpp -o expense_tracker -lssl -lcrypto -lcurl -lws2_32  
expense_tracker.exe
```

---

## 🔐 Security System (Phase 1)  

### 🔑 Key Features  
**1. SHA-256 Password Hashing**  
- Converts passwords to irreversible 64-character hashes  
```cpp
string hashedPwd = sha256("user_password");
```

**2. AES-256 Encryption**  
- 32-byte key encrypts usernames/emails  
```cpp
const string AES_KEY = "ThisIsA32ByteLongSecretKeyForAES!";  
string encrypted = aes25_encrypt(text, AES_KEY, iv_source);
```
🔐 Encrypted data is stored in `users.txt`  

---

## 🤖 Smart Categorization (Phase 2)  

### 💡 How It Works  
**User Input:**  
```
"500 for dinner at KFC yesterday"
```

**AI Extracts:**  
- Amount: `500`  
- Category: `food`  
- Description: `dinner at KFC`  

📁 Saves to: `/expenses/food.csv` with a timestamp

---

## 📂 Data Storage & Management (Phase 3)  

### 📌 Key Features  
**1. Automated CSV Handling**  
- Creates `/expenses/` folder on first run  
- Generates new category files (e.g., `food.csv`) as needed  
- Appends entries with timestamps:  
```
2024-05-01 14:30:00,500,KFC dinner
```

**2. Category Management**  
- Tracks all used categories in `categories.txt`  
- Prevents duplicates using `SaveCategoryIfNotExists()`

**3. Fallback Manual Entry System**  
```cpp
void ManualEntry() { // If AI categorization fails  
  cout << "Enter Amount: ";  
  getline(cin, amount); // Handles spaces in input  
}
```

**4. Error Handling**  
- Checks file permissions before writing  
- Validates directory creation using `std::filesystem`  

---

## 📊 Reporting Features (Phase 4)  

### 📁 Sample Outputs  

**1. Category Report**  
```
---------------------------------------------------
| Timestamp           | Amount | Description      |
---------------------------------------------------
| 2024-05-01 12:30:45 | 500    | dinner at KFC    |
---------------------------------------------------
```

**2. Summary Report**  
```
---------------------------
| Category | Total Amount |
---------------------------
| food     | 1500         |
| transport| 800          |
---------------------------
```

---

## ⚠️ Important Notes  
1. Replace your **OpenAI API key** in `main.cpp` (around **Line 290**)  
2. Keep `cacert.pem` in the **project directory**  
3. For production use, **change the hardcoded AES key**

---

## 👥 Team  

| Phase | Component        | Contributor     |  
|-------|------------------|-----------------|  
| 1     | Authentication   | Irab Zara       |  
| 2     | AI Integration   | Reyan Kashif    |  
| 3     | File Management  | Ali Kamran      |  
| 4     | Reporting        | Reyan Kashif    |  

---

