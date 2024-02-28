#pragma once
#include <type_traits>
#include <string>
#include <vector>
#include <assets\\File.h>
#include <ctime>
#include <queue>
#include <functional>
#include <thread>
#include <mutex>



//
//class OpenGLLogger {
//private:
//    sqlite3* db;
//
//    WorkerThread logWorker;
//
//
//public:
//    static OpenGLLogger& Instance() {
//        static OpenGLLogger instance;
//        return instance;
//    }
//
//    void Init(const std::string& dbFilePath) {
//        // Open or create the SQLite database file
//        int rc = sqlite3_open(dbFilePath.c_str(), &db);
//        if (rc != SQLITE_OK) {
//            throw std::runtime_error("Failed to open or create the SQLite database: " + std::string(sqlite3_errmsg(db)));
//        }
//
//        // Create a table to store log data (if it doesn't exist)
//        const char* createTableSQL = "CREATE TABLE IF NOT EXISTS OpenGLLog (FunctionName TEXT, ParameterName TEXT, ParameterValue TEXT);";
//        rc = sqlite3_exec(db, createTableSQL, 0, 0, 0);
//        if (rc != SQLITE_OK) {
//            throw std::runtime_error("Failed to create the table: " + std::string(sqlite3_errmsg(db)));
//        }
//    }
//
//
//    void LogCall(const std::string& functionName, const std::vector<std::string>& parameterNames, const std::vector<std::string>& parameterValues) {
//        logWorker.AddWork(
//            [this, functionName, parameterNames, parameterValues]() {
//                logCall(functionName, parameterNames, parameterValues); 
//            });
//    }
//    
//
//
//    ~OpenGLLogger() {
//        if (db) {
//            sqlite3_close(db);
//        }
//    }
//private:
//    void logCall(const std::string functionName, const std::vector<std::string> parameterNames, const std::vector<std::string> parameterValues) {
//        if (parameterNames.size() != parameterValues.size()) {
//            throw std::invalid_argument("Parameter names and values size mismatch");
//        }
//
//        const char* insertSQL = "INSERT INTO OpenGLLog (FunctionName, ParameterName, ParameterValue) VALUES (?, ?, ?);";
//        sqlite3_stmt* stmt;
//        int rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, 0);
//        if (rc != SQLITE_OK) {
//            throw std::runtime_error("Failed to prepare the SQL statement: " + std::string(sqlite3_errmsg(db)));
//        }
//
//
//        std::string formattedData = "[";
//
//        for (size_t i = 0; i < parameterNames.size(); ++i) {
//            formattedData += "[" + parameterNames[i] + ", " + parameterValues[i] + "]";
//            if (i < parameterNames.size() - 1) {
//                formattedData += ", ";
//            }
//        }
//
//        formattedData += "]";
//
//        rc = sqlite3_bind_text(stmt, 1, functionName.c_str(), -1, SQLITE_STATIC);
//        rc |= sqlite3_bind_text(stmt, 2, formattedData.c_str(), -1, SQLITE_STATIC);
//        if (rc != SQLITE_OK) {
//            throw std::runtime_error("Failed to bind values to the SQL statement: " + std::string(sqlite3_errmsg(db)));
//        }
//
//        rc = sqlite3_step(stmt);
//        if (rc != SQLITE_DONE) {
//            throw std::runtime_error("Failed to execute the SQL statement: " + std::string(sqlite3_errmsg(db)));
//        }
//
//        sqlite3_finalize(stmt);
//    }
//};
//
//
//template<typename T, typename = void>
//struct has_insertion_operator : std::false_type {};
//
//template<typename T>
//struct has_insertion_operator<T, std::void_t<decltype(std::declval<std::ostream&>() << std::declval<T>())>> : std::true_type {};
//
//struct OpenGLCall {
//    std::string functionName;
//
//    std::vector<std::string> parameterNames;
//    std::vector<std::string> parameterValues;
//
//    // Only enable this overload if the tokenType has an operator<<
//    template<typename T>
//    std::enable_if_t<has_insertion_operator<T>::value, void>
//        addParameterValue(const T& parameter) {
//        std::ostringstream oss;
//        oss << parameter;
//        parameterValues.push_back(oss.str());
//    }
//
//    // Fallback for types without operator<<
//    template<typename T>
//    std::enable_if_t<!has_insertion_operator<T>::value, void>
//        addParameterValue(const T& parameter) {
//        std::ostringstream oss;
//        oss << "No << operator; Address: " << &parameter;
//        parameterValues.push_back(oss.str());
//    }
//
//
//
//};
//
//
//#define GL_CALL(Function, ...) \
//    glCallCUSTOMIMPL(Function, #Function, #__VA_ARGS__, ##__VA_ARGS__)
//
//void GL_TIME_STAMP(std::string name) {
//    auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
//
//    std::tm timeInfo;
//    localtime_s(&timeInfo, &currentTime);
//
//    char timeString[80];
//    std::strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeInfo);
//
//    OpenGLLogger::Instance().LogCall("timestamp", { "Name", "time" }, { name , timeString });
//}
//
//
//template<typename Function, typename... Args>
//decltype(auto) glCallCUSTOMIMPL(Function func, const char* functionName, const char* parameterNames, Args&&... args) {
//    OpenGLCall call;
//    /*
//	call.functionName = functionName;
//
//    int32_t bracketCount = 0;
//    int32_t lastFlush = 0;
//    size_t i = 0;
//    for (; parameterNames[i] != char(0); i++) {
//        if (parameterNames[i] == '(') {
//            bracketCount++;
//        }
//        if (parameterNames[i] == ')') {
//            bracketCount--;
//        }
//        if (bracketCount == 0 && parameterNames[i] == ',') {
//            call.parameterNames.emplace_back(parameterNames+lastFlush, parameterNames+i);
//            lastFlush = i;
//        }
//    }
//    call.parameterNames.emplace_back(lastFlush, i);
//
//    (call.addParameterValue(std::forward<Args>(args)), ...);
//    OpenGLLogger::Instance().LogCall(call.functionName, call.parameterNames, call.parameterValues);*/
//
//    if constexpr (std::is_same_v<decltype(func(std::forward<Args>(args)...)), void>) {
//        func(std::forward<Args>(args)...);
//    }
//    else {
//        decltype(auto) result = func(std::forward<Args>(args)...);
//        return result;
//    }
//}