
#ifndef UI_INPUT_HPP
#define UI_INPUT_HPP

#include <string>

// Read and validate user input.
bool readInt(const char *prompt, int &out);
bool readDouble(const char *prompt, double &out);
int requireInt(const char *prompt);
double requireDouble(const char *prompt);
std::string readText(const char *prompt);

#endif
