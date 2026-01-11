CC := g++
CFLAGS = -std=c++17 -w -g -I$(INCLUDES)
SRC := source
OBJECT := build/object
BIN := build/bin
INCLUDES := source/includes



all: server client attacker

client: socket.o client.o rsa.o cmd.o
	$(CC) $(OBJECT)/socket.o $(OBJECT)/client.o $(OBJECT)/rsa.o $(OBJECT)/cmd.o -o $(BIN)/client

server: socket.o server.o rsa.o cmd.o
	$(CC) $(OBJECT)/socket.o $(OBJECT)/server.o $(OBJECT)/rsa.o $(OBJECT)/cmd.o -o $(BIN)/server

attacker: socket.o attacker.o rsa.o cmd.o
	$(CC) $(OBJECT)/socket.o $(OBJECT)/attacker.o $(OBJECT)/rsa.o $(OBJECT)/cmd.o -o $(BIN)/attacker


tests: socket.o rsa.o test.o cmd.o
	$(CC) $(OBJECT)/socket.o $(OBJECT)/rsa.o $(OBJECT)/test.o $(OBJECT)/cmd.o -o $(BIN)/test

socket.o: $(INCLUDES)/socket.cpp $(INCLUDES)/socket.hpp 
	$(CC) $(CFLAGS) -c $(INCLUDES)/socket.cpp -o $(OBJECT)/socket.o

rsa.o: $(INCLUDES)/rsa.cpp $(INCLUDES)/rsa.hpp
	$(CC) $(CFLAGS) -c $(INCLUDES)/rsa.cpp -o $(OBJECT)/rsa.o

cmd.o: $(INCLUDES)/cmd.cpp $(INCLUDES)/cmd.hpp
	$(CC) $(CFLAGS) -c $(INCLUDES)/cmd.cpp -o $(OBJECT)/cmd.o

server.o: $(SRC)/server/server.cpp
	$(CC) $(CFLAGS) -c $< -o $(OBJECT)/server.o

client.o : $(SRC)/client/client.cpp
	$(CC) $(CFLAGS) -c $< -o $(OBJECT)/client.o

attacker.o: $(SRC)/attacker/attacker.cpp
	$(CC) $(CFLAGS) -c $< -o $(OBJECT)/attacker.o


test.o: $(SRC)/tests/test.cpp
	$(CC) $(CFLAGS) -c $< -o $(OBJECT)/test.o

clean:
	rm -f $(BIN)/*
	rm -f $(OBJECT)/*