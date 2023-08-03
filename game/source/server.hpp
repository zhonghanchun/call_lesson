#include "util.hpp"
#include "db.hpp"
#include "matcher.hpp"
#include "online.hpp"
#include "session.hpp"

#define WWWROOT "./wwwroot/"

class gobang_server{
    private:
        wsserver_t _wssrv; 
        user_table _ut;
        online_game _og;
        room_manager _rm;
        session_manager _sm;
        matcher _mm;
        std::string _www_root;
    private:
        void file_handler(wsserver_t::connection_ptr &conn){
            //静态资源请求的处理
            //1.获取请求uri-资源路径，了解客户端请求的页面文件名称
            websocketpp::http::parser::request req=conn->get_request();
            std::string uri=req.get_uri();
            //2.组合出文件的实际路径 相对根目录 + uri
            std::string realpath=_www_root+uri;
            //3.如果请求的是个目录，增加一个后缀 login.html
            if(realpath.back()=='/'){
                realpath+="login.html";
            }
            std::string body;
            //4.读取文件内容
            bool ret=file_util::read(realpath,body);
                //1.文件不存在，读取内容失败返回404
            if(ret==false){
                body+="<html>";
                body+="<head>";
                body+="<meta charset='utf-8'/>";
                body+="</head>";
                body+="<body>";
                body+="<h1> Not Found </h1>";
                body+="</body>";
                conn->set_status(websocketpp::http::status_code::not_found);
                conn->set_body(body);
                return;
            }
            //5.设置响应正文
            conn->set_status(websocketpp::http::status_code::ok);
            conn->set_body(body);
        }
        void http_resp(wsserver_t::connection_ptr &conn,bool result,websocketpp::http::status_code::value code,const std::string &reason){
                Json::Value resp_json;
                resp_json["result"]=result;
                resp_json["reason"]=reason;
                std::string resp_body;
                json_util::serialize(resp_json,resp_body);
                conn->set_status(code);
                conn->append_header("Content-Type","application/json");
                conn->set_body(resp_body);
                return;
        }
        void reg(wsserver_t::connection_ptr &conn){
            //用户注册功能请求处理
            websocketpp::http::parser::request req=conn->get_request();
            //1.获取到请求正文
            std::string req_body=conn->get_request_body();
            //2.对正文进行json反序列化
            Json::Value login_info;
            Json::Value resp_json;
            bool ret=json_util::unserialize(req_body,login_info);
            if(ret==false){
                return http_resp(conn,false,websocketpp::http::status_code::bad_request,"请求正文错误");
            }
            //3.进行数据库的用户新增操作 成功则返回200/失败则返回400 
            if(login_info["username"].isNull()||login_info["password"].isNull()){
                return http_resp(conn,false,websocketpp::http::status_code::bad_request,"请输入用户名和密码");
            }
            bool flag=_ut.insert(login_info);
            if(flag==false){
                return http_resp(conn,false,websocketpp::http::status_code::bad_request,"用户名已被占用");
            }else{
                return http_resp(conn,true,websocketpp::http::status_code::ok,"注册成功");
            }
        }
        void login(wsserver_t::connection_ptr &conn){
            //用户登陆功能请求处理
            //1.获取请求正文进行json反序列化，得到用户名和密码
            std::string req_body=conn->get_request_body();
            Json::Value login_info;
            bool ret=json_util::unserialize(req_body,login_info);
            if(ret==false){
                DLOG("请求正文格式错误");
                return http_resp(conn,false,websocketpp::http::status_code::bad_request,"请求正文格式错误");
            }
            if(login_info["username"].isNull()||login_info["password"].isNull()){
                DLOG("用户名和密码错误");
                return http_resp(conn,false,websocketpp::http::status_code::bad_request,"用户名和密码错误");
            }
            bool flag=_ut.login(login_info); 
            if(flag==false){
                DLOG("用户名和密码错误");
                return http_resp(conn,false,websocketpp::http::status_code::bad_request,"用户名和密码错误");
            }
            //验证成功,给客户端创建session
            uint64_t uid=login_info["id"].asUInt64();
            session_ptr ssp=_sm.create_session(uid,LOGIN);
            if(ssp.get()==nullptr){
                DLOG("创建会话失败");
                return http_resp(conn,false,websocketpp::http::status_code::internal_server_error,"创建会话失败");
            }
            _sm.set_session_expire_time(ssp->ssid(),SESSION_TIMEOUT);
            //设置响应头部，Set-Cookie，将sessionid通过cookie返回
            std::string cookie_ssid="SSID="+std::to_string(ssp->ssid());
            conn->append_header("Set-Cookie",cookie_ssid);
            return http_resp(conn,true,websocketpp::http::status_code::ok,"登陆成功");
        }
        bool get_cookie_val(const std::string &cookie_str,const std::string &key,std::string &val){
            std::string sep="; ";
            std::vector<std::string> cookie_arr;
            string_util::split(cookie_str,sep,cookie_arr);
            for(auto str : cookie_arr){
              std::vector<std::string> tmp_arr;
              string_util::split(str,"=",tmp_arr);
              if(tmp_arr.size()!=2){continue;}
              if(tmp_arr[0]==key){
                val=tmp_arr[1];
                return true;
              }
            }
            return false;
        }
        void info(wsserver_t::connection_ptr &conn){
            //用户信息获取功能请求处理
            //获取请求信息中的Cookie，从Cookie中获取ssid
            std::string cookie_str=conn->get_request_header("Cookie");
            //如果没有cookie，返回错误：没有cookie信息，让客户端重新登陆
            if(cookie_str.empty()){
                DLOG("找不到cookie，请重新登陆");
                return http_resp(conn,true,websocketpp::http::status_code::bad_request,"找不到cookie，请重新登陆");
            }
            std::string ssid_str;
            bool ret=get_cookie_val(cookie_str,"SSID",ssid_str);
            if(ret==false){
              //cookie中没有ssid，返回错误，没有ssid信息，让客户端重新登陆
              DLOG("找不到ssid，请重新登陆");
                return http_resp(conn,true,websocketpp::http::status_code::bad_request,"找不到ssid，请重新登陆");
            }
            session_ptr ssp=_sm.get_session_by_ssid(std::stol(ssid_str));
            if(ssp.get()==nullptr){
              //没有找到session，则认为登陆过期，需要重新登陆
                return http_resp(conn,true,websocketpp::http::status_code::bad_request,"找不到session/登陆过期，请重新登陆");
            }
            uint64_t uid=ssp->get_user();
            Json::Value user_info;
            bool flag=_ut.select_by_id(uid,user_info);
            if(flag==false){
                DLOG("找不到用户信息");
                return http_resp(conn,true,websocketpp::http::status_code::bad_request,"找不到用户信息");
            }
            std::string body;
            json_util::serialize(user_info,body);
            conn->set_body(body);
            conn->append_header("Content-Type","application/json");
            conn->set_status(websocketpp::http::status_code::ok);
            DLOG("ssid: %d,set SESSION_TIMEOUT",(int)ssp->ssid());
            _sm.set_session_expire_time(ssp->ssid(),SESSION_TIMEOUT);
        }
        void http_callback(websocketpp::connection_hdl hd1){
            wsserver_t::connection_ptr conn=_wssrv.get_con_from_hdl(hd1);
            websocketpp::http::parser::request rep=conn->get_request();
            std::string method=rep.get_method();
            std::string uri=rep.get_uri();
            if(method=="POST"&&uri=="/reg"){
                return reg(conn);
            }else if(method=="POST"&&uri=="/login"){
                return login(conn);
            }else if(method=="GET"&&uri=="/info"){
                return info(conn);
            }else {
                return file_handler(conn);
            }
        }
        void ws_resp(wsserver_t::connection_ptr conn,Json::Value &resp){
            std::string body;
            json_util::serialize(resp,body);
            conn->send(body);
        }
        session_ptr get_session_by_cookie(wsserver_t::connection_ptr conn){
            Json::Value err_resp;
            //1 登陆验证--判断当前客户端是否已经登陆成功
            std::string cookie_str=conn->get_request_header("Cookie");
            if(cookie_str.empty()){
                //return http_resp(conn,true,websocketpp::http::status_code::bad_request,"找不到cookie，请重新登陆");
                err_resp["optype"]="hall_ready";
                err_resp["reason"]="找不到cookie，请重新登陆";
                err_resp["result"]=false;
                ws_resp(conn,err_resp);
            }
            std::string ssid_str;
            bool ret=get_cookie_val(cookie_str,"SSID",ssid_str);
            if(ret==false){
                //cookie中没有ssid，返回错误，没有ssid信息，让客户端重新登陆
                DLOG("找不到ssid，请重新登陆");
                //return http_resp(conn,true,websocketpp::http::status_code::bad_request,"找不到ssid，请重新登陆");
                err_resp["optype"]="hall_ready";
                err_resp["reason"]="找不到ssid，请重新登陆";
                err_resp["result"]=false;
                ws_resp(conn,err_resp);
                return session_ptr();
            }
            session_ptr ssp=_sm.get_session_by_ssid(std::stol(ssid_str));
            if(ssp.get()==nullptr){
                //没有找到session，则认为登陆过期，需要重新登陆
                //return http_resp(conn,true,websocketpp::http::status_code::bad_request,"找不到session/登陆过期，请重新登陆");
                err_resp["optype"]="hall_ready";
                err_resp["reason"]="找不到session，请重新登陆";
                err_resp["result"]=false;
                ws_resp(conn,err_resp);
                return session_ptr();
            }
            return ssp;
        }
        void wsopen_game_hall(wsserver_t::connection_ptr conn){
            //游戏大厅长连接建立成功
            Json::Value err_resp;
            //1 登陆验证--判断当前客户端是否已经登陆成功
            session_ptr ssp=get_session_by_cookie(conn);
            if(ssp.get()==nullptr){
                return ;
            }
            //2 判断当前用户端是否重复登陆
            if(_og.is_in_game_hall(ssp->get_user())||_og.is_in_game_room(ssp->get_user())){
                err_resp["optype"]="hall_ready";
                err_resp["reason"]="用户重复登陆";
                err_resp["result"]=false;
                return ws_resp(conn,err_resp);
            }
            //3 将当前客户端以及连接加入到游戏大厅
            _og.join_game_hall(ssp->get_user(),conn);
            //4 给客户端响应游戏大厅连接建立成功
            Json::Value json_resp;
            json_resp["optype"]="hall_ready";
            json_resp["result"]=true;
            ws_resp(conn,json_resp);
            //5 记得将session设置永久存在
            DLOG("ssid: %d,set SESSION_FOREVER",(int)ssp->ssid());
            _sm.set_session_expire_time(ssp->ssid(),SESSION_FOREVER);
        }
        void wsopen_game_room(wsserver_t::connection_ptr conn){
            //获取当前客户端的session
            session_ptr ssp=get_session_by_cookie(conn); 
            if(ssp.get()==nullptr){
              return;
            }
            //当前用户是否已经在游戏房间或者游戏大厅中
            Json::Value resp_json;
            if(_og.is_in_game_hall(ssp->get_user())||_og.is_in_game_room(ssp->get_user())){
              resp_json["optype"]="room_ready";
              resp_json["reason"]="玩家重复登陆";
              resp_json["result"]=false;
              return ws_resp(conn,resp_json);
            }
            //判断当前用户是否创建好了房间
            room_ptr rp=_rm.get_room_by_uid(ssp->get_user());
            if(rp.get()==nullptr){
              resp_json["optype"]="room_ready";
              resp_json["reason"]="没有玩家的房间信息";
              resp_json["result"]=false;
              return ws_resp(conn,resp_json);
            }
            //将当前用户添加到在线用户管理的游戏房间中
            _og.join_game_room(ssp->get_user(),conn); 
            //将session重新设置为永久存在
            DLOG("ssid: %d,set SESSION_FOREVER",(int)ssp->ssid());
            _sm.set_session_expire_time(ssp->ssid(),SESSION_FOREVER);
            //回复房间准备完毕
            resp_json["optype"]="room_ready";
            resp_json["result"]=true;
            resp_json["room_id"]=(Json::UInt64)rp->id();
            resp_json["uid"]=(Json::UInt64)ssp->get_user();
            resp_json["white_id"]=(Json::UInt64)rp->get_white_user();
            resp_json["black_id"]=(Json::UInt64)rp->get_black_user();
            return ws_resp(conn,resp_json);
        }
        void wsopen_callback(websocketpp::connection_hdl hd1){
            //websocket长连接建立成功之后的处理函数
            wsserver_t::connection_ptr conn=_wssrv.get_con_from_hdl(hd1);
            websocketpp::http::parser::request req=conn->get_request();
            std::string uri=req.get_uri();
            if(uri=="/hall"){
                //建立游戏大厅长连接
                return wsopen_game_hall(conn);
            }else if(uri=="/room"){
                //建立游戏房间长连接
                return wsopen_game_room(conn);
            }
        }
        void wsclose_game_hall(wsserver_t::connection_ptr conn){
            //游戏大厅长连接断开处理
            //1 登陆验证--判断当前客户端是否已经登陆成功
            session_ptr ssp=get_session_by_cookie(conn);
            if(ssp.get()==nullptr){
              return ;
            }
            //1 将玩家从游戏大厅中移除
            _og.exit_game_hall(ssp->get_user());
            //2 将session恢复生命周期的管理，设置定时销毁
            DLOG("ssid: %d,set SESSION_TIMEOUT",(int)ssp->ssid());
            _sm.set_session_expire_time(ssp->ssid(),SESSION_TIMEOUT);
        }
        void wsclose_game_room(wsserver_t::connection_ptr conn){
            //获取会话信息，识别客户端
            session_ptr ssp=get_session_by_cookie(conn);
            if(ssp.get()==nullptr){
              return ;
            }
            //将玩家从在线用户的房间中移除
            _og.exit_game_room(ssp->get_user());
            //将session恢复生命周期，设置定时销毁
            DLOG("ssid: %d,set SESSION_TIMEOUT",(int)ssp->ssid());
            _sm.set_session_expire_time(ssp->ssid(),SESSION_TIMEOUT);
            //将玩家从房间中移除，房间中所有用户退出了就销毁房间
            _rm.remove_room_user(ssp->get_user());

        }
        void wsclose_callback(websocketpp::connection_hdl hd1){
            //websocket连接断开前的处理函数
            wsserver_t::connection_ptr conn=_wssrv.get_con_from_hdl(hd1);
            websocketpp::http::parser::request req=conn->get_request();
            std::string uri=req.get_uri();
            if(uri=="/hall"){
                //游戏大厅断开连接
                return wsclose_game_hall(conn);
            }else if(uri=="/room"){
                //游戏房间断开连接
                return wsclose_game_room(conn);
            }
        }
        void wsmsg_game_hall(wsserver_t::connection_ptr conn,wsserver_t::message_ptr msg){
            //1 身份验证
            session_ptr ssp=get_session_by_cookie(conn);
            if(ssp.get()==nullptr){
              return ;
            }
            //获取请求信息
            Json::Value resp_json;
            std::string req_body=msg->get_payload();
            Json::Value req_json;
            bool ret=json_util::unserialize(req_body,req_json);
            if(ret==false){
              resp_json["result"]=false;
              resp_json["reason"]="请求信息解析失败";
              return ws_resp(conn,resp_json);
            }
            //对于请求进行处理
            if(!req_json["optype"].isNull()&&req_json["optype"].asString()=="match_start"){
              //开始对战匹配：通过匹配模块，将用户添加到匹配队列中
              DLOG("用户id:%d  在匹配中",int(ssp->get_user()));
              _mm.add(ssp->get_user());
              resp_json["result"]=true;
              resp_json["optype"]="match_start";
              return ws_resp(conn,resp_json);
            }else if(!req_json["optype"].isNull()&&req_json["optype"].asString()=="match_stop"){
              //停止对战匹配：通过匹配模块，将用户匹配队列移除 
              DLOG("用户id:%d 退出匹配",int(ssp->get_user()));
              _mm.del(ssp->get_user());
              resp_json["result"]=true;
              resp_json["optype"]="match_stop";
              return ws_resp(conn,resp_json);
            }
            resp_json["result"]=false;
            resp_json["reason"] = "请求类型未知";
            resp_json["optype"]="unknow";
            return ws_resp(conn,resp_json);
        }
        void wsmsg_game_room(wsserver_t::connection_ptr conn,wsserver_t::message_ptr msg){
            //获取客户端session，识别客户端身份
            session_ptr ssp=get_session_by_cookie(conn);
            if(ssp.get()==nullptr){
              return ;
            }
            //获取客户端房间信息
            room_ptr rp=_rm.get_room_by_uid(ssp->get_user());
            Json::Value resp_json;
            if(rp.get()==nullptr){
              resp_json["optype"]="unknow";
              resp_json["result"]=false;
              resp_json["reason"]="没有找到玩家的房间信息";
              return ws_resp(conn,resp_json);
            }
            //对消息进行反序列化
            Json::Value req_json;
            std::string req_body=msg->get_payload();
            bool ret=json_util::unserialize(req_body,req_json);
            if(ret==false){
              resp_json["optype"]="unknow";
              resp_json["result"]=false;
              resp_json["reason"]="请求解析失败";
              return ws_resp(conn,resp_json);
            }
            //通过房间模块进行消息请求处理
            rp->handle_request(req_json); 
        }
        void wsmsg_callback(websocketpp::connection_hdl hd1,wsserver_t::message_ptr msg){
            //websocket长连接通信处理
            wsserver_t::connection_ptr conn=_wssrv.get_con_from_hdl(hd1);
            websocketpp::http::parser::request req=conn->get_request();
            std::string uri=req.get_uri();
            if(uri=="/hall"){
                return wsmsg_game_hall(conn,msg);
            }else if(uri=="/room"){
                return wsmsg_game_room(conn,msg);
            }
        }
    public:
        gobang_server(const std::string&host,const std::string &user,const std::string &pass,const std::string &dbname,uint64_t port=3306,
                const std::string &wwwroot = WWWROOT):_ut(host,user,pass,dbname,port),_rm(&_ut,&_og),_sm(&_wssrv),_mm(&_rm,&_ut,&_og),_www_root(wwwroot)
        {
            _wssrv.set_access_channels(websocketpp::log::alevel::none);
            _wssrv.init_asio();
            _wssrv.set_reuse_addr(true);
            _wssrv.set_http_handler(std::bind(&gobang_server::http_callback,this,std::placeholders::_1));
            _wssrv.set_open_handler(std::bind(&gobang_server::wsopen_callback,this,std::placeholders::_1));
            _wssrv.set_close_handler(std::bind(&gobang_server::wsclose_callback,this,std::placeholders::_1));
            _wssrv.set_message_handler(std::bind(&gobang_server::wsmsg_callback,this,std::placeholders::_1,std::placeholders::_2));
        }
        void start(int port)
        {
            _wssrv.listen(port);
            _wssrv.start_accept();
            _wssrv.run();
        }
};
