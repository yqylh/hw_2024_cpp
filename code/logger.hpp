#ifndef __LOGGER_H__
#define __LOGGER_H__
#include <iostream>
#include <fstream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

// 基类 Logger
class Logger {
public:
    Logger() : isRunning(true) {
        // 启动日志记录线程
#ifdef DEBUG
        logThread = std::thread(&Logger::logThreadFunction, this);
#endif
    }

    virtual ~Logger() {
#ifdef DEBUG
        {
            std::lock_guard<std::mutex> lock(mutex);
            isRunning = false;
        }
        // 通知日志线程退出
        condition.notify_one();
        // 等待日志线程退出
        logThread.join();
#endif
    }

    // 添加日志接口
    void log(const std::string& message) {
#ifdef DEBUG
        std::lock_guard<std::mutex> lock(mutex);
        logQueue.push(message);
        // 通知日志线程有新日志
        condition.notify_one();
#endif
    }

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

    // 写日志函数，需要子类实现
    virtual void writeLog(const std::string& message) = 0;

    std::thread logThread;
    std::mutex mutex;
    std::condition_variable condition;
    std::queue<std::string> logQueue;
    bool isRunning;
};

// 文件日志记录器
class FileLogger : public Logger {
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
};


#endif