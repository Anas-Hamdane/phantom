TARGET := phantom

CXX := clang++
CXXFLAGS := -g -std=c++17 -Wall -Wextra -I./include/
LFLAGS := -static

BUILD := build
OBJSDIR := build/objs

FRONTEND := ./src/Lexer.cpp ./src/Parser.cpp ./src/Driver.cpp
UTILS := ./src/Logger.cpp ./src/utils/NumUtils.cpp
AST := ./src/ast/Expr.cpp ./src/ast/Stmt.cpp
SRCS := ./src/main.cpp $(FRONTEND) $(UTILS) $(AST)

# IMPORTANT: don't forget to add new paths here too
VPATH := src src/utils src/ast

SRC_WITHOUT_PATH := $(notdir $(SRCS))
OBJS := $(addprefix $(OBJSDIR)/, $(SRC_WITHOUT_PATH:.cpp=.o))

.PHONY: all clean

all: clean $(BUILD)/$(TARGET)

$(BUILD)/$(TARGET): $(OBJS) | $(BUILD)
	$(CXX) $(OBJS) -o $@ $(LFLAGS)

$(OBJSDIR)/%.o: %.cpp | $(OBJSDIR)
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(OBJSDIR): | $(BUILD)
	mkdir -p $(OBJSDIR)

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)
