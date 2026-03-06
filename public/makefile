# =============================
# 基础配置区（可复用）
# =============================

CXX      := g++
AR       := ar
ARFLAGS  := rcs

CXXSTD   := -std=c++17
CXXFLAGS := -Wall -Wextra -g $(CXXSTD)

SRC_DIR   := src
INC_DIR   := include
BUILD_DIR := build
LIB_DIR   := lib

STATIC_LIB := libstrUtil.a libtimeUtil.a
SHARED_LIB := libstrUtil.so libtimeUtil.so

INCLUDES := -I$(INC_DIR)
LIBS     :=
LIB_DIRS :=

# =============================
# 自动收集源文件
# =============================

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# =============================
# 默认目标
# =============================

all: static shared test

static: $(LIB_DIR)/$(STATIC_LIB)
shared: $(LIB_DIR)/$(SHARED_LIB)

test: main.cpp $(LIB_DIR)/$(STATIC_LIB)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIB_DIR)/$(STATIC_LIB)

# =============================
# 静态库
# =============================

$(LIB_DIR)/$(STATIC_LIB): $(OBJS)
	@mkdir -p $(LIB_DIR)
	$(AR) $(ARFLAGS) $@ $^

# =============================
# 动态库
# =============================

$(LIB_DIR)/$(SHARED_LIB): $(OBJS)
	@mkdir -p $(LIB_DIR)
	$(CXX) -shared -o $@ $^

# =============================
# 编译规则
# =============================

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -fPIC $(INCLUDES) -c $< -o $@

# =============================
# 清理
# =============================

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(LIB_DIR)
	rm test

.PHONY: all static shared clean