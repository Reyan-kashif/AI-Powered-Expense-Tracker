/*1. Install OpenSSL 
I used this link: https://www.hostnoc.com/install-openssl-on-windows/
2. Unzip the folder this program is in on your system and open the "vs code" folder
3. In tasks.json, set the file path to where you saved the folder.
4. Under "args" in the same file, add these 2 lines:  
"-lssl", 
"-lcrypto"
5. In launch.json, paste the same path after "program:"*/

#include <iostream>
#include <fstream>
#include <string>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <cstring>
#include <curl/curl.h>
#include <vector>
#include <sstream>
#include <map>
#include <iomanip>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <set>

using namespace std;
namespace fs = std::filesystem;


//***********************************//
// This is the Start of Phase 1 Code//
//**********************************//


//key for aes-256
//aes-256-> 256 bits so 32 bytes
//each character is 1 byte which equates to 32 characters 
const string AES_KEY = "ThisIsA32ByteLongSecretKeyForAES!"; //has to be exact 32 characters

// derive 16-byte iv from a string
void derive_iv(const string& source, unsigned char* iv) {
    //makes an array "hash" of size 32 (-> sha-256 again so 32 bytes)
    //type "unsigned" because a hash can't have negative numbers so we have the full range of values(0-255)
    unsigned char hash[32];
    //OPENSSL built in method 
    //1. passing pointer to first char of string, 
    //2. the length so it knows how many bytes to read and 
    //3. the hash array so the function writes the output into it
    SHA256((unsigned char*)source.c_str(), source.length(), hash);
    memcpy(iv, hash, 16); // use first 16 bytes of SHA-256 as IV
}

// hash a password using sha256
string sha256(const string& input) {
    unsigned char hash[32];
    SHA256((unsigned char*)input.c_str(), input.length(), hash);

    //like cin, cout
    //cleaner alternative to concatenation
    stringstream ss;
    //1 hex character represents 4 bits so we need 2 hex characters to form 1 byte
    //and hence 64 characters for the full hash
    //this loop converts each hexa into a two character version and if there is no preceding value it sets it to zero
    for (int i = 0; i < 32; i++)
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    return ss.str();
}

// encrypt using aes-256-cbc
string aes256_encrypt(const string& plaintext, const string& key, const string& inputForIV) {
    unsigned char iv[16];
    derive_iv(inputForIV, iv);

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new(); // create an "encryption context"

    if (!ctx) {
        cout<<"error creating encryption context";
        exit(1);
    }
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, (unsigned char*)key.c_str(), iv))
        exit(1);

    // padding is the process of adding extra bytes to make the text a multiple of the block size 
    // this ensures the data can be properly encrypted and decrypted
    // padding the input text to make it fit the required block size (that is 16 bytes in AES' case)
    // padding ensures the text length is a multiple of 16 for the encryption to work
    int padding = 16 - (plaintext.length() % 16);
    int totalLength = plaintext.length() + padding;
    unsigned char* padded = new unsigned char[totalLength];
    memcpy(padded, plaintext.c_str(), plaintext.length());
    memset(padded + plaintext.length(), padding, padding);

    unsigned char* ciphertext = new unsigned char[totalLength + 16]; // to allow for extra padding
    int len, ciphertext_len;

    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, padded, totalLength))
        exit(1);
    ciphertext_len = len;

    // finalize the encryption process
    // ensures the final part of the text is encrypted
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        exit(1);
    ciphertext_len += len;

    stringstream ss;
    for (int i = 0; i < ciphertext_len; i++)
        ss << hex << setw(2) << setfill('0') << (int)ciphertext[i];

    // free memory properly
    delete[] padded;
    delete[] ciphertext;
    EVP_CIPHER_CTX_free(ctx);

    return ss.str();
}

//  User class to initialise credentials
class User {
public:
    string username; // encrypted username in hexa form
    string email; //""
    string passwordHash; //hashed password

    //initialise manually
    User(string un, string mail, string hashPwd) {
        username = un;
        email = mail;
        passwordHash = hashPwd;
    }
};

// The Authentication System class (our main functioning class)
class AuthenSystem{
public:
    string userfilename;
    bool signup() {
        int choice=0;
        string un, mail, pwd;
        cout << "Enter username: "; 
        cin >> un;
        cout << "Enter email: "; 
        cin >> mail;
        cout << "Enter password: ";
        cin >> pwd;

        // store hashed password, and encrypted data by calling earlier self-made functions
        string hashedPwd = sha256(pwd);
        string encryptedUn = aes256_encrypt(un, AES_KEY, un);
        string encryptedMail = aes256_encrypt(mail, AES_KEY, mail);

        // make the User instance and pass prepared parameters into it
        User newUser(encryptedUn, encryptedMail, hashedPwd);

        // Open the file to store the user info in write mode
        //ios::app will make the file and write to it on first run

        userfilename=un;
        filesystem::create_directories("Expenses/" + userfilename);

        ofstream file("users.txt", ios::app);
        if (file.is_open()) {
            file << newUser.username << " " << newUser.email << " " << newUser.passwordHash << endl;
            file.close();
            cout << "Signup successful!" << endl;
            choice=1;
        } else {
            cerr << "Failed to open file!" << endl; 
        }

        return choice;
    }

    int login() {
        //take user input once again to match with stored credentials
        int choice=0;
        string un, mail, pwd;
        cout << "Enter username: ";
        cin >> un;
        cout << "Enter email: ";
        cin >> mail;
        cout << "Enter password: ";
        cin >> pwd;

        string encryptedUn = aes256_encrypt(un, AES_KEY, un);
        string encryptedMail = aes256_encrypt(mail, AES_KEY, mail);
        //since we dont want to decrypt the hash revealing our password we can just get the hash 
        //for the entered password and compare that as the same password will also have the same hash
        string hashedPwd = sha256(pwd);

        //open file in read mode for comparison
        ifstream file("users.txt");
        //prepare variable to store file contents
        string file_un, file_mail, file_hash;
        //initialise a verification flag with false
        bool found = false;

        //read and compare contents untill eof
        while (file >> file_un >> file_mail >> file_hash) {
            if (file_un == encryptedUn && file_mail == encryptedMail && file_hash == hashedPwd) {
                userfilename=un; 
                found = true;
                choice=1;
                break;
            }
        }

        file.close();

        if (found) {
            cout << "Login successful!" << endl;
        } else {
            cout << "Invalid credentials!" << endl;
        }
        return choice;
    }
    friend void SaveExpenseToCSV(AuthenSystem& auth); 
    friend void Display_Category_Report(AuthenSystem& auth);
    friend void Display_Summary_Report(AuthenSystem& auth);
};

//***********************************//
// This is the Start of Phase 2 Code//
//**********************************//

class Expense
{
private:
    string Amount;
    string Category;
    string Description;

public:
    
    string get_category(){
        return Category;
    }
    string get_description(){
        return Description;
    }
    string get_amount(){
        return Amount;
    }

    void set_category(string category)
    {
        Category = category;
    }

    void set_description(string description)
    {
        Description = description;
    }

    void set_amount(string amount)
    {
        Amount = amount;
    }

friend class AI_Categorization;
};


class AI_Categorization
{
private:
    Expense& expense;
    AuthenSystem& auth;
    string User_input;
public:
    AI_Categorization(string user_input, Expense& exp,AuthenSystem& au) : User_input(user_input), expense(exp),auth(au) {}

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* output) {
        size_t totalSize = size * nmemb;
        output->append((char*)contents, totalSize);
        return totalSize;
    }

    string extractContent(const string& jsonResponse) {
        string key = "\"content\":";
        size_t start = jsonResponse.find(key);
    
        if (start == string::npos) {
            return "Could not find \"content\" key in response.";
        }
    
        // Move past the key
        start = jsonResponse.find("\"", start + key.length());
        if (start == string::npos) return "Malformed content field.";
    
        start++; // move past opening quote
        string content;
        bool escape = false;
    
        for (size_t i = start; i < jsonResponse.size(); ++i) {
            char c = jsonResponse[i];
    
            if (escape) {
                if (c == 'n') content += '\n';
                else if (c == 't') content += '\t';
                else if (c == '"') content += '"';
                else if (c == '\\') content += '\\';
                else content += c;
                escape = false;
            }
            else {
                if (c == '\\') escape = true;
                else if (c == '"') break;
                else content += c;
            }
        }
    
        return content;
    }

    string Categorization(const string& prompt,const string& input){
            CURL* curl;
            CURLcode res;
            string response;
        
            string api_key = "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";  // Replace with your key
            string post_data = R"({
                "model": "gpt-3.5-turbo",
                "messages": [{"role": "user", "content": ")" + prompt + input + R"("}]
            })";
        
            curl_global_init(CURL_GLOBAL_ALL);
            curl = curl_easy_init();
        
            if (curl) {
                curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/chat/completions");
        
                struct curl_slist* headers = NULL;
                headers = curl_slist_append(headers, "Content-Type: application/json");
                headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key).c_str());
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
                curl_easy_setopt(curl, CURLOPT_POST, 1L);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
        
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        
                curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
        
                res = curl_easy_perform(curl);
        
                if (res != CURLE_OK)
                    cerr << "cURL Error: " << curl_easy_strerror(res) << endl;
        
                curl_slist_free_all(headers);
                curl_easy_cleanup(curl);
            }
        
            curl_global_cleanup();
            return extractContent(response);
    }

     void CategorizeAll() {
        expense.Category = Categorization("What is the expense category (one word)", User_input);
        expense.Description = Categorization("Extract the description from the following don't add anything yourself (without amount): ", User_input);
        expense.Amount = Categorization("Extract the amount in numbers only", User_input);
     }



//***********************************//
// This is the Start of Phase 3 Code//
//**********************************//



void ConfirmAndSaveExpense()
    {
        // Show the extracted information for confirmation
        cout << "\nExtracted Information:\n";
        cout << "Category: " << expense.get_category() << endl;
        cout << "Description: " << expense.get_description() << endl;
        cout << "Amount: " << expense.get_amount() << endl;

        // Ask for confirmation before saving
        char confirm;
        cout << "\nDo you want to save this expense? (y/n): ";
        cin >> confirm;

        if (confirm == 'y' || confirm == 'Y')
        {
            SaveExpenseToCSV(auth);
        }
        else
        {
            cout << "Expense was not saved.\n";
        }
    }

    void SaveExpenseToCSV(AuthenSystem& auth)
    {
        // Create expenses directory if it doesn't exist
        if (!fs::exists("expenses"))
        {
            fs::create_directory("expenses");
        }

        // Normalize category for filename
        string category_original = expense.get_category();
        string category_normalized = category_original;
        for (auto &ch : category_normalized)
            ch = tolower(ch);

        // File path
        string filename = "expenses/"+ auth.userfilename +"/"+ category_normalized + ".csv";

        // Check if file exists and if it's empty
        bool file_exists = fs::exists(filename);
        bool file_empty = file_exists && fs::file_size(filename) == 0;

        ofstream file(filename, ios::app);

        if (file.is_open())
        {
            // Only write header if file is new or empty
            if (!file_exists || file_empty)
            {
                file << "Timestamp,Amount,Description\n";
            }

            // Current timestamp
            auto now = chrono::system_clock::now();
            time_t raw_time = chrono::system_clock::to_time_t(now);
            tm *timeinfo = localtime(&raw_time);

            stringstream timestamp_ss;
            timestamp_ss << (1900 + timeinfo->tm_year) << "-";
            timestamp_ss << setw(2) << setfill('0') << (1 + timeinfo->tm_mon) << "-";
            timestamp_ss << setw(2) << setfill('0') << timeinfo->tm_mday << " ";
            timestamp_ss << setw(2) << setfill('0') << timeinfo->tm_hour << ":";
            timestamp_ss << setw(2) << setfill('0') << timeinfo->tm_min << ":";
            timestamp_ss << setw(2) << setfill('0') << timeinfo->tm_sec;

            // Write expense entry
            file << timestamp_ss.str() << ","
                 << expense.get_amount() << ","
                 << expense.get_description() << "\n";

            file.close();
            cout << "Expense saved to " << filename << endl;
        }
        else
        {
            cerr << "Failed to open file for writing.\n";
        }
    }

    // Function to handle manual entry
    void ManualEntry() {
        string amount, category, description;
    
        cin.clear();
        cin.sync();
    
        cout << "\nEnter Amount: ";
        getline(cin, amount);
    
        cout << "Enter Category: ";
        getline(cin, category);
    
        cout << "Enter Description: ";
        getline(cin, description);
    
        expense.set_amount(amount);
        expense.set_category(category);
        expense.set_description(description);
    
        SaveExpenseToCSV(auth);
    }


    void SaveCategoryIfNotExists(const string& categoryName) {
        set<string> existingCategories;
        ifstream inputFile("categories.txt");
        string line;
    
        if (inputFile.is_open()) {
            while (getline(inputFile, line)) {
                if (!line.empty()) {
                    existingCategories.insert(line);
                }
            }
            inputFile.close();
        }
    
        if (existingCategories.find(categoryName) == existingCategories.end()) {
            ofstream outputFile("categories.txt", ios::app);
            if (outputFile.is_open()) {
                outputFile << categoryName << "\n";
                outputFile.close();
            } else {
                cout << "Error writing to categories.txt!\n";
            }
        }
    }
    
};

//***********************************//
// This is the Start of Phase 4 Code//
//**********************************//



class Display_Manager
{
public:
    void Display_Category_Report(string category_filename,AuthenSystem& auth){
        ifstream file("expenses/"+auth.userfilename + "/"+ category_filename +".csv");
        string line;

        vector<string> columnNames;
        map<string, vector<string>> columnData;
        int rowCount = 0;
    
        if (file.is_open()) {
            // Read header
            if (getline(file, line)) {
                stringstream ss(line);
                string columnName;
    
                while (getline(ss, columnName, ',')) {
                    columnNames.push_back(columnName);
                    columnData[columnName] = {};
                }
            }
    
            // Read data rows
            while (getline(file, line)) {
                stringstream ss(line);
                string cell;
                int colIndex = 0;
    
                while (getline(ss, cell, ',')) {
                    if (colIndex < columnNames.size()) {
                        columnData[columnNames[colIndex]].push_back(cell);
                    }
                    colIndex++;
                }
                rowCount++;
            }
    
            file.close();
        } else {
            cout << "Error opening file!" << endl;
        }
    
        const int width = 30;
    
        cout << string((width + 3) * columnNames.size() + 1, '-') << endl;
    
        cout << "|";
        for (const string& col : columnNames) {
            cout << " " << setw(width) << left << col << " |";
        }
        cout << endl;
    
        cout << string((width + 3) * columnNames.size() + 1, '-') << endl;
    
        for (int i = 0; i < rowCount; ++i) {
            cout << "|";
            for (const string& col : columnNames) {
                string cell = (i < columnData[col].size()) ? columnData[col][i] : "";
                cout << " " << setw(width) << left << cell << " |";
            }
            cout << endl;
        }
        cout << string((width + 3) * columnNames.size() + 1, '-') << endl;
    
    }

    void Display_Summary_Report(vector<string> category_files,AuthenSystem& auth) {
        map<string, int> categoryTotals;
        int grandTotal = 0;
    
        for (const string& fileName : category_files) {
            ifstream file("expenses/" +auth.userfilename+"/"+ fileName + ".csv");
            string line;
            int total = 0;
            int amountColumnIndex = -1;
    
            if (file.is_open()) {
                if (getline(file, line)) {
                    stringstream ss(line);
                    string headerCell;
                    int index = 0;
                    while (getline(ss, headerCell, ',')) {
                        if (headerCell == "Amount" || headerCell == "amount") {
                            amountColumnIndex = index;
                            break;
                        }
                        index++;
                    }
                }
    

                if (amountColumnIndex != -1) {
                    while (getline(file, line)) {
                        stringstream ss(line);
                        string cell;
                        int colIndex = 0;
    
                        while (getline(ss, cell, ',')) {
                            if (colIndex == amountColumnIndex) {
                                try {
                                    total += stoi(cell);
                                } catch (...) {
                                }
                                break;
                            }
                            colIndex++;
                        }
                    }
                } else {
                    cout << "Amount column not found in " << fileName << ".csv\n";
                }
    
                file.close();
                categoryTotals[fileName] = total;
                grandTotal += total;
            }
        }
    
        // --- Display summary table ---
        const int width = 20;
        cout << string((width + 3) * 2 + 1, '-') << endl;
        cout << "| " << setw(width) << left << "Category"
             << " | " << setw(width) << left << "Total Amount ($)" << " |" << endl;
        cout << string((width + 3) * 2 + 1, '-') << endl;
    
        for (const auto& entry : categoryTotals) {
            cout << "| " << setw(width) << left << entry.first
                 << " | " << setw(width) << left << entry.second << " |" << endl;
        }
    
        cout << string((width + 3) * 2 + 1, '-') << endl;
        cout << "| " << setw(width) << left << "Total Spent"
             << " | " << setw(width) << left << grandTotal << " |" << endl;
        cout << string((width + 3) * 2 + 1, '-') << endl;
    }    
    
};


void ShowExpenseMenu()
{
    cout << "\n============================================\n";
    cout << "            Expense Tracker Menu\n";
    cout << "============================================\n";
    cout << "0. Go Back\n";
    cout << "1. Use AI Categorization\n";
    cout << "2. Manual Entry\n";
    cout << "============================================\n";
    cout << "Please choose an option (0/1/2): ";
}

void ShowDisplayMenu(){
    cout << "\n============================================\n";
    cout << "            Expense Tracker Menu\n";
    cout << "============================================\n";
    cout << "0. Go Back\n";
    cout << "1. Display Category Spending\n";
    cout << "2. Display Total Spending\n";
    cout << "============================================\n";
    cout << "Please choose an option (0/1/2): ";
}


void ShowMainMenu(){
    cout << "\n============================================\n";
    cout << "            Expense Tracker Menu\n";
    cout << "============================================\n";
    cout << "1. Enter A Expense\n";
    cout << "2. View Spending's\n";
    cout << "3. Exit\n";
    cout << "============================================\n";
    cout << "Please choose an option (1/2/3): ";
}

void ShowLoginMenu(){
    cout << "1. Sign up\n";
    cout << "2. Login\n";
    cout << "3. Exit\n";
    cout << "============================================\n";
    cout << "Please choose an option (1/2/3): ";
}

void clearConsole() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear"); 
    #endif
}

void waitForEnter() {
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}



// main menu
// uses object to access methods
int main() {
    int choice;
    int loggedIn = 0;
    string User_Input;
    string User_Input_Category;

    AuthenSystem auth;
    Expense Data;
    Display_Manager Display;

    // --- Welcome Message ---
    cout << "\n============================================\n";
    cout << "          Welcome to Expense Tracker \n";
    cout << "============================================\n";

    // --- Login or Sign Up ---
    while (true) {
        ShowLoginMenu();
        cout << "\nYour choice: ";
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 1) {
            if (auth.signup()) {
                loggedIn = 1;
                break;
            }
        } else if (choice == 2) {
            if (auth.login()) {
                loggedIn = 1;
                break;
            }
        } else if (choice == 3) {
            cout << "\nThank you for visiting Expense Tracker. See you again!\n";
            return 0;
        } else {
            cout << "\nInvalid option. Please enter 1, 2, or 3.\n";
        }
    }

    // --- Main Program After Login ---
    if (loggedIn) {
        while (true) {
            clearConsole();
            ShowMainMenu();
            cout << "\nYour choice: ";
            cin >> choice;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            if (choice == 1) {
                while (true) {
                    clearConsole();
                    ShowExpenseMenu();
                    cout << "\nYour choice: ";
                    cin >> choice;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');

                    if (choice == 1) {
                        cout << "\nEnter the expense for AI Categorization: ";
                        getline(cin, User_Input);
                        AI_Categorization user(User_Input, Data,auth);
                        user.CategorizeAll();
                        user.ConfirmAndSaveExpense();
                        cout << "\nExpense recorded successfully!\n";
                        user.SaveCategoryIfNotExists(Data.get_category());
                        waitForEnter();
                        break;
                    } else if (choice == 2) {
                        AI_Categorization user(User_Input, Data,auth);
                        user.ManualEntry();
                        cout << "\nExpense manually entered successfully!\n";
                        user.SaveCategoryIfNotExists(Data.get_category());
                        waitForEnter();
                        break;
                    } else if (choice == 0) {
                        cout << "\nReturning to Main Menu...\n";
                        break;
                    } else {
                        cout << "\nInvalid option. Please enter 0, 1, or 2.\n";
                    }
                }
            }

            else if (choice == 2) {
                while (true) {
                    clearConsole();
                    ShowDisplayMenu();
                    cout << "\nYour choice: ";
                    cin >> choice;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');

                    if (choice == 1) {
                        cout << "\nEnter the category for report: ";
                        cin >> User_Input_Category;
                        Display.Display_Category_Report(User_Input_Category,auth);
                        cout << "\nReturning to Main Menu...\n";
                        waitForEnter();
                        waitForEnter();
                        break;
                    } else if (choice == 2) {
                        vector<string> categoryFiles;
                        ifstream inputFile("categories.txt");
                        string category;

                        if (inputFile.is_open()) {
                            while (getline(inputFile, category)) {
                                if (!category.empty()) {
                                    categoryFiles.push_back(category);
                                }
                            }
                            inputFile.close();
                        } else {
                            cout << "\nCould not open categories.txt!\n";
                        }

                        Display.Display_Summary_Report(categoryFiles,auth);
                        cout << "\nReturning to Main Menu...\n";
                        waitForEnter();
                        break;
                    } else if (choice == 0) {
                        cout << "\nReturning to Main Menu...\n";
                        break;
                    } else {
                        cout << "\nInvalid option. Please enter 0, 1, or 2.\n";
                    }
                }
            }

            else if (choice == 3) {
                cout << "\nThank you for using Expense Tracker. Goodbye!\n";
                break;
            }

            else {
                cout << "\nInvalid option. Please enter 1, 2, or 3.\n";
            }
        }
    }

    return 0;
}
