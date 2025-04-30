#include<iostream>
#include<fstream>
#include<vector>
#include<sstream>
#include<map>
#include<iomanip>
using namespace std;


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



int main(){
    Display_Manager data;
    string user_input;
    cout<<"Enter the category for report: ";
    cin>>user_input;
    data.Display_Category_Report(user_input);

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
        cout << "Could not open categories.txt!" << endl;
        return 1;
    }

    data.Display_Summary_Report(categoryFiles);

    return 0;
}

