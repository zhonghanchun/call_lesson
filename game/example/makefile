#wsserver:wsserver.cpp
#	g++ -o $@ $^ -std=c++11 -lpthread -lboost_system
mysql:mysql.c
	gcc -o $@ $^  -L/usr/lib64/mysql/ -lmysqlclient

.PHONY:clean
clean: 
	rm -rf wsserver mysql
