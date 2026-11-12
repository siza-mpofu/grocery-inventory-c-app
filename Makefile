
CC = gcc
CXX = g++

CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror -g -O2
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -Werror -g -O2

INCDIR = include
SRCDIR = src
OBJDIR = build
TARGET = $(OBJDIR)/grocery_inventory

C_SOURCES = $(wildcard $(SRCDIR)/*.c)
CPP_SOURCES = $(wildcard $(SRCDIR)/*.cpp)

C_OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(C_SOURCES))
CPP_OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.opp,$(CPP_SOURCES))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(C_OBJECTS) $(CPP_OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

$(OBJDIR)/%.opp: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -Isrc -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)
