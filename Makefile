CC = g++
# -Wall removed for now
CFLAGS = -Wextra -std=c++17
SRC_DIR = src
INC_DIR = include
BIN_DIR = bin
OBJ_DIR = obj
TARGET = main

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp) $(TARGET).cpp
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES))

$(BIN_DIR)/$(TARGET): $(OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

.PHONY: run runf clean

run: $(BIN_DIR)/$(TARGET)
	./bin/main

runf: $(BIN_DIR)/$(TARGET)
	./bin/main owo.kt

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
