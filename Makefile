CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = go
SRCS = main.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)

.PHONY: all run clean

