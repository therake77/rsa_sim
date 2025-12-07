CC := g++
CFLAGS = -std=c++17 -w -I$(INCLUDES)
SRC := source
OBJECT := build/object
BIN := build/bin
INCLUDES := source/includes



all: server client

client: socket.o client.o
	$(CC) $(OBJECT)/socket.o $(OBJECT)/client.o -o $(BIN)/client

server: socket.o server.o
	$(CC) $(OBJECT)/socket.o $(OBJECT)/server.o -o $(BIN)/server

socket.o: $(INCLUDES)/socket.cpp $(INCLUDES)/socket.hpp 
	$(CC) $(CFLAGS) -c $(INCLUDES)/socket.cpp -o $(OBJECT)/socket.o

server.o: $(SRC)/server/server.cpp
	$(CC) $(CFLAGS) -c $< -o $(OBJECT)/server.o

client.o : $(SRC)/client/client.cpp
	$(CC) $(CFLAGS) -c $< -o $(OBJECT)/client.o

clean:
	rm -f $(BIN)/*
	rm -f $(OBJECT)/*