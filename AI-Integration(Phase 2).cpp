#include<iostream>
#include<string>
#include<curl/curl.h>
using namespace std;

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
    string User_input;
public:
    AI_Categorization(string user_input, Expense& exp) : User_input(user_input), expense(exp) {}

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
        
            string api_key = "Insert your Api Key";
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
        expense.Description = Categorization("Extract the description from the following don't add anything yourself (without amount)", User_input);
        expense.Amount = Categorization("Extract the amount in numbers only", User_input);
     }
};

int main(){

    string User_Input;
    cout<<"Enter the Expense";
    getline(cin,User_Input);

    Expense Data;
    AI_Categorization user(User_Input,Data);
    user.CategorizeAll();
    

    cout<<"Category :"<<Data.get_category()<<endl;
    cout<<"Description :"<<Data.get_description()<<endl;
    cout<<"Amount :"<<Data.get_amount()<<endl;

}