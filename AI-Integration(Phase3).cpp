#include <iostream>
#include <string>
#include <fstream>
#include <curl/curl.h>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

using namespace std;
namespace fs = std::filesystem;

class Expense
{
private:
    string Amount;
    string Category;
    string Description;

public:
    string get_category()
    {
        return Category;
    }

    string get_description()
    {
        return Description;
    }

    string get_amount()
    {
        return Amount;
    }

    void set_category(const string &category)
    {
        Category = category;
    }

    void set_description(const string &description)
    {
        Description = description;
    }

    void set_amount(const string &amount)
    {
        Amount = amount;
    }

    friend class AI_Categorization;
};

class AI_Categorization
{
private:
    Expense &expense;
    string User_input;

public:
    AI_Categorization(string user_input, Expense &exp) : User_input(user_input), expense(exp) {}

    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, string *output)
    {
        size_t totalSize = size * nmemb;
        output->append((char *)contents, totalSize);
        return totalSize;
    }

    string extractContent(const string &jsonResponse)
    {
        string key = "\"content\":";
        size_t start = jsonResponse.find(key);

        if (start == string::npos)
        {
            return "Could not find \"content\" key in response.";
        }

        start = jsonResponse.find("\"", start + key.length());
        if (start == string::npos)
            return "Malformed content field.";

        start++;
        string content;
        bool escape = false;

        for (size_t i = start; i < jsonResponse.size(); ++i)
        {
            char c = jsonResponse[i];

            if (escape)
            {
                if (c == 'n')
                    content += '\n';
                else if (c == 't')
                    content += '\t';
                else if (c == '"')
                    content += '"';
                else if (c == '\\')
                    content += '\\';
                else
                    content += c;
                escape = false;
            }
            else
            {
                if (c == '\\')
                    escape = true;
                else if (c == '"')
                    break;
                else
                    content += c;
            }
        }

        return content;
    }

    string Categorization(const string &prompt, const string &input)
    {
        CURL *curl;
        CURLcode res;
        string response;

        string api_key = "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"; // Replace with your actual API key
        string post_data = R"({
            "model": "gpt-3.5-turbo",
            "messages": [{"role": "user", "content": ")" +
                           prompt + input + R"("}]
        })";

        curl_global_init(CURL_GLOBAL_ALL);
        curl = curl_easy_init();

        if (curl)
        {
            curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/chat/completions");

            struct curl_slist *headers = NULL;
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

    void CategorizeAll()
    {
        expense.set_category(Categorization("What is the expense category (one word)? ", User_input));
        expense.set_description(Categorization("Extract the description from the following don't add anything yourself (without amount): ", User_input));
        expense.set_amount(Categorization("Extract the amount only: ", User_input));
    }

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
            SaveExpenseToCSV();
        }
        else
        {
            cout << "Expense was not saved.\n";
        }
    }

    void SaveExpenseToCSV()
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
        string filename = "expenses/" + category_normalized + ".csv";

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
        cin.sync();  // Clears any leftover input
    
        cout << "\nEnter Amount: ";
        getline(cin, amount);
    
        cout << "Enter Category: ";
        getline(cin, category);
    
        cout << "Enter Description: ";
        getline(cin, description);
    
        expense.set_amount(amount);
        expense.set_category(category);
        expense.set_description(description);
    
        SaveExpenseToCSV();
    }
    
};

void ShowStartMenu()
{
    cout << "\n============================================\n";
    cout << "            Expense Tracker Menu\n";
    cout << "============================================\n";
    cout << "1. Use AI Categorization\n";
    cout << "2. Manual Entry\n";
    cout << "3. View All Expenses\n";
    cout << "4. Exit\n";
    cout << "============================================\n";
    cout << "Please choose an option: ";
}

int main()
{
    string User_Input;
    char choice;

    while (true)
    {
        // Show start menu
        ShowStartMenu();
        cin >> choice;

        // Clear the input buffer to avoid leftover newlines
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clears the input buffer

        if (choice == '1')
        {
            Expense Data;
            cout << "\nEnter the Expense for AI Categorization: ";
            getline(cin, User_Input); // Now getline will work correctly
            AI_Categorization user(User_Input, Data);
            user.CategorizeAll();
            user.ConfirmAndSaveExpense();
        }
        else if (choice == '2')
        {
            Expense Data;
            AI_Categorization user(User_Input, Data);
            user.ManualEntry(); // Directly saves the manual entry without confirmation
        }
        else if (choice == '4')
        {
            cout << "\nExiting the program. Thank you!\n";
            break; // Exit the program
        }
        else
        {
            cout << "\nInvalid choice. Please enter a valid option (1/2/3/4).\n";
        }
    }

    return 0;
}