#include <iostream>
#include <string>
class Util
{
public:
    static std::string get_one_line(std::string buffer,const std::string& sep)
    {
        auto pos=buffer.find(sep);
        if(pos==std::string::npos)return "";
        std::string sub=buffer.substr(0,pos);
        buffer.erase(0,sub.size()+sep.size());
        return sub;
    }
};
