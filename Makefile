SRC_DIR = src
INCLUDE_DIR = include
OBJ_DIR = obj
BIN_DIR = bin

COMMON_SRCS = $(wildcard $(SRC_DIR)/common.c)
BLOG_SRCS = $(wildcard $(SRC_DIR)/blog.c)
CLIENT_SRCS = $(wildcard $(SRC_DIR)/client.c)
SERVER_SRCS = $(wildcard $(SRC_DIR)/server.c)

COMMON_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(COMMON_SRCS))
BLOG_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(BLOG_SRCS))
CLIENT_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(CLIENT_SRCS))
SERVER_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SERVER_SRCS))

INCLUDES = -I$(INCLUDE_DIR)

CC = gcc
CFLAGS = -Wall -pthread

CLIENT_EXEC = $(BIN_DIR)/client
SERVER_EXEC = $(BIN_DIR)/server

all: $(CLIENT_EXEC) $(SERVER_EXEC)

$(OBJ_DIR)/common.o: $(COMMON_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $^ -o $@

$(OBJ_DIR)/blog.o: $(BLOG_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $^ -o $@

$(OBJ_DIR)/client.o: $(CLIENT_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $^ -o $@

$(OBJ_DIR)/server.o: $(SERVER_SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $^ -o $@

$(CLIENT_EXEC): $(CLIENT_OBJS) $(COMMON_OBJS) $(BLOG_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(SERVER_EXEC): $(SERVER_OBJS) $(COMMON_OBJS) $(BLOG_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(OBJ_DIR)/*.o
	rm -rf $(BIN_DIR)/*

run-client: $(CLIENT_EXEC)
	./$<

run-server: $(SERVER_EXEC)
	./$<
