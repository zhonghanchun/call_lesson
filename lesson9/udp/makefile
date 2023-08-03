.PHONY:all 

all:udpServer udpClient

udpServer:udpServer.cc
	g++ -o $@ $^ -std=c++11 -lpthread
udpClient:udpClient.cc 
	g++ -o $@ $^ -std=c++11 -lpthread 

PHONE:clean
clean:
	rm -rf udpClient udpServer
