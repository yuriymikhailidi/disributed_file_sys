SERVER_DIR := server
CLIENT_DIR := client
BIN_DIR := .
SERVER_EXE := $(BIN_DIR)/dfs
CLIENT_EXE := $(BIN_DIR)/dfc
SERVER_SRC := $(wildcard $(SERVER_DIR)/*.c)
CLIENT_SRC := $(wildcard $(CLIENT_DIR)/*.c)
SERVER_OBJ := $(SERVER_SRC: $(SERVER_DIR)/%.c = $(SERVER_DIR)/%.o)
CLIENT_OBJ := $(CLIENT_SRC: $(CLIENT_DIR)/%.c = $(CLIENT_DIR)/%.o)

CPPFLAGS := -Iinclude -MMD -MP
CFLAGS := -g -Wall
LDLIBS := -lm
LDFLAGS := -Llib

all: $(SERVER_EXE)  $(CLIENT_EXE)

.PHONY: all clean

$(SERVER_EXE) : $(SERVER_OBJ) | $(BIN_DIR)
	$(CC) $^ $(LDLIBS) -o $@

$(CLIENT_EXE) : $(CLIENT_OBJ) | $(BIN_DIR)
	$(CC) $^ $(LDLIBS) -o $@

$(SERVER_OBJ)/%.o: $(SERVER_SRC)/%.c | $(SERVER_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

$(CLIENT_OBJ)/%.o: $(CLIENT_SRC)/%.c | $(CLIENT_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

clean:
	$(RM) -rv $(SERVER_DIR)/*.o $(CLIENT_DIR)/*.o ./dfs ./dfc

# Resource:
#https://stackoverflow.com/questions/30573481/how-to-write-a-makefile-with-separate-source-and-header-directories/30602701#30602701