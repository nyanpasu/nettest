#CXX=clang++
CXX=g++
FLAGS=-O2 -Wall -lSDL2 -lSDL2_net -std=c++11
all: client server

client:
	$(CXX) -o bin/client Client/main.cpp $(FLAGS)

server:
	$(CXX) -o bin/server Server/main.cpp $(FLAGS)

clean:
	rm bin/client bin/server
