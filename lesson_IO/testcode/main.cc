#include "util.hpp"
#include <functional>
#include <vector>

#define INIT(v)do{\
    v.push_back(a);\
    v.push_back(b);\
    v.push_back(c);\
}while(0)
#define EXECL_OUT(v)do{\
    for(const auto& tb: v)tb();\
}while(0)

using fun_t = std::function<void()>;
int main()
{ 
    std::vector<fun_t> v;
    INIT(v);
    setNonBlock(0);
    char buffer[1024]={0};
    while(true){
        printf(">>> ");
        fflush(stdout);
        ssize_t s=read(0,buffer,sizeof(buffer));
        if(s>0){
            buffer[s-1]=0;
            std::cout<<"echo# "<<buffer<<std::endl;
        }else if(s==0){
            std::cout<<"end"<<std::endl;
        }else{

        }
        EXECL_OUT(v);
        sleep(1);
    }
    return 0;
}
