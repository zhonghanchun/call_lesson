#include "tcpClient.hpp"
#include <memory>

void Usage(string proc)
{
    std::cout<<"\nUsage:\n\t"<<proc<<" ip prot"<<endl;
}

int main(int argc, char* argv[])
{
    if(argc!=3)
    {
        Usage(argv[0]);
        exit(1);
    }
    uint16_t port=atoi(argv[2]);
    string ip=argv[1];
    std::unique_ptr<TcpClinet> p(new TcpClinet(ip,port));
    p->Init();
    p->Start();
    return 0;
}
