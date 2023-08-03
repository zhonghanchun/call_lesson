#include "http_server.hpp"
#include <memory>

void Usage(std::string port)
{
    cout<<"Usage \n\t"<<port<<" port"<<std::endl;
}

void cmd(Reques& q,Reponse& p)
{
    std::cout<<"method: "<<q.method<<std::endl;
    std::cout<<"url: "<<q.url<<std::endl;
    std::cout<<"httpversion: "<<q.httpversion<<std::endl;
    std::cout<<"path: "<<q.path<<std::endl;

    std::cout<<"------------------------------start-----------------------------"<<std::endl;
    std::cout<<q.req<<std::endl;
    std::cout<<"-------------------------------end------------------------------"<<std::endl;

    std::string respline = "HTTP/1.1 200 OK\r\n";
    std::string respheader = "Content-Type: text/html\r\n";
    std::string respblank = "\r\n";
    std::string body = "<html lang=\"en\"><head><meta charset=\"UTF-8\"><title>for test</title><h1>hello world</h1></head><body><p>北京交通广播《一路畅通》“交通大家谈”节目，特邀北京市交通委员会地面公交运营管理处处长赵震、北京市公安局公安交通管理局秩序处副处长 林志勇、北京交通发展研究院交通规划所所长 刘雪杰为您解答公交车专用道6月1日起社会车辆进出公交车道须注意哪些？</p></body></html>";

    p.rep+=respline;
    p.rep+=respheader;
    p.rep+=respblank;
    p.rep+=body;

}

int main(int argc,char* argv[])
{
    if(argc!=2)
    {
        Usage(argv[0]);
        exit(0);
    }
    int port=atoi(argv[1]);
    std::unique_ptr<Http_Server> p(new Http_Server(port));
    p->Init();
    p->start(cmd);
    return 0;
}
