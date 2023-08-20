#include "../source/server.hpp"

int main()
{
    Socket sock;
    sock.CreateClient(8085,"127.0.0.1");
    for(int i=0;i<5;i++){
        std::string str="hello world!";
        sock.Send(str.c_str(),str.size());
        char buf[1024]={0};
        sock.Recv(buf,1023);
        DBG_LOG("%s",buf);
        sleep(1);
    }
    while(1){}
    return 0;
}
