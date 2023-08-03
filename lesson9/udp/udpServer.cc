#include "udpServer.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <fstream>
#include "onlineUser.hpp"
using namespace std;
unordered_map<string,string> dict;
const string dict_text="./dict.txt";
void Usage(string port)
{
    cout<<"Usage \n\t"<< port<<" port"<<endl;
}

static bool curString(const string& cur,string& s1,string& s2)
{
    size_t pos=cur.find(":");
    if(pos!=string::npos)
    {
        s1=cur.substr(0,pos);
        s2=cur.substr(pos+1);
        return true;
    }
    return false;
}
static void Init_dict()
{
    ifstream in(dict_text,std::ios::binary);
    if(!in.is_open())
    {
        cout<<"dict_text fail!"<<endl;
        exit(4);
    }
    string line;
    string key,value;
    while(getline(in,line))
    {
        //cout<<line<<endl;
         if(curString(line,key,value))
         {
             dict.insert(make_pair(key,value));
        }
    }
}

void handler(int sock,string ip,uint16_t port,string res)
{
    //cout<<ip<<" "<<port<<" "<<res<<endl;
    cout<<"handler:"<<ip<<":"<<port<<"# "<<res<<endl;
    auto pos=dict.find(res);
    string str;
    if(pos!=dict.end())
    {
        str=pos->second;
    }
    else 
    {
        str="unknow";
    }
    struct sockaddr_in client;
    memset(&client,0,sizeof(client));
    client.sin_family=AF_INET;
    client.sin_port=htons(port);
    client.sin_addr.s_addr=inet_addr(ip.c_str());
    socklen_t len=sizeof(client);
    sendto(sock,str.c_str(),str.size(),0,(struct sockaddr*)&client,len);
}

OnlineUser online;
void routeMessage(int sock,string clientip,uint16_t clientport,string message)
{
    if(message=="online")
    {
        online.add_user(clientip,clientport);
    }
    if(message=="offine")
    {
        online.deluser(clientip,clientport);
    }
    if(online.is_onlie(clientip,clientport))
    {
        online.broadcastMessage(sock,clientip,clientport,message);
    }
    else 
    {
        struct sockaddr_in client;
        memset(&client,0,sizeof(client));
        client.sin_family=AF_INET;
        client.sin_port=htons(clientport);
        client.sin_addr.s_addr=inet_addr(clientip.c_str());
        string s="您还没上线，请先上线运行: online";
        sendto(sock,s.c_str(),s.size(),0,(struct sockaddr*)&client,sizeof(client));
    }
}

int main(int argc,char* argv[])
{
    if(argc!=2)
    {
        Usage(argv[0]);
        exit(1);
    }
    uint16_t port=atoi(argv[1]);
    Init_dict();
    std::unique_ptr<udpServer> p(new udpServer(routeMessage,port));
    //std::unique_ptr<udpServer> p(new udpServer(handler,port));
    p->Init_Server();
    p->Start();
    return 0;
}

