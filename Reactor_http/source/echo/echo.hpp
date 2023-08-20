#include "../server.hpp"

class EchoServer
{
    private:
        TcpServer _server;
    private:
        void OnConnected(const PtrConnection& conn){
            DBG_LOG("new connection: %p",conn.get());
        }
        void OnClose(const PtrConnection& conn){
            DBG_LOG("close connection: %p",conn.get());
        }
        void OnMessage(const PtrConnection& conn,Buffer* buf){
            DBG_LOG("%s",buf->ReadPosition());
            buf->MoveReadOffset(buf->ReadAbleSize());
            std::string str="hello world";
            conn->Send(&str[0],str.size());
            conn->Shutdown();
        }
    public:
        EchoServer(int port)
            :_server(port)
        {
            _server.SetClosedCallback(std::bind(&EchoServer::OnClose,this,std::placeholders::_1));
            _server.SetMessageCallback(std::bind(&EchoServer::OnMessage,this,std::placeholders::_1,std::placeholders::_2));
            _server.SetConnectedCallback(std::bind(&EchoServer::OnConnected,this,std::placeholders::_1));
        }
        void Start(){
            _server.Start();
        }
};
