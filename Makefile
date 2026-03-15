# Makefile for the key-value store project

CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

TARGET = kvstore
SRC = main.cpp

# Default target: build the executable
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

# Remove build artifacts
clean:
	rm -f $(TARGET) data.db

.PHONY: all clean
