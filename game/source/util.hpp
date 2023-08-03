#pragma once

#include "logger.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <memory>
#include <fstream>
#include <mysql/mysql.h>
#include <jsoncpp/json/json.h>
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>

typedef websocketpp::server<websocketpp::config::asio> wsserver_t;

class mysql_util{
    public:
        static MYSQL* mysql_create(const std::string &host,
                                    const std::string &username,
                                    const std::string &password,
                                    const std::string &dbname,
                                    uint16_t port=3306)
        {
            //1.初始化mysql句柄
            MYSQL* mysql=mysql_init(NULL);
            if(mysql==NULL){
                ELOG("mysql init failed!");
                return NULL;
            }
            //2.连接服务器
            if(mysql_real_connect(mysql,host.c_str(),username.c_str(),password.c_str(),dbname.c_str(),port,NULL,0)==NULL){
                ELOG("connect mysql failed : %s",mysql_error(mysql));
                return NULL;
            }
            //3.设置客户端字符集
            if(mysql_set_character_set(mysql,"utf8")!=0){
                ELOG("mysql set character utf8 failed : %s",mysql_error(mysql));
                return NULL;
            }
            return mysql;
        }
        static bool mysql_execl(MYSQL* mysql,const std::string sql)
        {
            //5.执行sql语句
            if(mysql_query(mysql,sql.c_str())!=0){
                DLOG("%s",sql.c_str());
                ELOG("mysql query failed : %s",mysql_error(mysql));
                return false;
            }
            //return true;
            
            //6.如果sql语句是查询语句，则需要保存结构到本地
            //MYSQL_RES* mysql_store_result(MYSQL* mysql);
            // MYSQL_RES* res=mysql_store_result(mysql); 
            // if(res==NULL){
            //     mysql_close(mysql);
            //     return -5;
            // }

            // //7.获取结果集中的结果条数
            // //int mysql_num_rows(MYSQL_RES* res);
            // //int mysql_num_fields(MYSQL_RES* res);
            // int num_row=mysql_num_rows(res); 
            // int num_col=mysql_num_fields(res);

            // //8.遍历保存到本地的结果集
            // for(int i=0;i<num_row;i++){
            //     //MYSQL_ROW mysql_fetch_row(MYSQL* res);
            //     MYSQL_ROW row=mysql_fetch_row(res);
            //     for(int i=0;i<num_col;i++){
            //         printf("%s\t",row[i]);
            //     }
            //     printf("\n");
            // }

            // //9.释放结果集
            // mysql_free_result(res);
            return true;
        }
        static void mysql_destroy(MYSQL* mysql)
        {
            if(mysql!=NULL)mysql_close(mysql);
        }
};


class json_util{
    public:
        static bool serialize(const Json::Value &root, std::string &str){
            //实例化一个StreamWriterBuilder工厂类
            Json::StreamWriterBuilder swb;

            //通过StreamWriterBuilder工厂类对象生产一个StreamWriter对象
            std::unique_ptr<Json::StreamWriter> sw(swb.newStreamWriter());
            
            //使用STreamWriter对象，对Json::Value中存储的数据进行序列化
            std::stringstream ss;
            int ret=sw->write(root,&ss);
            if(ret!=0)
            {
                ELOG("json serialize failed!");
                return false;
            }
            str= ss.str();
            return true;

        }
        static bool unserialize(const std::string &str,Json::Value &root){
            //实例化CharReaderBuilder工厂类对象
            Json::CharReaderBuilder crb;

            //使用CharReaderBuilderr工厂类对象生产一个CharReader对象
            std::unique_ptr<Json::CharReader> cr(crb.newCharReader());
            std::string err;
            bool ret=cr->parse(str.c_str(),str.c_str()+str.size(),&root,&err);
            if(ret == false)
            {
                ELOG("json unserialize failed : %s",err.c_str());
                return false;
            }
            return true;
        }
};


class string_util{
    public:
        static int split(const std::string &src,const std::string &sep,std::vector<std::string> &res)
        {
            size_t pos,index=0;
            while(index<src.size())
            {
                pos=src.find(sep,index);
                if(pos==std::string::npos)
                {
                    res.push_back(src.substr(index));
                    break;  
                }
                if(pos==index){
                    index+=sep.size();
                    continue;
                }
                res.push_back(src.substr(index,pos-index));
                index=pos+sep.size();
            }
            return res.size();
        }
};

class file_util{
    public:
        static bool read(const std::string& filename,std::string &body){
            //打开文件
            std::ifstream ifs(filename, std::ios::binary);
            if (ifs.is_open() == false) {
                ELOG("%s file open failed!!", filename.c_str());
                return false;
            }
            //获取文件大小
            size_t fsize = 0;
            ifs.seekg(0, std::ios::end);
            fsize = ifs.tellg();
            ifs.seekg(0, std::ios::beg);
            body.resize(fsize);
            //将文件所有数据读取出来
            ifs.read(&body[0], fsize);
            if (ifs.good() == false) {
                ELOG("read %s file content failed!", filename.c_str());
                ifs.close();
                return false;
            }
            //关闭文件
            ifs.close();
            return true;
        }    
};
