#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <fstream>

class Counter{
private:
    std::unordered_map<std::string, int> variables;
    std::unordered_map<std::string, bool> locks;
    std::unordered_map<std::string, std::vector<int>> vectors;
    std::unordered_map<std::string, std::vector<std::pair<int, int>>> vectors2d;

public:
    // 注册变量
    void registerVariable(const std::string& name, int initialValue = 0) {
        variables[name] = initialValue;
        locks[name] = false; // 默认未锁定
    }

    void registerVector(const std::string& name) {
        vectors[name] = std::vector<int>();
        locks[name] = false;
    }

    void registerVector2D(const std::string& name){
        vectors2d[name] = std::vector<std::pair<int, int>>();
        locks[name] = false;
    }

    void push_back(const std::string& name, int value) {
    // 给 vector 添加值
        if (vectors.find(name) != vectors.end() && !locks[name]) {
            vectors[name].push_back(value);
        }
    }

    void push_back(const std::string& name, int value1 ,int value2) {
    // 给 vector 添加值
        if (vectors2d.find(name) != vectors2d.end() && !locks[name]) {
            vectors2d[name].push_back(std::make_pair(value1, value2));
        }
    }

    // 给变量加上某个值
    void add(const std::string& name, int value) {
        if (variables.find(name) != variables.end() && !locks[name]) {
            variables[name] += value;
        }
    }

    // 给变量减去某个值
    void subtract(const std::string& name, int value) {
        if (variables.find(name) != variables.end() && !locks[name]) {
            variables[name] -= value;
        }
    }

    // 锁定变量
    void lock(const std::string& name) {
        if (variables.find(name) != variables.end()) {
            locks[name] = true;
        }
    }

    void max_put(const std::string& name, int value) {
        if (variables.find(name) != variables.end() && !locks[name]) {
            variables[name] = std::max(variables[name], value);
        }
    }

    void min_put(const std::string& name, int value) {
        if (variables.find(name) != variables.end() && !locks[name]) {
            variables[name] = std::min(variables[name], value);
        }
    }

    // 解锁变量
    void unlock(const std::string& name) {
        if (variables.find(name) != variables.end()) {
            locks[name] = false;
        }
    }

    // 获取变量的值
    int getValue(const std::string& name) {
        if (variables.find(name) != variables.end()) {
            return variables[name];
        }
        return 0; // 如果变量不存在，返回0
    }

    void writeToFile(const std::string& filename) {
        std::ofstream outFile(filename, std::ios::app);
        outFile << "============================================" << std::endl;
        for (const auto& var : variables) {
            outFile << "Variable: " << var.first << ",\t Value: " << var.second << ",\t Locked: " << (locks[var.first] ? "Yes" : "No") << std::endl;
        }
        outFile << "=====================" << std::endl;
        outFile << "avg_robot_move_length: " << (getValue("robot_move_length") / getValue("robot_get_nums")) << std::endl;
        outFile << "max_robot_move_length: " << getValue("robot_move_length_max") << std::endl;
        outFile << "min_robot_move_length: " << getValue("robot_move_length_min") << std::endl;
        outFile << "============================================" << std::endl;
        outFile.close();
        for (const auto& vec : vectors) {
            std::ofstream outFile(filename + "_" + vec.first + ".txt");
            for (int i = 0; i < vec.second.size(); i++) {
                outFile << vec.second[i] << std::endl;
            }
            outFile.close();
        }
        for (const auto& vec : vectors2d){
            std::ofstream outFile(filename + "_" + vec.first + ".txt");
            for (int i = 0; i < vec.second.size(); i++) {
                outFile << vec.second[i].first << " " << vec.second[i].second << std::endl;
            }
            outFile.close();
        }
    }
};

class Void_Counter {
public:
    void registerVariable(const std::string& name, int initialValue = 0) {return;}
    void registerVector(const std::string& name) {return;}
    void registerVector2D(const std::string& name)  {return;}
    void push_back(const std::string& name, int value1 ,int value2) {return;}
    void push_back(const std::string& name, int value) {return;}
    void add(const std::string& name, int value) {return;}
    void max_put(const std::string& name, int value) {return;}
    void min_put(const std::string& name, int value) {return;}
    void subtract(const std::string& name, int value) {return;}
    void lock(const std::string& name) {return;}
    void unlock(const std::string& name) {return;}
    int getValue(const std::string& name) {return 0;}
    void writeToFile(const std::string& filename) {return;}
};