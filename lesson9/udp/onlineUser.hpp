#pragma once 
#include <iostream>
#include <string>
#include <unordered_map>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

using namespace std;

class User
{
public:
    User(const string& ip,const uint16_t& port)
        :_ip(ip)
         ,_port(port)
    {}
    ~User(){}
    string get_ip()
    {
        return _ip;
    }
    uint16_t get_port()
    {
        return _port;
    }
private:
    string _ip;
    uint16_t _port;
};
class OnlineUser
{
public:
    OnlineUser(){}
    ~OnlineUser(){}
    void add_user(const string& ip,const uint16_t& port)
    {
        string id=ip+"-"+to_string(port);
        _users.insert(make_pair(id,User(ip,port)));
    }
    void deluser(const string& ip,const uint16_t& port)
    {
        string id=ip+"-"+to_string(port);
        _users.erase(id);
    }
    bool is_onlie(const string&ip,const uint16_t& port)
    {
        string id=ip+"-"+to_string(port);
        return _users.find(id)==_users.end()?false:true;
    }
    void broadcastMessage(int socket,const string& ip,const uint16_t& port,const string& message)
    {
        for(auto& user:_users)
        {
            struct sockaddr_in client;
            memset(&client,0,sizeof(client));
            client.sin_family=AF_INET;
            client.sin_port=htons(user.second.get_port());
            client.sin_addr.s_addr=inet_addr(user.second.get_ip().c_str());
            
            string s=ip+"-"+to_string(port)+"# ";
            s+=message;

            sendto(socket,s.c_str(),s.size(),0,(struct sockaddr*)&client,sizeof(client));
            
        }
    }
private:
    unordered_map<string,User> _users;
};
