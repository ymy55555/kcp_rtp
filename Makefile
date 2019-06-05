# Makefile
#

#编译标志
CC = gcc
DEBUG_SERVER = -g -O2 -Wall 
DEBUG_CLIENT = -g -O2 -Wall

SERVER_LINK =  -luuid -D DBUG_SERVER=1
CLIENT_LINK =  -luuid -D DBUG_CLIENT=1
#COMPILE = -lpthread
CFLAGS += $(DEBUG)
EX_FLAGS += $(COMPILE)

#编译选项
KCP_INC_DIR += -I./include/
INCLUDE = $(KCP_INC_DIR)

#LIBS +=
KCP_COMMON_SRC = \
          ./src/ikcp.c \
		  ./src/common.c \
		  ./src/cirqueue.c
KCP_SERVER_SRC = \
          ./src/kcp_server.c 
KCP_CLIENT_SRC = \
           ./src/kcp_client.c
SRC = $(KCP_COMMON_SRC)
SERVER_SRC = $(KCP_SERVER_SRC)
CLIENT_SRC = $(KCP_CLIENT_SRC)

SERVER_TARGET = KCP_SERVER
CLIENT_TARGET = KCP_CLIENT


#编译 
server :
	$(CC) $(DEBUG_SERVER) $(INCLUDE) -o $(SERVER_TARGET) $(SRC) $(SERVER_SRC) $(SERVER_LINK)
client:
	$(CC) $(DEBUG_CLIENT) $(INCLUDE) -o $(CLIENT_TARGET) $(SRC) $(CLIENT_SRC) $(CLIENT_LINK)

.PHONY: clean 
	
clean : 
	-rm -f  KCP_*
