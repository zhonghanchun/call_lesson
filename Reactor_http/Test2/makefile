all:server client

server:server.cc
	g++ -o $@ $^ -std=c++11 -lpthread -g
client:client.cc 
	g++ -o $@ $^ -std=c++11

.PHONY:clean
clean:
	rm -rf server client
