using namespace std;
#define NOMINMAX

#define byte win_byte_backup
#include <windows.h>
#undef byte

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <chrono>
#include <thread>
#include <conio.h>
#include <fstream>
#include <sstream>
#include <stack>
#include <iomanip>

namespace fs = std::filesystem;

vector<pair<string, double>> stockData;
string startDate, endDate;
int startIdx = 0, endIdx = stockData.size() - 1;

void resetIndices() {
    startIdx = 0;
    endIdx = (int)stockData.size() - 1;
}

void loadCSV(std::string filename) {
    ::stockData.clear();
    std::ifstream file(filename);
    std::string line;
    std::getline(file, line);
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string date, priceStr;
        std::getline(ss, date, ',');
        std::getline(ss, priceStr);
        if(date.empty() || priceStr.empty())
            continue;
        double price = std::stod(priceStr);
        ::stockData.emplace_back(date, price);
    }
    if (!::stockData.empty()) {
        startDate = ::stockData.front().first;
        endDate = ::stockData.back().first;
    } else {
        startDate = "";
        endDate = "";
    }
    resetIndices();
}

void gotoxy(int x, int y) {
    COORD coord = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

vector<string> getCSVFiles() {
    vector<string> files;
    for (const auto& entry : fs::directory_iterator("."))
        if (entry.is_regular_file() && entry.path().extension() == ".csv")
            files.push_back(entry.path().filename().string());
    return files;
}

string chooseCSVFile() {
    system("cls");
    gotoxy(0, 0);
    vector<string> csvFiles = getCSVFiles();
    if (csvFiles.empty()) {
        cout << "No CSV Files found in this directory" << endl;
        return "";
    }
    int choice;
    do {
        cout << "Select the file to analyze" << endl;
        for (int i = 0; i < csvFiles.size(); i++)
            cout << i + 1 << ". " << csvFiles[i] << endl;
        cout << "Enter your choice: ";
        cin >> choice;
    } while (choice < 1 || choice > csvFiles.size());
    return csvFiles[choice - 1];
}

bool feasibleDateRange(string from, string to) {
    int startYear = stoi(from.substr(0, 4)), endYear = stoi(to.substr(0, 4));
    if (startYear < endYear)
        return true;
    if (startYear > endYear)
        return false;
    int startMonth = stoi(from.substr(5, 2)), endMonth = stoi(to.substr(5, 2));
    if (startMonth < endMonth)
        return true;
    if (startMonth > endMonth)
        return false;
    int startDay = stoi(from.substr(8, 2)), endDay = stoi(to.substr(8, 2));
    return (endDay - startDay >= 2);
}

int findIdx(string date) {
    for (int i = 0; i < stockData.size(); i++)
        if (stockData[i].first >= date)
            return i;
    return (int)stockData.size() - 1;
}

void dateRange() {
    system("cls");
    gotoxy(0, 0);
    string from, to;
    do {
        cout << "Enter the start date : ";
        cin >> from;
        cout << "Enter the end date : ";
        cin >> to;
        startIdx = findIdx(from);
        endIdx = findIdx(to);
        if (startIdx == -1 || endIdx == -1 || endIdx <= startIdx) {
            cout << "Enter dates within valid range (" << startDate << " - " << endDate << ") and at least 2 days apart" << endl;
            continue;
        }
    } while (!feasibleDateRange(stockData[startIdx].first, stockData[endIdx].first));
}

bool calculateMaxProfit() {
    system("cls");
    gotoxy(0, 0);
    dateRange();
    if (stockData.size() < 2 || startIdx >= endIdx) {
        cout << "Not enough data to calculate maximum profit.\n";
        return true;
    }
    int buyIdx = startIdx, sellIdx = startIdx;
    double maxProfit = 0.0, minPrice = stockData[startIdx].second;
    for (int i = startIdx + 1; i <= endIdx; i++) {
        double profit = stockData[i].second - minPrice;
        if (profit > maxProfit) {
            maxProfit = profit;
            sellIdx = i;
            for (int j = startIdx; j < i; j++) {
                if (stockData[j].second == minPrice) {
                    buyIdx = j;
                    break;
                }
            }
        }
        if (stockData[i].second < minPrice)
            minPrice = stockData[i].second;
    }
    if (maxProfit <= 0)
        cout << "No profitable buy-sell opportunity found.\n";
    else {
        cout << "Buy on : " << stockData[buyIdx].first << " at \u20B9" << stockData[buyIdx].second << endl;
        cout << "Sell on : " << stockData[sellIdx].first << " at \u20B9" << stockData[sellIdx].second << endl;
        cout << "Maximum Profit of \u20B9" << maxProfit << endl;
    }
    return true;
}

bool displayMinMaxPrice() {
    system("cls");
    gotoxy(0, 0);
    dateRange();
    if (stockData.size() < 1 || startIdx >= endIdx) {
        cout << "Not enough data to search for minimum and maximum price.\n";
        return true;
    }
    int minPriceIdx = startIdx, maxPriceIdx = startIdx;
    for (int i = startIdx + 1; i <= endIdx; i++) {
        if (stockData[i].second < stockData[minPriceIdx].second)
            minPriceIdx = i;
        if (stockData[maxPriceIdx].second < stockData[i].second)
            maxPriceIdx = i;
    }
    cout << "Minimum price was observed on " << stockData[minPriceIdx].first << " at \u20B9" << stockData[minPriceIdx].second << endl;
    cout << "Maximum price was observed on " << stockData[maxPriceIdx].first << " at \u20B9" << stockData[maxPriceIdx].second << endl;
    return true;
}

bool calculateAvgPrice() {
    system("cls");
    gotoxy(0, 0);
    dateRange();
    if (stockData.size() < 1 || startIdx > endIdx) {
        cout << "Not enough data to search for minimum and maximum price.\n";
        return true;
    }
    int cnt = 0;
    double priceSum = 0.0;
    for (int i = startIdx; i <= endIdx; i++) {
        cnt += 1;
        priceSum += stockData[i].second;
    }
    double avgPrice = priceSum / (double)cnt;
    cout << fixed << setprecision(2);
    cout << "Average price between " << stockData[startIdx].first << " and " << stockData[endIdx].first << " is : \u20B9" << avgPrice << endl;
    return true;
}

bool calculateStockSpan() {
    system("cls");
    gotoxy(0, 0);
    dateRange();
    if (stockData.size() < 1 || startIdx > endIdx) {
        cout << "Not enough data to Calculate Stock Span.\n";
        return true;
    }
    vector<int> span(endIdx - startIdx + 1);
    stack<int> s;
    for (int i = 0; i < endIdx - startIdx + 1; i++) {
        while (!s.empty() && stockData[startIdx + s.top()].second <= stockData[startIdx + i].second)
            s.pop();
        span[i] = s.empty() ? (i + 1) : (i - s.top());
        s.push(i);
    }
    cout << "Date\t\tPrice\tSpan\n";
    cout << "-------------------------------\n";
    for (int i = 0; i <= endIdx - startIdx; i++) {
        cout << stockData[startIdx + i].first << "\t\u20B9" << fixed << setprecision(2) << stockData[startIdx + i].second << "\t" << span[i] << endl;
    }
    return true;
}

bool searchDateOrPrice() {
    system("cls");
    gotoxy(0, 0);
    cout << "Search Options:\n";
    cout << "1. Search by Date\n";
    cout << "2. Search by Price\n";
    cout << "Enter your choice: ";
    int choice;
    cin >> choice;
    if (choice == 1) {
        string date;
        cout << "Enter the date to search (format YYYY-MM-DD): ";
        cin >> date;
        bool found = false;
        for (auto& entry : stockData) {
            if (entry.first == date) {
                cout << "Price on " << date << " was \u20B9" << fixed << setprecision(2) << entry.second << endl;
                found = true;
                break;
            }
        }
        if (!found)
            cout << "Date not found in data.\n";
    } else if (choice == 2) {
        double price;
        cout << "Enter the price to search: ";
        cin >> price;
        bool found = false;
        cout << "Dates with price \u20B9" << fixed << setprecision(2) << price << ":\n";
        for (auto& entry : stockData) {
            if (abs(entry.second - price) < 1e-2) {
                cout << entry.first << endl;
                found = true;
            }
        }
        if (!found)
            cout << "Price not found in data.\n";
    } else
        cout << "Invalid option. Returning to main menu.\n";
    return true;
}

int getIntInput() {
    int val;
    while (!(std::cin >> val)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid input. Please enter a number: ";
    }
    return val;
}

int displayMenu() {
    system("cls");
    gotoxy(0, 0);
    cout << "Select the option" << endl;
    cout << "1. Calculate Maximum Profit (Buy/Sell)" << endl;
    cout << "2. Show Minimum and Maximum Price" << endl;
    cout << "3. Calculate Average Price" << endl;
    cout << "4. Calculate Stock Span" << endl;
    cout << "5. Search by Date or Price" << endl;
    cout << "6. Select Another File" << endl;
    cout << "7. Exit" << endl;
    int option = getIntInput();
    return option;
}

bool selectOption(int option) {
    system("cls");
    gotoxy(0, 0);
    bool result = false;
    switch (option) {
        case 1:
            result = calculateMaxProfit();
            break;
        case 2:
            result = displayMinMaxPrice();
            break;
        case 3:
            result = calculateAvgPrice();
            break;
        case 4:
            result = calculateStockSpan();
            break;
        case 5:
            result = searchDateOrPrice();
            break;
        case 6:
            return false;
        case 7:
            exit(0);
        default:
            cout << "Select a valid option" << endl;
            break;
    }
    if (option >= 1 && option <= 5) {
        cout << "\nPress any key to return to menu...";
        _getch();
    }
    return true;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    string selectedFile;
    while (true) {
        selectedFile = chooseCSVFile();
        if (selectedFile.empty()) {
            cout << "The file is empty or no files found" << endl;
            continue;
        }
        loadCSV(selectedFile);
        bool analyzing = true;
        while (analyzing) {
            int option = displayMenu();
            analyzing = selectOption(option);
        }
    }
    return 0;
}