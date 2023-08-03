#include "epollServer.hpp"
#include <memory>

using namespace epoll_ns;
using namespace std;

void Usage(std::string proc){
    std::cout<<"Usage: \n\t"<<proc<<" port"<<std::endl;
}

string Reader(const string& reponse){
    return reponse;
}

int main(int argc,char* argv[])
{
    if(argc!=2){
        Usage(argv[0]);
        exit(0);
    }
    unique_ptr<epollServer> esvr(new epollServer(Reader,stoi(argv[1])));
    esvr->Init();
    esvr->Start();
    return 0;
}
