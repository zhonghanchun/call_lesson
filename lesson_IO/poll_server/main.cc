#include "pollServer.hpp"
#include <memory>
#include <vector>
using namespace std;
using namespace poll_ns;

void Usage(std::string proc){
    std::cout<<"Usage: \n\t"<<proc<<" port"<<std::endl;
}
string reader(const string& responce){
    return responce;
}

int main(int argc,char* argv[])
{
    if(argc!=2){
        Usage(argv[0]);
        exit(-1);
    }
    unique_ptr<pollServer> svr(new pollServer(reader,atoi(argv[1])));
    svr->Init();
    svr->Start();
    return 0;
}
