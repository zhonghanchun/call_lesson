#include "tcpServer.hpp"
#include "deamon.hpp"
#include <memory>
using namespace std;
void Usage(string proc)
{
    cout<<"\nUsage:\n\t"<<proc<< " port"<<endl;
}

int main(int args,char* argv[])
{
    if(args!=2)
    {
        Usage(argv[0]);
    }
    uint16_t port=atoi(argv[1]);
    unique_ptr<TcpServer> p(new TcpServer(port));
    p->Init();
    daemonSelf(); 
    p->Start();
    return 0;
}
