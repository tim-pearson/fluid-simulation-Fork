#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

inline void readFile(const std::string& filePath, std::string& content) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filePath << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    content = buffer.str();
    file.close();
}