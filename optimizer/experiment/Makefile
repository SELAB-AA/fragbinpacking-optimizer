BUILD_DIR ?= build
INCLUDE_DIR ?= include
PCG_DIR ?= dist/pcg-cpp
SRC_DIR := src
OPTFLAGS ?= -Ofast -flto
LDEXTRA ?= -fuse-ld=gold
COMPILER_FLAGS ?= -std=c++14 -march=native $(OPTFLAGS) -fno-exceptions -fno-rtti -Wall -Wextra -Werror -pedantic -Wshadow -Wmissing-include-dirs -Winvalid-pch -Wformat=2
BUILD_DIR := ../$(BUILD_DIR)
INCLUDE_DIR := ../$(INCLUDE_DIR)
PCG_DIR := ../$(PCG_DIR)
CXXFLAGS = $(COMPILER_FLAGS) -DPCG_USE_INLINE_ASM -I$(INCLUDE_DIR) -I$(PCG_DIR)/include
LDFLAGS = $(OPTFLAGS) $(LDEXTRA)
SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEP := $(SRC:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.d)
TARGET := $(SRC:$(SRC_DIR)/%.cpp=%)

all: $(TARGET)

%: $(BUILD_DIR)/%.o
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

clean:
	$(RM) $(OBJ) $(TARGET) $(DEP)

.SECONDARY: $(OBJ)

.PHONY: all clean

-include $(DEP)
