
#include "ui_input.hpp"

#include <cstdlib>
#include <cctype>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>

bool readInt(const char *prompt, int &out)
{
    std::string input;

    while (true) {
        std::cout << prompt;

        if (!std::getline(std::cin, input))
            return false;

        char *end = nullptr;
        long value = std::strtol(input.c_str(), &end, 10);

        while (end && std::isspace(static_cast<unsigned char>(*end)))
            ++end;

        if (end != input.c_str() && end && *end == '\0' &&
            value >= std::numeric_limits<int>::min() &&
            value <= std::numeric_limits<int>::max()) {

            out = static_cast<int>(value);
            return true;
        }

        std::cout << "Enter a valid whole number.\n";
    }
}

bool readDouble(const char *prompt, double &out)
{
    std::string input;

    while (true) {
        std::cout << prompt;

        if (!std::getline(std::cin, input))
            return false;

        char *end = nullptr;
        double value = std::strtod(input.c_str(), &end);

        while (end && std::isspace(static_cast<unsigned char>(*end)))
            ++end;

        if (end != input.c_str() && end && *end == '\0' &&
            std::isfinite(value) &&
            value >= 0.0 && value <= 1.0e12) {

            out = value;
            return true;
        }

        std::cout << "Enter a valid non-negative amount.\n";
    }
}

// These wrappers distinguish EOF from valid numeric input.
int requireInt(const char *prompt)
{
    int value;

    if (!readInt(prompt, value)) {
        std::cout << "\nInput ended. Exiting application.\n";
        std::exit(EXIT_SUCCESS);
    }

    return value;
}

double requireDouble(const char *prompt)
{
    double value;

    if (!readDouble(prompt, value)) {
        std::cout << "\nInput ended. Exiting application.\n";
        std::exit(EXIT_SUCCESS);
    }

    return value;
}

std::string readText(const char *prompt)
{
    std::string input;

    while (true) {
        std::cout << prompt;

        if (!std::getline(std::cin, input)) {
            std::cout << "\nInput ended. Exiting application.\n";
            std::exit(EXIT_SUCCESS);
        }

        if (!input.empty() &&
            input.find('|') == std::string::npos) {
            return input;
        }

        std::cout << "Text cannot be empty or contain |.\n";
    }
}
