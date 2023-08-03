#include "tcpServer.hpp"
#include "protol.hpp"
#include <memory>
using namespace std;
void Usage(string proc)
{
    cout<<"\nUsage:\n\t"<<proc<< " port"<<endl;
}


bool my_string(const Requse& req,Repose& rep)
{
    switch(req._op)
    {
        case '+':
            rep._result=req._x+req._y;
            break;
        case '-':
            rep._result=req._x-req._y;
            break;
        case '*':
            rep._result=req._x*req._y;
            break;
        case '/':
            {
                if(req._y==0)
                {
                    rep._exitcode=DIV_ZERO;
                }
                else 
                {
                    rep._result=req._x+req._y;
                }
            }
            break;
        case '%':
            {
                if(req._y==0)
                {
                    rep._exitcode=MOD_ZERO;
                }
                else 
                {
                    rep._result=req._x+req._y;
                }
            }
            break;
    }
    return true;
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
    p->Start(my_string);
    return 0;
}
