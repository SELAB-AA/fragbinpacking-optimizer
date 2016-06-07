export LIBNAME := liboptimizer.a
LIB := $(LIBNAME:%.a=%)
export BUILD_DIR := build
export INCLUDE_DIR := include
export LIB_DIR := lib
SRC_DIR := src
export OPTFLAGS := -Ofast -flto
export COMPILER_FLAGS := -std=c++14 -march=native -mtune=native $(OPTFLAGS) -fno-exceptions -fno-rtti -Wall -pedantic -Wshadow
CXXFLAGS := $(COMPILER_FLAGS) -I$(INCLUDE_DIR)
LDFLAGS := $(OPTFLAGS) -L$(LIB_DIR)
export LDLIBS := -l$(LIBNAME:lib%.a=%)
SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEP := $(SRC:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.d $(BUILD_DIR)/$(BIN).d)
SUBDIRS := b3test experiment optimizer test

ifeq ($(CXX), g++)
	AR := gcc-ar
#else ifeq ($(CXX), clang++)
#	AR := llvm-ar
endif

all: $(LIB) subdirs

$(LIB): $(LIB_DIR)/$(LIBNAME)

$(LIB_DIR)/$(LIBNAME): $(OBJ) | $(LIB_DIR)
	$(AR) $(ARFLAGS) $@ $(OBJ)

$(LIB_DIR):
	@mkdir $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR):
	@mkdir $@

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

clean:
	for dir in $(SUBDIRS); do $(MAKE) -C $$dir clean; done
	$(RM) $(OBJ) $(DEP) $(LIB_DIR)/$(LIBNAME)
	$(RM) -d $(BUILD_DIR) 2> /dev/null || true
	$(RM) -d $(LIB_DIR) 2> /dev/null || true

.PHONY: all clean $(LIB) $(SUBDIRS)

-include $(DEP)
