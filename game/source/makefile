gobang:gobang.cc
	g++ -o $@ $^ -std=c++11 -lpthread -L/usr/lib64/mysql/ -lmysqlclient -ljsoncpp -lboost_system -lpthread 

.PHONY:clean
clean:
	rm -rf gobang
