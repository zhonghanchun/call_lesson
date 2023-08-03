#include "TcpServer.hpp"
#include <memory>
using namespace tcpserver;
using namespace std;

void Usage(const string &proc)
{
    cout<<"Usage: \n\t"<<proc<<" port"<<endl;
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
void calculate(Connection* conn)
{
    std::string OnePackage;
    while(ParseOnePackage(conn->inbuffer,&OnePackage))
    {
        std::string reqStr;
        if(!deLength(OnePackage,&reqStr)) return;
        std::cout<<"去掉报头的正文: \n"<<reqStr<<std::endl;
        Requse req;
        if(!req.deserialize(reqStr))return;
        Repose resp;
        my_string(req,resp);
        std::string respStr;
        resp.serialize(&respStr);
        std::string send_string=enLength(respStr);
        conn->outbuffer+=send_string;
        std::cout<<"result: "<<send_string<<std::endl;
           
    }
    if(conn->_sender)
        conn->_sender(conn);
    if(!conn->outbuffer.empty())
        conn->_tsp->EnableEeadWrite(conn,true,true);
    else conn->_tsp->EnableEeadWrite(conn,true,false);
}

int main(int argc,char* argv[])
{
    if(argc!=2){
        Usage(argv[0]);
        exit(0);
    } 
    unique_ptr<TcpServer> tp(new TcpServer(calculate,stoi(argv[1])));
    tp->Init();
    tp->Dispatcher();
    return 0;
}
