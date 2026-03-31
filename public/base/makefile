# ========================
# 基本配置
# ========================
CXX = g++
CXXFLAGS = -std=c++17 -O2 -fPIC -Wall

# 目录
SRC_DIR = .
OBJ_DIR = ./bin
LIB_DIR = ../lib

# 库名
LIB_NAME = base

# ========================
# 文件列表
# ========================
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# 把 .cpp → bin/*.o
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# 输出文件
STATIC_LIB = $(LIB_DIR)/lib$(LIB_NAME).a
SHARED_LIB = $(LIB_DIR)/lib$(LIB_NAME).so

# ========================
# 默认目标
# ========================
all: $(STATIC_LIB) $(SHARED_LIB)

# ========================
# 编译 .o（关键修改点）
# ========================
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
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