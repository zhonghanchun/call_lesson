#include <iostream>
#include <fcntl.h>
#include <unistd.h>

void setNonBlock(int fd){
    int fl=fcntl(fd,F_GETFL);
    if(fl<0){
      std::cout<<"fcntl failed!"<<std::endl;
      return;
    }
    fcntl(fd,F_SETFL,fl|O_NONBLOCK);
}
void a(){
    std::cout<<"a ";
}
void b(){
    std::cout<<"b ";
}
void c(){
    std::cout<<"c"<<std::endl;
}
