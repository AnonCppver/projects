CXX = g++
CXXFLAGS = -std=c++17 -fPIC -Iinclude

SRC_DIR = src
BUILD_DIR = build
LIB_DIR = lib

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

.SECONDARY: $(OBJS)

STATIC_LIBS = $(patsubst $(BUILD_DIR)/%.o,$(LIB_DIR)/lib%.a,$(OBJS))
SHARED_LIBS = $(patsubst $(BUILD_DIR)/%.o,$(LIB_DIR)/lib%.so,$(OBJS))

all: $(STATIC_LIBS) $(SHARED_LIBS)

# 生成 .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 生成静态库
$(LIB_DIR)/lib%.a: $(BUILD_DIR)/%.o
	mkdir -p $(LIB_DIR)
	ar rcs $@ $<

# 生成动态库
$(LIB_DIR)/lib%.so: $(BUILD_DIR)/%.o
	mkdir -p $(LIB_DIR)
	$(CXX) -shared $< -o $@

clean:
	rm -rf $(BUILD_DIR)/*.o $(LIB_DIR)/*.a $(LIB_DIR)/*.so