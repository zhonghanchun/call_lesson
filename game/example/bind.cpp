#include <iostream>
#include <string>
#include <functional>

void print(const char* str,int num)
{
    std::cout<<str<<" "<<num<<std::endl;
}

int main()
{
    auto func=std::bind(print,"你好!",std::placeholders::_1);
    func(23);
   // print("你好!",10);
    return 0;
}
