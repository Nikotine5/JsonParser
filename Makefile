CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -Wpedantic -g
CPPFLAGS = -Ih

TARGET = Parser
SOURCES = Main.cpp src/JsonParser.cpp src/Tokenizer.cpp src/JsonValue.cpp
HEADERS = h/JsonParser.hpp h/Tokenizer.hpp h/JsonValue.hpp

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(SOURCES) -o $@

clean:
	rm -f $(TARGET)
