#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cctype>
#include <algorithm>

using namespace std;

// Функция для удаления лишних пробелов
string removeExtraSpaces(const string& str) {
    string result;
    unique_copy(str.begin(), str.end(), back_inserter(result),
        [](char a, char b) { return isspace(a) && isspace(b); });  //лямбда функция, возвращает true, если а и b пробелы
    return result;
}

// Функция для нормализации имени макроса (замена спецсимволов на _)
string normalizeMacroName(string str) {
    for (char& c : str) {
        if (!isalnum(c)) {
            c = '_';
        }
    }
    return str;
}

// Функция для обработки одного оператора C и преобразования его в макрос
string translateToMacro(const string& code) {
    string processed = removeExtraSpaces(code);
    stringstream ss(processed);
    string token;
    vector<string> tokens;
    
    // Улучшенное разбиение на токены с сохранением строковых литералов
    bool inString = false;
    string currentToken;
    for (char c : processed) {
        if (c == '"') {
            inString = !inString;
            currentToken += c;
        } else if (isspace(c) && !inString) {
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
        } else {
            currentToken += c;
        }
    }
    if (!currentToken.empty()) {
        tokens.push_back(currentToken);
    }
    
    if (tokens.empty()) return "";
    
    // Обработка if-условий
    if (tokens[0] == "if") {
        if (tokens.size() < 4) return code; // Неполный if
        
        string condition;
        // Объединяем все токены условия (могут быть вложенные скобки)
        size_t i = 1;
        int bracketLevel = 0;
        while (i < tokens.size()) {
            if (tokens[i].find('(') != string::npos) bracketLevel++;
            if (tokens[i].find(')') != string::npos) bracketLevel--;
            
            condition += tokens[i] + " ";
            i++;
            
            if (bracketLevel <= 0 && i > 1 && 
                (tokens[i-1].back() == ')' || bracketLevel <= 0)) {
                break;
            }
        }
        
        // Убираем внешние скобки если они есть
        if (condition.front() == '(' && condition.back() == ')') {
            condition = condition.substr(1, condition.size() - 2);
        }
        
        // Остаток - тело условия
        string body;
        for (; i < tokens.size(); ++i) {
            body += tokens[i] + " ";
        }
        
        string macroName = "IF_" + normalizeMacroName(condition);
        return "#define " + macroName + " if (" + condition + ") " + body;
    }
    // Обработка return
    else if (tokens[0] == "return") {
        if (tokens.size() < 2) return code;
        string value;
        for (size_t i = 1; i < tokens.size(); ++i) {
            value += tokens[i] + " ";
        }
        value = value.substr(0, value.size() - 1); // Удаляем последний пробел
        return "#define RETURN_" + normalizeMacroName(value) + " return " + value;
    }
    // Обработка присваиваний
    else if (tokens.size() >= 3 && tokens[1] == "=") {
        string var = tokens[0];
        string value;
        for (size_t i = 2; i < tokens.size(); ++i) {
            value += tokens[i] + " ";
        }
        value = value.substr(0, value.size() - 1); // Удаляем последний пробел
        return "#define SET_" + var + "_" + normalizeMacroName(value) + " " + var + " = " + value;
    }
    // Обработка for-циклов
    else if (tokens[0] == "for") {
        if (tokens.size() < 5) return code;
        
        string params;
        size_t i = 1;
        int bracketLevel = 0;
        while (i < tokens.size()) {
            if (tokens[i].find('(') != string::npos) bracketLevel++;
            if (tokens[i].find(')') != string::npos) bracketLevel--;
            
            params += tokens[i] + " ";
            i++;
            
            if (bracketLevel <= 0 && i > 1 && 
                (tokens[i-1].back() == ')' || bracketLevel <= 0)) {
                break;
            }
        }
        
        string body;
        for (; i < tokens.size(); ++i) {
            body += tokens[i] + " ";
        }
        
        string macroName = "FOR_" + normalizeMacroName(params);
        return "#define " + macroName + " for (" + params + ") " + body;
    }
    
    // Если не распознано, вернуть как есть
    return code;
}

int main() {
    cout << "Введите C-код (пустая строка для завершения):\n";
    
    string line;
    while (true) {
        cout << "> ";
        getline(cin, line);
        if (line.empty()) break;
        
        string macro = translateToMacro(line);
        cout << "Результат преобразования:\n" << macro << "\n\n";
    }
    
    return 0;
}