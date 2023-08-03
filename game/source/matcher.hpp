#pragma once 
#include "util.hpp"
#include "room.hpp"
#include <thread>
#include <list>
#include <mutex>
#include <condition_variable>

template <class T>
class match_queue{
    private:
        //用链表而不直接使用queue是因为我们有中间删除数据的需要
        std::list<T> _list;
        //实现线程安全
        std::mutex _mutex;
        //这个条件变量主要为了阻塞消费者，后边使用的时候：队列元素个数<2则阻塞    
        std::condition_variable _cond;
    public:
        //获取元素个数
        int size(){
            std::unique_lock<std::mutex> lock(_mutex);
            return _list.size();
        }
        //判断是否为空
        bool empty(){
            std::unique_lock<std::mutex> lock(_mutex);
            return _list.empty();
        }
        //阻塞线程
        void wait(){
            std::unique_lock<std::mutex> lock(_mutex);
            _cond.wait(lock);
        }
        //入队数据，并唤醒线程
        bool push(const T &data){
            std::unique_lock<std::mutex> lock(_mutex);
            _list.push_back(data);
            _cond.notify_all();
        }
        //出队数据
        bool pop(T &data){
            std::unique_lock<std::mutex> lock(_mutex);
             if(_list.empty()==true)return false;
             data=_list.front();
             _list.pop_front();
             return true;
        }
        //移除指定的数据
        bool remove(T &data){
            std::unique_lock<std::mutex> lock(_mutex);
            _list.remove(data);
            return true;
        }
};

class matcher{
    private:
        //普通选手匹配队列
        match_queue<uint64_t> _q_normal;
        //高手匹配队列
        match_queue<uint64_t> _q_high;
        //大神匹配队列
        match_queue<uint64_t> _q_super;
        //对应三个匹配队列的处理线程
        std::thread _th_normal;
        std::thread _th_high;
        std::thread _th_super;
        room_manager* _rm;
        user_table* _ut;
        online_game* _og;
    private:
        void handle_match(match_queue<uint64_t> &mq){
            while(1)
            {
                while(mq.size()<2){
                    mq.wait();
                }    
                uint64_t uid1,uid2;
                bool ret=mq.pop(uid1);
                if(ret==false){
                    continue;
                }
                ret=mq.pop(uid2);
                if(ret==false){
                    this->add(uid1);
                    continue;
                }
                wsserver_t::connection_ptr conn1=_og->get_conn_from_hall(uid1);
                if(conn1.get()==nullptr){
                    this->add(uid2);
                    continue;
                }
                wsserver_t::connection_ptr conn2=_og->get_conn_from_hall(uid2);
                if(conn2.get()==nullptr){
                    this->add(uid1);
                    continue;
                }
                room_ptr rp=_rm->create_room(uid1,uid2);
                if(rp.get()==nullptr){
                    this->add(uid1);
                    this->add(uid2);
                    continue;
                }
                Json::Value resp;
                resp["optype"]="match_success";
                DLOG("match_success");
                resp["result"]=true;
                std::string body;
                json_util::serialize(resp,body);
                conn1->send(body);
                conn2->send(body);
            }
        }
        void th_normal_entry(){ handle_match(_q_normal); }
        void th_high_entry(){ handle_match(_q_high); }
        void th_super_entry(){ handle_match(_q_super); }
    public:
        matcher(room_manager* rm,user_table* ut,online_game* og)
                :_rm(rm),_ut(ut),_og(og)
            ,_th_normal(std::thread(&matcher::th_normal_entry,this))
            ,_th_high(std::thread(&matcher::th_high_entry,this))
            ,_th_super(std::thread(&matcher::th_super_entry,this))
            {
                 DLOG("游戏匹配模块初始化完毕...");
            }
        bool add(uint64_t uid){
            Json::Value user;
            bool ret=_ut->select_by_id(uid,user);
            if(ret==false){
                return false;
            }
            int score=user["score"].asInt();
            if(score<2000){
                DLOG("push uid:%d",(int)uid);
                _q_normal.push(uid);
            }else if(score>=2000&&score<3000){
                _q_high.push(uid);
            }else {
                _q_super.push(uid);
            }
            return true;
        }
        bool del(uint64_t uid){
            Json::Value user;
            bool ret=_ut->select_by_id(uid,user);
            if(ret==false){
                return false;
            }
            int score=user["score"].asInt();
            if(score<2000){
                _q_normal.remove(uid);
            }else if(score>=2000&&score<3000){
                _q_high.remove(uid);
            }else {
                _q_super.remove(uid);
            }
            return true;
        }
};
