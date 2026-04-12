# ========================
# 编译器配置
# ========================
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -fPIC

# ========================
# 路径
# ========================
SRC_DIR = .
OBJ_DIR = ./bin
LIB_DIR = ../lib

# ========================
# 库名
# ========================
LIB_NAME = lrpc

# ========================
# 源文件
# ========================
SRCS := $(wildcard $(SRC_DIR)/*.cpp) \
        $(wildcard $(SRC_DIR)/*.cc)

# ========================
# 目标文件列表
# ========================
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(filter %.cpp,$(SRCS))) \
        $(patsubst $(SRC_DIR)/%.cc,$(OBJ_DIR)/%.o,$(filter %.cc,$(SRCS)))

# ========================
# 输出库
# ========================
STATIC_LIB = $(LIB_DIR)/lib$(LIB_NAME).a
SHARED_LIB = $(LIB_DIR)/lib$(LIB_NAME).so

# ========================
# 默认目标
# ========================
all: $(STATIC_LIB) $(SHARED_LIB)

# ========================
# 编译 cpp -> o
# ========================
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ========================
# 编译 cc -> o
# ========================
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ========================
# 静态库
# ========================
$(STATIC_LIB): $(OBJS)
	@mkdir -p $(LIB_DIR)
	ar rcs $@ $^

# ========================
# 动态库
# ========================
$(SHARED_LIB): $(OBJS)
	@mkdir -p $(LIB_DIR)
	$(CXX) -shared -o $@ $^

# ========================
# 清理
# ========================
clean:
	rm -rf $(OBJ_DIR)
	rm -f $(STATIC_LIB) $(SHARED_LIB)

# ========================
# 重新构建
# ========================
rebuild: clean all

.PHONY: all clean rebuild