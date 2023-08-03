.PHONE:all
all:shm_client shm_server

shm_client:shm_client.cc
	g++ -o $@ $^ -std=c++11
shm_server:shm_server.cc
	g++ -o $@ $^ -std=c++11
.PHONE:clean
clean:
	rm -rf shm_client shm_server