#include "jsonParser.hpp"
#include "Tokenizer.cpp"
#include "jsonValue.cpp"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>


int main() {
    std::string jsonMessage = R"(
    {
        "name": "Niko",
        "homelab": {
            "servers": 4,
            "online": true,
            "services": [
                "Proxmox",
                "Ollama",
                "Nginx"
            ]
        }
    }
    )"; //R for raw string

    try
    {
        JsonParser parser(jsonMessage);
        JsonValue root = parser.parse();
        std::cout << "Parsed JSON successfully!\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON Error: " << e.what() << '\n';
    }
    return 0;
}