#pragma once 

#include "util.hpp"
#include <unordered_map>
#include <mutex>

typedef enum{UNLOGIN,LOGIN}ss_statu;

class session{
    private:
        uint64_t _ssid;//标识符 
        uint64_t _uid;//session对应的用户ID
        ss_statu _statu; //用户登陆状态：未登陆/已登陆
        wsserver_t::timer_ptr _tp;//session关联定时器
    public:
        session(uint64_t ssid):_ssid(ssid){DLOG("session %p 被创建!!",this);}
        ~session(){DLOG("session %p 被释放!!",this);}
        uint64_t ssid(){return _ssid;}
        void set_statu(ss_statu statu){ _statu=statu; }
        void set_user(uint64_t uid){ _uid=uid; }
        uint64_t get_user(){ return _uid; }
        bool is_login(){ return (_statu==LOGIN); }
        void set_timer(const wsserver_t::timer_ptr &tp){ _tp=tp; }
        wsserver_t::timer_ptr& get_timer(){ return _tp; }
};
#define SESSION_TIMEOUT 30000
#define SESSION_FOREVER -1

using session_ptr=std::shared_ptr<session>;
class session_manager{
    private:
        uint64_t _next_ssid;
        std::mutex _mutex;
        std::unordered_map<uint64_t,session_ptr> _session;
        wsserver_t* _server;
    public:
        session_manager(wsserver_t* srv)
            :_next_ssid(1),_server(srv){
                DLOG("session管理初始化完毕!");
            }
        ~session_manager(){ DLOG("session管理区即将销毁!"); }
        session_ptr create_session(uint64_t uid,ss_statu statu){
            std::unique_lock<std::mutex> lock(_mutex);
            session_ptr ssp(new session(_next_ssid));
            ssp->set_statu(statu); 
            ssp->set_user(uid); 
            _session.insert(std::make_pair(_next_ssid,ssp));
            _next_ssid++;
            return ssp;
        }
        session_ptr get_session_by_ssid(uint64_t ssid){
            std::unique_lock<std::mutex> lock(_mutex);
            auto it=_session.find(ssid);
            if(it==_session.end()){
                return session_ptr();
            }
            return _session[ssid];
        } 
        void set_session_expire_time(uint64_t ssid,int ms){
            //依赖于websocketpp的定时器来完成session生命周期的管理。
            //登陆之后，创建session，session需要在指定时间无通信后删除
            //但是进入游戏大厅/游戏房间，session应该永久存在
            //等到退出游戏大厅/游戏房间，session应该被重新设置为临时，在长时间无通信后被删除
            session_ptr ssp=get_session_by_ssid(ssid);
            if(ssp.get()==nullptr){
                return;
            }
            wsserver_t::timer_ptr tp=ssp->get_timer();
            //1 session永久情况下，设置永久
            if(tp.get()==nullptr&&ms==SESSION_FOREVER){
                return;
            }
            else if(tp.get()==nullptr&&ms!=SESSION_FOREVER){
                //2 session永久情况下，设置指定时间之后被删除的定时任务
                wsserver_t::timer_ptr time_tp=_server->set_timer(ms,std::bind(&session_manager::remove_session,this,ssid));
                ssp->set_timer(time_tp);
            }
            else if(tp.get()!=nullptr&&ms==SESSION_FOREVER){
                //3 session设置了定时删除的情况下，将session设置永久存在
                //删除定时任务-----stready_timer删除定时任务导致直接被执行
                tp->cancel();
                ssp->set_timer(wsserver_t::timer_ptr());
                //因此重新给session管理器中，添加一个session信息
                //_session.insert(std::make_pair(ssp->ssid(),ssp));
                _server->set_timer(0,std::bind(&session_manager::append_session,this,ssp));
            }
            else if(tp.get()!=nullptr && ms!=SESSION_FOREVER){
                //4 session设置了定时删除的情况下，将session重制删除时间
                tp->cancel();
                ssp->set_timer(wsserver_t::timer_ptr());
                _server->set_timer(0,std::bind(&session_manager::append_session,this,ssp));
                wsserver_t::timer_ptr timer_tp=_server->set_timer(ms,std::bind(&session_manager::remove_session,this,ssp->ssid()));
                ssp->set_timer(timer_tp);
            }
        }
        void append_session(const session_ptr& sp){
            std::unique_lock<std::mutex> lock(_mutex);
            _session.insert(std::make_pair(sp->ssid(),sp));
        }
        void remove_session(uint64_t ssid){
            std::unique_lock<std::mutex> lock(_mutex);
            _session.erase(ssid);
        }
};
