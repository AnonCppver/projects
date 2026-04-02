# 编译器
CXX := g++

# 编译选项
CXXFLAGS := -std=c++17 -O2

# 库路径
LIB_DIR := ../lib

# 链接选项
LDFLAGS := -L$(LIB_DIR) -Wl,-rpath=$(LIB_DIR)

# 库
LIBS := -lbase

# 源文件
SRCS := $(wildcard *.cpp)

# 可执行文件（直接 .o 后缀）
TARGETS := $(SRCS:.cpp=.o)

all: $(TARGETS)

# 直接从 cpp 生成“可执行文件.o”
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean