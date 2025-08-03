CXX        := clang++
CXXFLAGS   := -g -std=c++17 -Wall -Wextra -static -I./include/

SRC        := src
BUILD      := build
TARGET     := $(BUILD)/phantom

SOURCES = $(SRC)/main.cpp \
          $(SRC)/common.cpp \
          $(SRC)/Lexer.cpp \
          $(SRC)/Driver.cpp \
          $(SRC)/Logger.cpp \
          $(SRC)/ast/Parser.cpp \
          $(SRC)/utils/num.cpp \
          $(SRC)/utils/str.cpp \
          $(SRC)/irgen/Gen.cpp \
          $(SRC)/codegen/Codegen.cpp

OBJECTS = $(BUILD)/main.o \
          $(BUILD)/common.o \
          $(BUILD)/Lexer.o \
          $(BUILD)/Driver.o \
          $(BUILD)/Logger.o \
          $(BUILD)/Parser.o \
          $(BUILD)/num.o \
          $(BUILD)/str.o \
          $(BUILD)/Gen.o \
          $(BUILD)/Codegen.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o$(TARGET)

$(BUILD)/%.o: src/%.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(BUILD)/%.o: src/ast/%.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(BUILD)/%.o: src/utils/%.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(BUILD)/%.o: src/irgen/%.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(BUILD)/%.o: src/codegen/%.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

clean:
	rm -rf $(BUILD)
