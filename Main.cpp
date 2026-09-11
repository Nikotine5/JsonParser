#include "JsonParser.hpp"
#include "JsonValue.hpp"
#include "Buffer.hpp"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>

void test();
void test2(std::string fileName);
std::string readFile(const std::string& filename);
struct Fd {
    int fd;
    explicit Fd(const char* p) : fd(::open(p, O_RDONLY)) {
        if (fd < 0) {
            throw std::system_error(errno, std::generic_category(), p);
        }
    }
    ~Fd() {if (fd >= 0) ::close(fd); }
    Fd(const Fd&) = delete;
    Fd& operator=(const Fd&) = delete;
};


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
        JsonObject& rootObj = root.getObject();
        JsonValue& name = rootObj.at("name");
        std::string nameStr = name.getString();
        std::cout << "Name: " << nameStr << '\n';
        JsonValue& homelab = rootObj.at("homelab");
        JsonObject& homelabObj = homelab.getObject();
        JsonValue& servers = homelabObj.at("servers");
        double serversCount = servers.getDouble();
        std::cout << "Servers: " << serversCount << '\n';
        JsonValue& online = homelabObj.at("online");
        bool onlineStatus = online.getBool();
        std::cout << "Online: " << (onlineStatus ? "true" : "false") << '\n';
        JsonValue& services = homelabObj.at("services");
        JsonArray& servicesArr = const_cast<JsonArray&>(services.getArray());
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
    Fd file = ::open(filename.c_str(), O_RDONLY);
    if (file.fd < 0) {
        std::cerr << "Failed to open file" << '\n';
        return "";
    }

    Buffer buffer(file.fd);

    

    // file is closed automatically by its destructor

    // The buffer already contains the file content from buffer.read()
    return std::move(buffer.str());
}

std::string writeToFile(const std::string& filename, const JsonValue& content) {
    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Failed to open or write to file" << '\n';
        return "";
    }



    return "success";
}
