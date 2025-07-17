CC = gcc
CFLAGS = -Wall -Wextra -std=c2x -O2
LDFLAGS =
INCLUDES = -Iinclude

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
TARGET = $(BIN_DIR)/kasd

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Run tests
test: $(TARGET)
	@echo "Running tests..."
	@echo "let x: int = 42;" > test.kasd
	@echo "let y: float = 3.14;" >> test.kasd
	@echo "let s: string = \"Hello, KASD!\";" >> test.kasd
	@echo "let b: bool = true;" >> test.kasd
	@$(TARGET) test.kasd
	@rm test.kasd

# Run with debug logging
debug: $(TARGET)
	$(TARGET) --log-level 4

# Install
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Uninstall
uninstall:
	rm -f /usr/local/bin/kasd 