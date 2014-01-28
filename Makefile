all: client server

client:
	g++ -o client main.cpp -lSDL2 -lSDL2_net -std=c++11

server:
	g++ -o server server.cpp -lSDL2 -lSDL2_net -std=c++11

clean:
	rm client server
