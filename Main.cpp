#include "JsonParser.hpp"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>


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
        JsonObject& rootObj = std::get<JsonObject>(root.value);
        JsonValue& name = rootObj.at("name");
        std::string nameStr = std::get<std::string>(name.value);
        std::cout << "Name: " << nameStr << '\n';
        JsonValue& homelab = rootObj.at("homelab");
        JsonObject& homelabObj = std::get<JsonObject>(homelab.value);
        JsonValue& servers = homelabObj.at("servers");
        double serversCount = std::get<double>(servers.value);
        std::cout << "Servers: " << serversCount << '\n';
        JsonValue& online = homelabObj.at("online");
        bool onlineStatus = std::get<bool>(online.value);
        std::cout << "Online: " << (onlineStatus ? "true" : "false") << '\n';
        JsonValue& services = homelabObj.at("services");
        JsonArray& servicesArr = std::get<JsonArray>(services.value);
        std::cout << "Services: ";
        for (const auto& service : servicesArr) {
            std::string serviceName = std::get<std::string>(service.value);
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

