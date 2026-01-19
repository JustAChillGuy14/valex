CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -MMD -MP -Iheaders

SRC_DIR = src
OBJ_DIR = objects

FRONTEND_DIR = frontend
RUNTIME_DIR  = runtime

TARGET = main

SOURCES = $(wildcard $(SRC_DIR)/$(FRONTEND_DIR)/*.c) \
          $(wildcard $(SRC_DIR)/$(RUNTIME_DIR)/*.c) \
          $(SRC_DIR)/$(TARGET).c


OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(OBJECTS:.o=.d)

.PHONY: clean
clean:
	rm -rf $(OBJ_DIR) $(TARGET)
