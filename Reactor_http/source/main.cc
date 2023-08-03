#include "server.hpp"



int main()
{
    Buffer buf;
    for(int i=1;i<=100;i++)
    {
        std::string str="hello world! " +std::to_string(i)+'\n';
        buf.WriteStringAndPop(str);
    }
    //std::string tmp=buf.ReadStringAndPop(buf.ReadAbleSize());
    while(buf.ReadAbleSize()>0)
    {
        std::string str=buf.GetLineAndPop();
        DBG_LOG("hello!");
    }
    std::cout<<1<<std::endl;
    return 0;
}
