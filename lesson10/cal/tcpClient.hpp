#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include "protol.hpp"
using namespace std;
class TcpClinet
{
public:
    TcpClinet(const string& serverip,const uint16_t serverport)
        :_serverip(serverip),_serverport(serverport)
    {}
    void Init()
    {
        sock=socket(AF_INET,SOCK_STREAM,0);
        if(sock<0)
        {
            cout<<"socket failed!"<<endl;
            exit(1);
        }
    }
    void Start()
    {
        struct sockaddr_in server;
        memset(&server,0,sizeof(server));
        server.sin_family=AF_INET;
        server.sin_port=htons(_serverport);
        server.sin_addr.s_addr=inet_addr(_serverip.c_str());
        if(connect(sock,(struct sockaddr*)&server,sizeof(server))<0)
        {
            cout<<"connect failed!"<<endl;
            exit(2);
        }
        char buffer[1024]={0};
        while(true)
        {
           cout<<"Enter x# "; 
           int x; cin>>x;
           cout<<"Enter y# ";
           int y;cin>>y;
           cout<<"Plase Enter operator# ";
           char op;cin>>op;

           Requse q(x,y,op);
           string out;
           q.serialize(&out);
           string send_string;
           send_string=enLength(out);
           send(sock,send_string.c_str(),send_string.size(),0);

            
           string package,text;
           ssize_t s=recv(sock,buffer,sizeof(buffer)-1,0);
           if(s>0)
           {
                buffer[s]=0;
                deLength(buffer,&text);
                Repose p;
                p.deserialize(text);

                cout<<"exitcode : "<<p._exitcode<<endl;
                cout<<"result: "<<p._result<<endl;
           }



          /* fgets(buffer,sizeof(buffer),stdin);
           write(sock,buffer,sizeof(buffer));

           buffer[0]=0;
           ssize_t s=read(sock,buffer,sizeof(buffer));
           if(s>0)
           {
               buffer[s]=0;
               cout<<buffer<<endl;
           }*/
        }
    }
    ~TcpClinet(){}

private:
    int sock;
    string _serverip;
    uint16_t _serverport;
};
