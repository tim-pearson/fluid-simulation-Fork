#pragma once

#include <iostream>
#include <stdexcept>

#include <string>

inline struct TextColors {
    std::string ESC = "\033[";
    std::string RESET = ESC + "0m";
    std::string BOLD = ESC + "1m";
    std::string RED = ESC + "31m";
    std::string GREEN = ESC + "32m";
    std::string YELLOW = ESC + "33m";
    std::string BLUE = ESC + "34m";
    std::string ORANGE = ESC + "35m";
    std::string BLUE_BG = ESC + "44m";
} Text;

inline void warn(const std::string& message) {
    std::cerr << Text.BOLD << Text.YELLOW << "WARNING : " << Text.RESET << message << std::endl;
}

inline void error(const std::string& message) {
    std::cerr << Text.BOLD << Text.RED << "ERROR : " << Text.RESET << message << std::endl;
}