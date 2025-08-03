CXX        := clang++
CXXFLAGS   := -g -std=c++17 -Wall -Wextra -static -I./include/

SRC        := src
BUILD      := build
TARGET     := $(BUILD)/phantom

SOURCES := $(SRC)/main.cpp \
           $(SRC)/common.cpp \
           $(SRC)/Lexer.cpp \
           $(SRC)/Driver.cpp \
           $(SRC)/Logger.cpp \
           $(SRC)/ast/Parser.cpp \
           $(SRC)/utils/num.cpp \
           $(SRC)/utils/str.cpp \
           $(SRC)/irgen/Gen.cpp \
           $(SRC)/codegen/Codegen.cpp

OBJECTS := $(SOURCES:$(SRC)/%.cpp=$(BUILD)/%.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET)

$(BUILD)/%.o: $(SRC)/%.cpp | $(BUILD)
	@mkdir -p $(dir $@)
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)
