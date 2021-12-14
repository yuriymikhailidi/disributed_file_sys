CC = gcc
CFLAGS = -g -Wall

SERVER_TARGET = ./dfserver/dfserver
CLIENT_TARGET = ./dfclient/dfclient


all: dfs dfc

dfs:
	$(CC) $(CFLAGS) -o dfs $(SERVER_TARGET).o
dfc:
	$(CC) $(CFLAGS) -o dfc $(CLIENT_TARGET).o

$(SERVER_TARGET).o : $(SERVER_TARGET).c
	$(CC) $(CFLAGS) -o $(SERVER_TARGET) $(SERVER_TARGET).c

$(CLIENT_TARGET).o : $(CLIENT_TARGET).c
	$(CC) $(CFLAGS) -o $(CLIENT_TARTGET) $(CLIENT_TARTGET).c

clean:
	$(RM) dfs dfc *.o *~
