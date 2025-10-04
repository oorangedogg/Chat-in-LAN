# 编译器与选项
CC      := gcc
CFLAGS  := -Wall -g 

# 目录
SRCDIR_server  := src/server/src
SRCDIR_client  := src/client/src

BUILDDIR := build
BINDIR  := bin


# 可执行文件名称
SERVER	:= $(BINDIR)/server
CLIENT 	:= $(BINDIR)/client

# 源文件与目标文件
SERVER_SRCS    := $(SRCDIR_server)/server.c
CLIENT_SRCS    := $(SRCDIR_client)/client.c

SERVER_OBJS := $(patsubst $(SRCDIR_server)/%.c,$(BUILDDIR)/%.o,$(SERVER_SRCS))
CLIENT_OBJS := $(patsubst $(SRCDIR_client)/%.c,$(BUILDDIR)/%.o,$(CLIENT_SRCS))



# 默认目标
all: $(SERVER) $(CLIENT)


# 链接生成最终可执行文件
$(SERVER): $(SERVER_OBJS) | $(BINDIR)
	$(CC) $(CFLAGS) $^ -o $@

$(CLIENT): $(CLIENT_OBJS) | $(BINDIR)
	$(CC) $(CFLAGS) $^ -o $@


# 编译规则：把 src/*.c 生成 build/*.o
$(BUILDDIR)/%.o: $(SRCDIR_server)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILDDIR)/%.o: $(SRCDIR_client)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@



# 确保目录存在
$(BINDIR) $(BUILDDIR):
	mkdir -p $@

# 清理
.PHONY: clean
clean:
	rm -rf $(BUILDDIR) $(BINDIR)
