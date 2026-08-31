#include "JsonParser.hpp"
#include "JsonValue.hpp"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>

void test();
void test2(std::string fileName);
std::string readFile(const std::string& filename);


int main() {
    test(); 
    return 0;
}


void test() {
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
        const JsonObject rootObj = root.getObject();
        JsonValue name = rootObj.at("name");
        std::string nameStr = name.getString();
        std::cout << "Name: " << nameStr << '\n';
        JsonValue homelab = rootObj.at("homelab");
        const JsonObject homelabObj = homelab.getObject();
        JsonValue servers = homelabObj.at("servers");
        double serversCount = servers.getDouble();
        std::cout << "Servers: " << serversCount << '\n';
        JsonValue online = homelabObj.at("online");
        bool onlineStatus = online.getBool();
        std::cout << "Online: " << (onlineStatus ? "true" : "false") << '\n';
        JsonValue services = homelabObj.at("services");
        JsonArray servicesArr = services.getArray();
        std::cout << "Services: ";
        for (const auto& service : servicesArr) {
            std::string serviceName = service.getString();
            std::cout << serviceName << " ";
        }
        std::cout << '\n';
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON Error: " << e.what() << '\n';
    }
}


void test2(std::string fileName) {
    try 
    {
        std::string jsonMessage = readFile(fileName);

        if (jsonMessage.empty()) {
            std::cerr << "Failed to read JSON content from file" << '\n';
            return;
        }
    
        JsonParser parser(jsonMessage);
        JsonValue root = parser.parse();
        std::cout << "Parsed JSON from file successfully!\n";
        
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON Error: " << e.what() << '\n';
    }
}


std::string readFile(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Failed to open or read file" << '\n';
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

