.PHONY:all

all:tcpServer tcpClient

tcpServer:tcpServer.cc 
	g++ -o $@ $^ -std=c++11 -lpthread -ljsoncpp
tcpClient:tcpClient.cc 
	g++ -o $@ $^ -std=c++11 -lpthread -ljsoncpp

.PHONY:clean 
clean:
	rm -rf tcpServer tcpClient 
