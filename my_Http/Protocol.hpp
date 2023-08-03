#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/sendfile.h>
#include "Util.hpp"
#include "Log.hpp"

#define OK 200
#define BAD_REQUEST 400
#define NOT_FOUND 404
#define SERVER_ERROR 500
#define WEB_ROOT "wwwroot"
#define HOME_PAGE "index.html"
#define HTTP_VERSION "HTTP/1.0"
#define LINE_END "\r\n"
#define PAGE_404 "404.html"

std::string Code2_desc(int code)
{
    switch(code)
    {
        case 200:return "OK";
        case 404:return "Not Found";
        default:break;
    }
    return "";
}
std::string Suffix2Desc(const std::string suffix)
{
    static std::unordered_map<std::string,std::string> suffix2desc={
        {".html","text/html"},
        {".css","text/css"},
        {".js","application/javascript"},
        {".xml","application/xml"}
    };
    auto iter=suffix2desc.find(suffix);
    if(iter==suffix2desc.end())
    {
        return "text/html";
    }
    return iter->second;
}
class HttpRequest
{
public:
    std::string request_line;
    std::vector<std::string> request_header;
    std::string blank;
    std::string request_body;

    //解析完毕之后的结果
    std::string method;
    std::string uri;
    std::string version;
    
    std::string path;
    std::string suffix;
    std::string query_string;

    std::unordered_map<std::string,std::string> header_kv;
    int content_length=0;
    bool cgi=false;
    int size=0;
};
class HttpResponse
{
public:
    std::string status_line;
    std::vector<std::string> response_header;
    std::string blank=LINE_END;
    std::string response_body;

    int status_code;
    int fd=-1;
    HttpResponse()
        :status_code(OK)
    {}
    ~HttpResponse(){}
};

//读取请求 分析请求 构建响应
//IO通信
class EndPoint
{
private:
    bool recv_http_request_line()
    {
        if(Util::ReadLine(sock,http_request.request_line)>0)
        {
            http_request.request_line.resize(http_request.request_line.size()-1);
            LOG(INFO,http_request.request_line);
        }
        else 
        {
            stop=true;
        }
        return stop;
    }
    bool recv_http_request_header()
    {
        std::string line;
        while(true)
        {
            //std::string line;
            line.clear();
            if(Util::ReadLine(sock,line)<=0)
            {
                stop=true;
                break;
            }
            if(line=="\n")
            {
                http_request.blank=line;
                break;
            }
            line.resize(line.size()-1);
            http_request.request_header.push_back(line);
            LOG(INFO,line);
        }
        return stop;
        // if(line=="\n")
        // {
        //     http_request.blank=line;
        // }
    }
    void pares_http_request_line()
    {
        std::stringstream ss(http_request.request_line);
        ss>>http_request.method>>http_request.uri>>http_request.version;
        LOG(INFO,http_request.method);
        LOG(INFO,http_request.uri);
        LOG(INFO,http_request.version);
        std::transform(http_request.method.begin(),http_request.method.end(),http_request.method.begin(),::toupper);//转大写
    }
    void pares_http_request_header()
    {
        for(auto& iter : http_request.request_header)
        {
            std::string key;
            std::string val;
            if(Util::CutString(iter,key,val,": "))
            {
                http_request.header_kv[key]=val;
                //std::cout<<"debug"<<key<<": "<<val<<std::endl;
            }
        }
    }
    bool isnedd_recv_http_request_body()
    {
        auto& method=http_request.method;
        if(method=="POST")
        {
            auto& header_kv=http_request.header_kv;
            auto pos=header_kv.find("Content-Length");
            if(pos!=header_kv.end())
            {
                http_request.content_length=atoi(pos->second.c_str());
                return true;
            }
        }
        return false;
        
    }
    bool recv_http_request_body()
    {
        if(isnedd_recv_http_request_body())
        {
            int content_length=http_request.content_length;
            auto& body=http_request.request_body;
            char ch=0;
            while(content_length)
            {
                ssize_t s=recv(sock,&ch,sizeof(ch),0);
                if(s>0)
                {
                    body.push_back(ch);
                    content_length--;
                }
                else{
                    stop=true;
                    break;
                }
            }
        }
        return stop;
    }
    int process_Non_cgi()
    {
        http_response.fd=open(http_request.path.c_str(),O_RDONLY);
        if(http_response.fd>=0)
        {
            /*http_response.status_line=HTTP_VERSION;
            http_response.status_line+=" ";
            http_response.status_line+=std::to_string(http_response.status_code);
            http_response.status_line+=" ";
            http_response.status_line+=Code2_desc(http_response.status_code);
            http_response.status_line+=LINE_END;

            std::string content_length_string="Content-Type: ";
            content_length_string+=Suffix2Desc(http_request.suffix);
            content_length_string+=LINE_END;
            http_response.response_header.push_back(content_length_string);
            
            content_length_string="Content-Length: ";
            content_length_string+=std::to_string(http_response.size);
            content_length_string+=LINE_END;
            http_response.response_header.push_back(content_length_string);
            */
            return OK;
        }
        return NOT_FOUND;
    }
    int process_cgi()
    {
        std::cout<<"process_cgi ...!"<<std::endl;
        int code=OK;
        auto& query_string=http_request.query_string;//GET
        auto& body_text=http_response.response_body;//POST
        auto& bin=http_request.path;
        auto& response_body=http_response.response_body;

        int content_length=http_request.content_length;

        std::string query_string_env;
        std::string method_env;
        std::string content_length_env;
        std::string body_env;
        int input[2];
        int output[2];
        if(pipe(input)<0)
        {
            LOG(ERROR,"pipe input error!");
            code=SERVER_ERROR;
            return code;
        }
        if(pipe(output)<0)
        {
            LOG(ERROR,"pipe output error!");
            code=SERVER_ERROR;
            return code;
        }

        pid_t pid=fork();
        if(pid==0)
        {
            //chile
            close(input[0]);
            close(output[1]);
            
            
            method_env="METHOD=";
            method_env+=http_request.method;
            putenv((char*)method_env.c_str());

            if(http_request.method=="GET")
            {
                query_string_env="QUERY_STRING=";
                query_string_env+=query_string;
                putenv((char*)query_string_env.c_str());
                LOG(WARNING,query_string_env);
            }
            else if(http_request.method=="POST")
            {
                content_length_env="CONTENT_LENGTH=";
                body_env="BODY=";
                body_env+=http_request.request_body;
                content_length_env+=std::to_string(content_length);
                putenv((char*)content_length_env.c_str());
                putenv((char*)body_env.c_str());
            }

            dup2(output[0],0);
            dup2(input[1],1);

            execl(bin.c_str(),bin.c_str(),nullptr);
            exit(1);
        }
        else if(pid<0)
        {
            //error
            LOG(ERROR,"fork error!");
            return 404;
        }
        else 
        {
            close(input[1]);
            close(output[0]);
            
            if(http_request.method=="POST")
            {
                const char* start=body_text.c_str();
                int size=0;
                int total=0;
                while(total<content_length&&(size=write(output[1],start+total,body_text.size()-total))>0)
                {
                        total+=size;
                }

            }
        
            char ch=0;
            while(read(input[0],&ch,1)>0)
            {
                response_body.push_back(ch);
            }
            
            int status=0;
            pid_t res=waitpid(pid,&status,0);
            if(res==pid)
            {
                if(WIFEXITED(status))
                {
                    if(WEXITSTATUS(status)==0)
                    {
                        code=OK;
                    }
                    else 
                    {
                        code=BAD_REQUEST;
                    }
                }
                else 
                {
                    code=SERVER_ERROR;
                }
            }


            close(input[0]);
            close(output[1]);
        }
        return code;
    }
    void Handler_Error(std::string page)
    {
        http_request.cgi=false;
        http_response.fd=open(page.c_str(),O_RDONLY);
        if(http_response.fd>0)
        {
            struct stat st;
            stat(page.c_str(),&st);
            http_request.size=st.st_size;
            std::string line="Content-Type: text/html";
            line+=LINE_END;
            http_response.response_header.push_back(line);
            line="Content-Length: ";
            line+=std::to_string(st.st_size);
            line+=LINE_END;
            http_response.response_header.push_back(line);
        }
    }
    void Build_OK_Response()
    {
        std::string line="Content-Type: ";
        line+=Suffix2Desc(http_request.suffix);
        line+=LINE_END;
        http_response.response_header.push_back(line);
        line="Conten-Length: ";
        if(http_request.cgi)
        {
            line+=std::to_string(http_response.response_body.size());//POST
        }
        else
        {
            line+=std::to_string(http_request.size);//GET
        }
        line+=LINE_END;
        http_response.response_header.push_back(line);
    }
    void build_http_response_helper()
    {
        auto& code=http_response.status_code;
        auto& status_line=http_response.status_line;
        status_line+=HTTP_VERSION;
        status_line+=" ";
        status_line+=std::to_string(code);
        status_line+=" ";
        status_line+=Code2_desc(code);
        status_line+=LINE_END;

        std::string path=WEB_ROOT;
        switch(code)
        {
            case OK:Build_OK_Response();
                break;
            case NOT_FOUND:
            case SERVER_ERROR:
            case BAD_REQUEST:
                path+="/";
                path+=PAGE_404;
                Handler_Error(path);
                break;
            default:
                break;
        }
    }
public:
    EndPoint(int _sock)
        :sock(_sock)
         ,stop(false)
    {}
    bool is_stop()
    {
        return stop;
    }
    void recv_http_request()
    {
        if((!recv_http_request_line()) && (!recv_http_request_header()))
        {
            pares_http_request_line();
            pares_http_request_header();
            recv_http_request_body();
        }
    }
   /* void parse_http_request()
    {
        this->pares_http_request_line();
        this->pares_http_request_header();
        this->recv_http_request_body();
    }*/
    void build_http_response()
    {
        std::string paths;
        auto& code=http_response.status_code;
        struct stat st;
        int size=0;
        size_t index=0;
        if(http_request.method!="GET"&&http_request.method!="POST")
        {
            LOG(WARNING,"method is not right!");
            code=BAD_REQUEST;
            goto END;
        }
        if(http_request.method=="GET")
        {
            size_t pos=http_request.uri.find('?');
            if(pos!=std::string::npos)
            {
                std::string left;
                std::string right;
                Util::CutString(http_request.uri,left,right,"?");
                http_request.path=left;
                http_request.query_string=right;
                http_request.cgi=true;
            }
            else{
                http_request.path=http_request.uri;
            }
        }
        else if(http_request.method=="POST") 
        {
            //POST
            http_request.cgi=true; 
            http_request.path=http_request.uri;
        }
        else{
            //Do Nothing
        }
        paths=http_request.path;
        http_request.path=WEB_ROOT;
        http_request.path+=paths;
        if(http_request.path[http_request.path.size()-1]=='/')
        {
            http_request.path+=HOME_PAGE;
        }
        
        if(stat(http_request.path.c_str(),&st)==0)
        {
            //说明资源存在
            if(S_ISDIR(st.st_mode))
            {
                //说明请求的资源是一个目录，需要处理
                http_request.path+="/";
                http_request.path+=HOME_PAGE;
                stat(http_request.path.c_str(),&st);
            }
            if((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH))
            {
                //可执行需要特殊处理
                http_request.cgi=true;
            }
            size=st.st_size;
            http_request.size=size;
        }
        else 
        {
            //说明资源不存在
            LOG(WARNING,http_request.path+" Not Found!");
            code=NOT_FOUND;
            goto END;

        }
        index=http_request.path.rfind(".");
        if(index!=std::string::npos)
        {
            http_request.suffix=http_request.path.substr(index);
        }
        else 
        {
            http_request.suffix=".html";
        }

        if(http_request.cgi)
        {
            code=process_cgi();
        }
        else 
        {
            code=process_Non_cgi();
        }
END:
    //   if(code!=OK)
    //   {
            build_http_response_helper();
    //   }
    }
    void send_http_response()
    {
        send(sock,http_response.status_line.c_str(),http_response.status_line.size(),0);
        for(auto& iter:http_response.response_header)
        {
            send(sock,iter.c_str(),iter.size(),0);
        }
        send(sock,http_response.blank.c_str(),http_response.blank.size(),0);
        if(http_request.cgi)
        {
            auto& response_body=http_response.response_body;
            size_t size=0;
            size_t total=0;
            const char* start=response_body.c_str();
            while(total<response_body.size()&&(size=send(sock,start+total,response_body.size()-total,0))>0)
            {
                total+=size;
            }
            //size=send(sock,start,response_body.size(),0);
            //std::cout<<"total: "<<total<<"   "<<"size: "<<size<<std::endl;
            //std::cout<<"response_body.size(): "<<response_body.size()<<std::endl;
            //send(sock,response_body.c_str(),response_body.size(),0);
        }
        else 
        {
            sendfile(sock,http_response.fd,nullptr,http_request.size);
        }
            close(http_response.fd);
    }   
    ~EndPoint()
    {
        close(sock);
    }
    void Printf()
    {
        // std::cout<<http_request.request_line<<std::endl;
        // std::cout<<http_response.status_line<<std::endl;
        // std::cout<<http_request.path<<std::endl;
        // std::cout<<http_request.uri<<std::endl;
        std::cout<<"..........................................."<<std::endl;
        LOG(INFO,http_request.request_line);
        LOG(INFO,http_response.status_line);
        LOG(INFO,http_request.path);
        LOG(INFO,http_request.uri);
        std::cout<<"..........................................."<<std::endl;
    }
private:
    int sock;
    HttpRequest http_request;
    HttpResponse http_response;
    bool stop;
};


class CallBack
{
    public:
        CallBack(){}
        void operator()(int sock)
        {
            HandlerRequest(sock);
        }
        void HandlerRequest(int sock)
        {
            LOG(INFO,"Hander Request begin!");

            //std::string out;
            //Util::ReadLine(sock,out);

            //std::cout<<out<<std::endl;
            //std::cout<<"get a new link..."<<std::endl;
            //close(sock);
#ifdef DEBUG
            char buffer[4096];
            recv(sock,buffer,sizeof buffer,0);
            std::cout<<"--------------begin----------------"<<std::endl;
            std::cout<<buffer<<std::endl;
            std::cout<<"---------------end-----------------"<<std::endl;
#else
            EndPoint* ep=new EndPoint(sock);
            //ep->Printf();
            ep->recv_http_request();
            //ep->parse_http_request();
            if(!ep->is_stop())
            {
                LOG(NFO,"Recv No Error,Begin Build And Send!");
                ep->build_http_response();
                ep->send_http_response();
                ep->Printf();
            }
            else 
            {
                LOG(WARNING,"Recv Error ,Stop Build And Send!");
            }
            delete ep;
#endif
            LOG(INFO,"Hander Request end!");
        }
        ~CallBack(){}
};
