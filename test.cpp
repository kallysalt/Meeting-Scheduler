#include <iostream>
#include <sstream>
#include <string>

int main() {
    std::string input = "12,34,56,78,90";
    std::stringstream ss(input); // Initialize the stringstream with the input string

    int value;

    // Extract integers from the stringstream
    while (ss >> value) {
        std::cout << "Extracted value: " << value << std::endl;
    }

    return 0;
}