LIBNAME ?= liboptimizer.a
BUILD_DIR ?= build
INCLUDE_DIR ?= include
LIB_DIR ?= lib
SRC_DIR := src
OPTFLAGS ?= -Ofast -flto
COMPILER_FLAGS ?= -std=c++14 -march=native -mtune=native $(OPTFLAGS) -fno-exceptions -fno-rtti -Wall -pedantic -Wshadow
LDLIBS ?= -l$(LIBNAME:lib%.a=%)
BUILD_DIR := ../$(BUILD_DIR)
INCLUDE_DIR := ../$(INCLUDE_DIR)
LIB_DIR := ../$(LIB_DIR)
CXXFLAGS = $(COMPILER_FLAGS) -I$(INCLUDE_DIR)
LDFLAGS := $(OPTFLAGS) -L$(LIB_DIR)
SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEP := $(SRC:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.d)
TARGET := $(SRC:$(SRC_DIR)/%.cpp=%)

all: $(TARGET)

%: $(BUILD_DIR)/%.o
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(LIB_DIR)/$(LIBNAME)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(LIB_DIR)/$(LIBNAME):
	$(MAKE) -C .. $(LIBNAME:%.a=%)

clean:
	$(RM) $(OBJ) $(TARGET) $(DEP)

.SECONDARY: $(OBJ)

.PHONY: all clean

-include $(DEP)
