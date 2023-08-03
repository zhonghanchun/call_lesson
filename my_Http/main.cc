#include <string>
#include <memory>
#include "Log.hpp"
#include "HttpServer.hpp"
static void Usage(std::string proc)
{
    std::cout<<"Usage:\n\t"<<proc<<" port"<<std::endl;
}

int main(int argc,char* argv[])
{
    if(argc!=2)
    {
        Usage(argv[0]);
        exit(4);
    }
    std::shared_ptr<HttpServer> http_server(new HttpServer(atoi(argv[1])));
    http_server->InitServer();
    http_server->Loop();

    //LOG(INFO,"debug project!");
    //std::cout<<__FILE__<<__LINE__<<std::endl;
    return 0;
}
