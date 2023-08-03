#pragma once 
#include <iostream>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <jsoncpp/json/json.h>

#define SEP " "
#define SEP_LEN strlen(SEP)
#define LINE_SEP "\r\n"
#define LINE_SEP_LEN strlen(LINE_SEP)

enum
{
    OK=0,
    DIV_ZERO,
    MOD_ZERO,
    OP_ERROR
};

std::string enLength(const std::string& text)
{
    std::string send_srting=std::to_string(text.size());
    send_srting+=LINE_SEP;
    send_srting+=text;
    send_srting+=LINE_SEP;

    return send_srting;
}
bool deLength(const std::string& package,std::string *text)
{
    auto pos=package.find(LINE_SEP);
    if(pos==std::string::npos)return false;
    std::string text_len_string =package.substr(0,pos);
    int text_len=std::stoi(text_len_string);
    *text=package.substr(pos+LINE_SEP_LEN,text_len);

    return true;
}


class Requse
{
public:
    Requse()
        :_x(0)
         ,_y(0)
         ,_op(0)
    {}
    Requse(const int x,const int y,const char op)
        :_x(x)
         ,_y(y)
         ,_op(op)
    {}
    
    bool serialize(std::string* out)
    {
#ifdef MYSELF
        *out="";
        std::string x_string=std::to_string(_x);
        std::string y_string=std::to_string(_y);
        *out+=x_string;
        *out+=SEP;
        *out+=_op;
        *out+=SEP;
        *out+=y_string;
#else 
        Json::Value root;
        root["first"]=_x;
        root["second"]=_y;
        root["operator"]=_op;

        Json::FastWriter writer;
        *out = writer.write(root);
#endif

        return true;
    }
    bool deserialize(const std::string &in)
    {
#ifdef MYSELF
        auto left=in.find(SEP);
        auto right=in.rfind(SEP);
        if(left==std::string::npos||right==std::string::npos)return false;
        if(left==right)return false;
        if(right-(left+SEP_LEN)!=1)return false;
        std::string x_string=in.substr(0,left);
        std::string y_string=in.substr(right+SEP_LEN);
        if(x_string.empty()||y_string.empty())return false;

        _x=std::stoi(x_string);
        _y=std::stoi(y_string);
        _op=in[left+SEP_LEN];
#else
        Json::Value root;
        Json::Reader reader;
        reader.parse(in,root);

        _x=root["first"].asInt();
        _y=root["second"].asInt();
        _op=root["operator"].asInt();

#endif

        return true;
    }
    ~Requse(){}

public:
    int _x;
    int _y;
    char _op;
};
class Repose
{
public:
    Repose()
        :_exitcode(0)
         ,_result(0)
    {}
    Repose(const int exitcode,const int result)
        :_exitcode(exitcode)
         ,_result(result)
    {}
    bool serialize(std::string* out)
    {
#ifdef MYSELF
        *out="";
        std::string exitcode_string=std::to_string(_exitcode);
        std::string result_string=std::to_string(_result);
        *out+=exitcode_string;
        *out+=SEP;
        *out+=result_string;
#else
        Json::Value root;
        root["exitcode"]=_exitcode;
        root["result"]=_result;

        Json::FastWriter writer;
        *out = writer.write(root); 
#endif

        return true;
    }

    bool deserialize(const std::string &in)
    {
#ifdef MYSELF
        auto min=in.find(SEP);
        if(min==std::string::npos)return false;
        std::string exitcode_string=in.substr(0,min);
        std::string result_string=in.substr(min+SEP_LEN);

        _exitcode=std::stoi(exitcode_string);
        _result=std::stoi(result_string);
#else
       Json::Value root;
       Json::Reader reader;
       reader.parse(in,root);

       _exitcode=root["exitcode"].asInt();
       _result=root["result"].asInt(); 
#endif

        return true;
    }
    ~Repose(){}
public:
    int _exitcode;
    int _result;
};


bool RecvPackage(int sock,std::string &inbuffer,std::string *text)
{
    char buffer[1024]={0};
    while(true)
    {
        ssize_t n=recv(sock,buffer,sizeof(buffer)-1,0);
        if(n>0)
        {
            buffer[n]=0;
            inbuffer+=buffer;

            auto pos=inbuffer.find(LINE_SEP);
            if(pos==std::string::npos)continue;
            std::string text_len_string=inbuffer.substr(0,pos);
            int text_len=std::stoi(text_len_string);
            int total=text_len_string.size()+2*LINE_SEP_LEN+text_len;
            if(inbuffer.size() < total)
                continue;
            *text=inbuffer.substr(0,total);
            inbuffer.erase(0,total);
            break;
        }
        else return false;
    }
    return true;
}
