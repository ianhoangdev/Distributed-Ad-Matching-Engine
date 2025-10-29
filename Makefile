CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2 -Iinclude -Ithird_party
TARGET = ad_matching_engine
SRCDIR = src
SOURCES = $(SRCDIR)/main.cpp $(SRCDIR)/matcher.cpp $(SRCDIR)/global.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

.PHONY: all clean

