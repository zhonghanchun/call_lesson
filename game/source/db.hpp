#pragma once 
#include "util.hpp"
#include <mutex>
#include <assert.h>

class user_table{
    private:
        //mysql 操作句柄
        MYSQL* _mysql;

        //互斥锁保护数据库的访问操作
        std::mutex _mutex;
    public:
        user_table(const std::string &host,
                const std::string &username,
                const std::string &password,
                const std::string &dbname,
                uint16_t port=3306){
            _mysql=mysql_util::mysql_create(host,username,password,dbname,port);
            assert(_mysql!=NULL);
        }
        ~user_table(){
            mysql_util::mysql_destroy(_mysql);
            _mysql=NULL;
        }

        //注册时新增用户
        bool insert(Json::Value &user){
            #define INSERT_USER "insert into user values(null,'%s',password('%s'),1000,0,0);"
            if(user["username"].isNull()||user["password"].isNull()){
                DLOG("INPUT PASSWORD OR USERNAME");
                return false;
            }    

            char sql[4096]={0};
            sprintf(sql,INSERT_USER,user["username"].asCString(),user["password"].asCString());
            //在新增用户时看用户是否存在过，存在应出错，不存在新增注册
            // Json::Value val;
            // bool flag=select_by_name(user["username"].asCString(),val);
            // if(flag==true){
            //     DLOG("user:%s is already exists!",user["username"].asCString());
            //     return false;
            // }
            
            bool ret=mysql_util::mysql_execl(_mysql,sql);
            if(ret==false){
                DLOG("insert into user failed!");
                return false;
            }
            return true;
        }

        //登陆验证，并返回详细的用户信息
        bool login(Json::Value &user){
            #define LOGIN_USER "select id,score,total_count,win_count from user where username='%s' and password=password('%s');"
            if(user["username"].isNull()||user["password"].isNull()){
                DLOG("INPUT PASSWORD OR USERNAME");
                return false;
            }  

            char sql[4096]={0};
            sprintf(sql,LOGIN_USER,user["username"].asCString(),user["password"].asCString());
           
            MYSQL_RES* res=NULL;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                bool ret=mysql_util::mysql_execl(_mysql,sql);
                if(ret==false){
                    DLOG("user login failed!");
                    return false;
                }
                res=mysql_store_result(_mysql);
                if(res==NULL){
                    DLOG("have no login user info!");
                    return false;
                }
            }

            int num_row=mysql_num_rows(res);
            if(num_row!=1){
                DLOG("the user information queried is not unique!");
                return false;
            }
            MYSQL_ROW row=mysql_fetch_row(res);
            user["id"]=std::stoi(row[0]);
            user["score"]=std::stoi(row[1]);
            user["total_count"]=std::stoi(row[2]);
            user["win_count"]=std::stoi(row[3]);
            mysql_free_result(res);

            return true;
        }

        //通过用户名获取用户信息
        bool select_by_name(const std::string &name,Json::Value &user){
            #define USER_BY_NAME "select id,score,total_count,win_count from user where username='%s';"
            char sql[4096]={0};
            sprintf(sql,USER_BY_NAME,name.c_str());
            
            MYSQL_RES* res=NULL;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                bool ret=mysql_util::mysql_execl(_mysql,sql);
                if(ret==false){
                    DLOG("get user by name failed!");
                    return false;
                }
                res=mysql_store_result(_mysql);
                if(res==NULL){
                    DLOG("have no user info!");
                    return false;
                }
            }
            std::cout<<"select_by_name  res: "<<res<<std::endl;
            int row_num=mysql_num_rows(res);
            if(row_num!=1){
                DLOG("the user information queried is not unique!");
                return false;
            }
            MYSQL_ROW row=mysql_fetch_row(res);
            user["id"]=std::stoi(row[0]);
            user["username"]=name;
            user["score"]=std::stoi(row[1]);
            user["total_count"]=std::stoi(row[2]);
            user["win_count"]=std::stoi(row[3]);
            mysql_free_result(res);
            return true;
        }

        //通过用户名获取用户信息
        bool select_by_id(uint64_t id,Json::Value &user){
            #define USER_BY_ID "select username,score,total_count,win_count from user where id='%d';"
            char sql[4096]={0};
            sprintf(sql,USER_BY_ID,id);
            MYSQL_RES* res=NULL;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                bool ret=mysql_util::mysql_execl(_mysql,sql);
                if(ret==false){
                    DLOG("get user by id failed!");
                    return false;
                }
                res=mysql_store_result(_mysql);
                if(res==NULL){
                    DLOG("have no user info!");
                    return false;
                }
            }
            int num_row=mysql_num_rows(res);
            if(num_row!=1){
                DLOG("the user information queried is not unique!!");
                return false;
            }
            MYSQL_ROW row=mysql_fetch_row(res);
            user["id"]=(Json::UInt64)id;
            user["username"]=row[0];
            user["score"]=(Json::UInt64)std::stol(row[1]);
            user["total_count"]=std::stoi(row[2]);
            user["win_count"]=std::stoi(row[3]);
            mysql_free_result(res);
            return true;
        }

        //胜利时天梯分数增加，战斗场次增加，胜利场次增加
        bool win(int id){
            #define USER_WIN "update user set score=score+30,total_count=total_count+1,win_count=win_count+1 where id=%d;"
            char sql[4096]={0};
            sprintf(sql,USER_WIN,id);
            bool ret=mysql_util::mysql_execl(_mysql,sql);
            if(ret==false){
                DLOG("update win user info failed!");
                return false;
            }
            return true;
        }

        //失败时天梯分数减少，战斗场次减少，其他不变
        bool lose(int id){
            #define USER_LOSE "update user set score=score-30,total_count=total_count+1 where id=%d;"
            char sql[4096]={0};
            sprintf(sql,USER_LOSE,id);
            bool ret=mysql_util::mysql_execl(_mysql,sql);
            if(ret==false){
                DLOG("update lose user info failed!");
                return false;
            }
            return true;
        }
};
