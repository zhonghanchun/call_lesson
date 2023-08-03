#include "../source/server.hpp"

int main()
{
    Socket sock;
    sock.CreateServer(8085,"127.0.0.1",false);
    while(1){
        int newfd=sock.Accept();
        if(newfd<0)continue;
        Socket cli_sock(newfd);
        char buf[1024]={0};
        int ret=cli_sock.Recv(buf,1023);
        if(ret<0){
            cli_sock.Close();
            continue;
        }
        cli_sock.Send(buf,ret);
        cli_sock.Close();
    }
    return 0;
}
