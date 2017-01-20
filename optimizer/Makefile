export BUILD_DIR := build
export INCLUDE_DIR := include
export PCG_DIR := dist/pcg-cpp
export OPTFLAGS := -Ofast -flto
export COMPILER_FLAGS := -std=c++14 -march=native $(OPTFLAGS) -fno-exceptions -fno-rtti -Wall -Wextra -Werror -pedantic -Wshadow -Wmissing-include-dirs -Winvalid-pch -Wformat=2
CXXFLAGS := $(COMPILER_FLAGS) -DPCG_USE_INLINE_ASM -I$(INCLUDE_DIR) -I$(PCG_DIR)/include
export LDEXTRA := -fuse-ld=gold
SUBDIRS := b3test experiment optimizer test

ifeq ($(CXX), g++)
	AR := gcc-ar
else ifeq ($(CXX), clang++)
	AR := llvm-ar
endif

all: subdirs

subdirs: $(SUBDIRS)

$(SUBDIRS): $(BUILD_DIR)
	$(MAKE) -C $@

$(BUILD_DIR):
	@mkdir $@

clean:
	for dir in $(SUBDIRS); do $(MAKE) -C $$dir clean; done
	$(RM) -d $(BUILD_DIR) 2> /dev/null || true

.PHONY: all clean $(SUBDIRS)

-include $(DEP)
