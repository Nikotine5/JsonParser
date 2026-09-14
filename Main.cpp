#include "JsonParser.hpp"
#include "JsonValue.hpp"
#include "Buffer.hpp"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cerrno>
#include <fcntl.h>
#include <system_error>
#include <unistd.h>

void test();
int main() {
    test(); 
    return 0;
}


void test() {
    try
    {
        std::ifstream file("test.json");
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: test.json");
        }
        JsonParser parser(file);
        JsonValue root = parser.parse();
        root.toString();
        std::cout << "Parsed JSON: " << root.toString() << '\n';
        file.close();
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON Error: " << e.what() << '\n';
    }


}
