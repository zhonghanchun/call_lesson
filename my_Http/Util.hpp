#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>

//工具类
class Util
{
public:
    static int ReadLine(int sock,std::string& out)
    {
        char ch='o';
        while(ch!='\n')
        {
            ssize_t s=recv(sock,&ch,sizeof ch,0);
            if(s>0)
            {
                if(ch=='\r')
                {
                    recv(sock,&ch,sizeof ch,MSG_PEEK);
                    if(ch=='\n')
                    {
                        recv(sock,&ch,sizeof ch,0);
                    }
                    else 
                    {
                        ch='\n';
                    }
                }
                out.push_back(ch);
            }
            else if(s==0)
            {
                break;
            }
            else 
            {
                return -1;
            }
        }
        return out.size();
    }
    static bool CutString(const std::string& target,std::string& key,std::string& val,const std::string& op)
    {
        size_t pos=target.find(op);
        if(pos!=std::string::npos)
        {
            key=target.substr(0,pos);
            val=target.substr(pos+op.size());
            return true;
        }
        return false;
    }
};
