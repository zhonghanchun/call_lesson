.PHONY:all

all:tcpServer tcpClient

tcpServer:tcpServer.cc 
	g++ -o $@ $^ -std=c++11 -lpthread
tcpClient:tcpClient.cc 
	g++ -o $@ $^ -std=c++11 -lpthread 

.PHONY:clean 
clean:
	rm -rf tcpServer tcpClient 
