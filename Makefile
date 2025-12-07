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

tests: socket.o rsa.o test.o
	$(CC) $(OBJECT)/socket.o $(OBJECT)/rsa.o $(OBJECT)/test.o -o $(BIN)/test

socket.o: $(INCLUDES)/socket.cpp $(INCLUDES)/socket.hpp 
	$(CC) $(CFLAGS) -c $(INCLUDES)/socket.cpp -o $(OBJECT)/socket.o

rsa.o: $(INCLUDES)/rsa.cpp $(INCLUDES)/rsa.hpp
	$(CC) $(CFLAGS) -c $(INCLUDES)/rsa.cpp -o $(OBJECT)/rsa.o

server.o: $(SRC)/server/server.cpp
	$(CC) $(CFLAGS) -c $< -o $(OBJECT)/server.o

client.o : $(SRC)/client/client.cpp
	$(CC) $(CFLAGS) -c $< -o $(OBJECT)/client.o

test.o: $(SRC)/tests/test.cpp
	$(CC) $(CFLAGS) -c $< -o $(OBJECT)/test.o

clean:
	rm -f $(BIN)/*
	rm -f $(OBJECT)/*