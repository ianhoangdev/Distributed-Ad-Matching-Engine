CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2
TARGET = ad_matching_engine
SOURCE = main.cpp

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)

.PHONY: all clean

