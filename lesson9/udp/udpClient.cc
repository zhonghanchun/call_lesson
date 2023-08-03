#include "udpClient.hpp"
#include <memory>
void Usage(string port)
{
    cout<<"Usage \n\t"<< port<<" server_ip server_port"<<endl;
}
int main(int argc,char* argv[])
{
    if(argc!=3)
    {
        Usage(argv[0]);
        exit(1);
    }
    string ip=argv[1];
    uint16_t port=atoi(argv[2]);
    std::unique_ptr<udpClient> p(new udpClient(ip,port));
    p->Init_Client();
    p->Run();
    return 0;
}
