#ifndef __LOGGER_H__
#define __LOGGER_H__
#include <iostream>
#include <fstream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

#ifdef DEBUG
#include "fmt/format.h"
#endif

// 基类 Logger
class Logger {
#ifdef DEBUG
public:
    Logger() : isRunning(true) {
        // 启动日志记录线程

        logThread = std::thread(&Logger::logThreadFunction, this);
    }

    virtual ~Logger() {
        {
            std::lock_guard<std::mutex> lock(mutex);
            isRunning = false;
        }
        // 通知日志线程退出
        condition.notify_one();
        // 等待日志线程退出
        logThread.join();
    }

    // 添加日志接口
    template<typename... Args>
    void log(const int& nowFrame, const std::string& formatStr, const Args&... args) {
        // std::string result = std::format(formatStr, args...);
        // concatenate the nowFrame to the front of the result
        // std::string result = std::format("{0}: {1}", nowFrame, result);
        // merge the above two lines
        std::string result = fmt::format("{0}:{1}", nowFrame, fmt::format(formatStr, args...));

        std::lock_guard<std::mutex> lock(mutex);
        logQueue.push(result);
        // 通知日志线程有新日志
        condition.notify_one();
    }

    // 添加日志接口
    /*
    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex);
        logQueue.push(message);
        // 通知日志线程有新日志
        condition.notify_one();
    }
    */
   
protected:
    // 日志线程函数
    void logThreadFunction() {
        while (isRunning) {
            std::unique_lock<std::mutex> lock(mutex);
            // 等待有新日志到来或者线程终止
            condition.wait(lock, [this] { return !logQueue.empty() || !isRunning; });
            // 输出日志
            while (!logQueue.empty()) {
                writeLog(logQueue.front());
                logQueue.pop();
            }
        }
    }
    
    std::thread logThread;
    std::mutex mutex;
    std::condition_variable condition;
    std::queue<std::string> logQueue;
    
#else
public:
    Logger() : isRunning(true) {}

    virtual ~Logger() {}

    template<typename... Args>

    void log(const int& nowFrame, const std::string& formatStr, const Args&... args) {}
#endif

protected:
    bool isRunning;
    // 写日志函数，需要子类实现
    virtual void writeLog(const std::string& message) = 0;



};

// 文件日志记录器
class FileLogger : public Logger {
#ifdef DEBUG
public:
    FileLogger(const std::string& filename) : outputStream(filename) {}

    ~FileLogger() {
        if (outputStream.is_open()) {
            outputStream.close();
        }
    }

private:
    void writeLog(const std::string& message) override {
        if (outputStream.is_open()) {
            outputStream << message << std::endl;
        }
    }

    std::ofstream outputStream;
#else
public:
    FileLogger(const std::string& filename) {}
private:
    void writeLog(const std::string& message) override {}
#endif
};


#endif