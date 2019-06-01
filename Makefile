# Makefile
#

#编译标志
CC = gcc
DEBUG_SERVER = -g -O2 -Wall 
DEBUG_CLIENT = -g -O2 -Wall
#COMPILE = -lpthread
CFLAGS += $(DEBUG)
EX_FLAGS += $(COMPILE)

#编译选项
KCP_INC_DIR += -I./
INCLUDE = $(KCP_INC_DIR)

#LIBS +=
KCP_COMMON_SRC = \
          ./ikcp.c \
		  ./common.c
KCP_SERVER_SRC = \
          ./kcp_server.c 
KCP_CLIENT_SRC = \
           ./kcp_client.c
SRC = $(KCP_COMMON_SRC)
SERVER_SRC = $(KCP_SERVER_SRC)
CLIENT_SRC = $(KCP_CLIENT_SRC)

SERVER_TARGET = KCP_SERVER
CLIENT_TARGET = KCP_CLIENT


#编译 
server :
	$(CC) $(DEBUG_SERVER) $(INCLUDE) -o $(SERVER_TARGET) $(SRC) $(SERVER_SRC) -D DBUG_SERVER=1
client:
	$(CC) $(DEBUG_CLIENT) $(INCLUDE) -o $(CLIENT_TARGET) $(SRC) $(CLIENT_SRC) -D DBUG_CLIENT=1

.PHONY: clean 
	
clean : 
	-rm -f  KCP_*
