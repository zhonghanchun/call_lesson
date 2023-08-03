namespace websocketpp {
    typedef lib::weak_ptr<void> connection_hdl;

    template <typename config>
    class endpoint : public config::socket_type {
        typedef lib::shared_ptr<lib::asio::steady_timer> timer_ptr;
        typedef typename connection_type::ptr connection_ptr;
        typedef typename connection_type::message_ptr message_ptr;

        typedef lib::function<void(connection_hdl)> open_handler;
        typedef lib::function<void(connection_hdl)> close_handler;
        typedef lib::function<void(connection_hdl)> http_handler;
        typedef lib::function<void(connection_hdl,message_ptr)>
                message_handler;
        /* websocketpp::log::alevel::none 禁⽌打印所有⽇志*/
        void set_access_channels(log::level channels);/*设置⽇志打印等级*/
        void clear_access_channels(log::level channels);/*清除指定等级的⽇志*/ /*设置指定事件的回调函数*/
        void set_open_handler(open_handler h);/*websocket握⼿成功回调处理函数*/ void set_close_handler(close_handler h);
        /*websocket连接关闭回调处理函数*/ 
        void set_message_handler(message_handler h);
        /*websocket消息回调处理函数*/ 
        void set_http_handler(http_handler h);/*http请求回调处理函数*/
        /*发送数据接⼝*/
        void send(connection_hdl hdl, std::string& payload,frame::opcode::value op);
        void send(connection_hdl hdl, void* payload, size_t len,frame::opcode::value op);
        /*关闭连接接⼝*/
        void close(connection_hdl hdl, close::status::value code, std::string& reason);
        /*获取connection_hdl 对应连接的connection_ptr*/
        connection_ptr get_con_from_hdl(connection_hdl hdl);
        /*websocketpp基于asio框架实现，init_asio⽤于初始化asio框架中的io_service调度器*/
        void init_asio();
        /*设置是否启⽤地址重⽤*/
        void set_reuse_addr(bool value);
        /*设置endpoint的绑定监听端⼝*/
        void listen(uint16_t port);
        /*对io_service对象的run接⼝封装，⽤于启动服务器*/
        std::size_t run();
        /*websocketpp提供的定时器，以毫秒为单位*/
        timer_ptr set_timer(long duration, timer_handler callback);
    };
    template <typename config>
    class server : public endpoint<connection<config>,config> {
    /*初始化并启动服务端监听连接的accept事件处理*/
    void start_accept();
};

template <typename config>
class connection : public config::transport_type::transport_con_type, public config::connection_base
{
    /*发送数据接⼝*/
    error_code send(std::string&payload, frame::opcode::value
    op=frame::opcode::text);
    /*获取http请求头部*/
    std::string const & get_request_header(std::string const & key)
    /*获取请求正⽂*/
    std::string const & get_request_body();
    /*设置响应状态码*/
    void set_status(http::status_code::value code);
    /*设置http响应正⽂*/
    void set_body(std::string const & value);
    /*添加http响应头部字段*/
    void append_header(std::string const & key, std::string const & val);
    /*获取http请求对象*/
    request_type const & get_request();
    /*获取connection_ptr 对应的 connection_hdl */
    connection_hdl get_handle();
};

namespace http {
    namespace parser {
        class parser {
            std::string const & get_header(std::string const & key)
        }
        class request : public parser {
            /*获取请求⽅法*/
            std::string const & get_method()
            /*获取请求uri接⼝*/
            std::string const & get_uri()
        };
    }};

namespace message_buffer {
    /*获取websocket请求中的payload数据类型*/
    frame::opcode::value get_opcode();
    /*获取websocket中payload数据*/
    std::string const & get_payload();
};

namespace log {
    struct alevel {
        static level const none = 0x0;
        static level const connect = 0x1;
        static level const disconnect = 0x2;
        static level const control = 0x4;
        static level const frame_header = 0x8;
        static level const frame_payload = 0x10;
        static level const message_header = 0x20;
        static level const message_payload = 0x40;
        static level const endpoint = 0x80;
        static level const debug_handshake = 0x100;
        static level const debug_close = 0x200;
        static level const devel = 0x400;
        static level const app = 0x800;
        static level const http = 0x1000;
        static level const fail = 0x2000;
        static level const access_core = 0x00003003;
        static level const all = 0xffffffff;
    };
}

namespace http {
    namespace status_code {
        enum value {
            uninitialized = 0,
            continue_code = 100,
            switching_protocols = 101,
            ok = 200,
            created = 201,
            accepted = 202,
            non_authoritative_information = 203,
            no_content = 204,
            reset_content = 205,
            partial_content = 206,
            multiple_choices = 300,
            moved_permanently = 301,
            found = 302,
            see_other = 303,
            not_modified = 304,
            use_proxy = 305,
            temporary_redirect = 307,
            bad_request = 400,
            unauthorized = 401,
            payment_required = 402,
            forbidden = 403,
            not_found = 404,
            method_not_allowed = 405,
            not_acceptable = 406,
            proxy_authentication_required = 407,
            request_timeout = 408,
            conflict = 409,
            gone = 410,
            length_required = 411,
            precondition_failed = 412,
            request_entity_too_large = 413,
            request_uri_too_long = 414,
            unsupported_media_type = 415,
            request_range_not_satisfiable = 416,
            expectation_failed = 417,
            im_a_teapot = 418,
            upgrade_required = 426,
            precondition_required = 428,
            too_many_requests = 429,
            request_header_fields_too_large = 431,
            internal_server_error = 500,
            not_implemented = 501,
            bad_gateway = 502,
            service_unavailable = 503,
            gateway_timeout = 504,
            http_version_not_supported = 505,
            not_extended = 510,
            network_authentication_required = 511
        };}}
namespace frame {
    namespace opcode {
        enum value {
            continuation = 0x0,
            text = 0x1,
            binary = 0x2,
            rsv3 = 0x3,
            rsv4 = 0x4,
            rsv5 = 0x5,
            rsv6 = 0x6,
            rsv7 = 0x7,
            close = 0x8,
            ping = 0x9,
            pong = 0xA,
            control_rsvb = 0xB,
            control_rsvc = 0xC,
            control_rsvd = 0xD,
            control_rsve = 0xE,
            control_rsvf = 0xF,
        };}}
}
