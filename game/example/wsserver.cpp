#include <iostream>
#include <string>
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>

typedef websocketpp::server<websocketpp::config::asio> wsserver_t;

void http_callback(wsserver_t* wsserver,websocketpp::connection_hdl hd1)
{
    wsserver_t::connection_ptr conn=wsserver->get_con_from_hdl(hd1);
    std::cout<<conn->get_request_body()<<std::endl;
    websocketpp::http::parser::request req=conn->get_request();
    std::cout<<"method : "<<req.get_method()<<std::endl;
    std::cout<<"uri : "<<req.get_uri()<<std::endl;

    std::string body="<html><body><h1>Hello World!</h1></body></html>";
    conn->set_body(body);
    conn->append_header("Content-Type","text/html");
    conn->set_status(websocketpp::http::status_code::ok);
}
void wsopen_callback(wsserver_t* wsserver,websocketpp::connection_hdl hd1)
{
    std::cout<<"websocketpp 握手成功!"<<std::endl;
}
void wsclose_callback(wsserver_t* wsserver,websocketpp::connection_hdl hd1)
{
    std::cout<<"websocketpp 链接断开!"<<std::endl;
}
void wsmsg_callback(wsserver_t* wsserver,websocketpp::connection_hdl hd1,wsserver_t::message_ptr msg)
{
    wsserver_t::connection_ptr conn=wsserver->get_con_from_hdl(hd1);
    std::cout<<"msg: "<<msg->get_payload()<<std::endl;
    std::string rsp="client say: " + msg->get_payload();
    conn->send(rsp,websocketpp::frame::opcode::text);
}

int main()
{
    //实例化server对象
    wsserver_t wsserver;
    //设置日期等级
    wsserver.set_access_channels(websocketpp::log::alevel::none);
    //初始化asio调度器
    wsserver.init_asio();
    //设置回调函数
    wsserver.set_http_handler(std::bind(http_callback,&wsserver,std::placeholders::_1));
    wsserver.set_open_handler(std::bind(wsopen_callback,&wsserver,std::placeholders::_1));
    wsserver.set_close_handler(std::bind(wsclose_callback,&wsserver,std::placeholders::_1));
    wsserver.set_message_handler(std::bind(wsmsg_callback,&wsserver,std::placeholders::_1,std::placeholders::_2));
    //设置监听端口函数
    wsserver.listen(8085);
    //开始获取新链接
    wsserver.start_accept();
    //启动服务器
    wsserver.run();
    return 0;
}
