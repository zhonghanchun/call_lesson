#include "server.hpp"

#define HOST "127.0.0.1"
#define PORT 3306
#define USER "root"
#define PASS "5201314zhc"
#define DBNAME "gobang" 

// void mysql_test()
// {
//     MYSQL* mysql=mysql_util::mysql_create(HOST,USER,PASS,DBNAME);
//     mysql_util::mysql_execl(mysql,"select * from stu;");
//     mysql_util::mysql_destroy(mysql);
// }
// void json_test()
// {
//     Json::Value root;
//     root["姓名"]="小明";
//     root["年龄"]=18;
//     root["成绩"].append(98);
//     root["成绩"].append(88.5);
//     root["成绩"].append(78.5);
//     std::string body;
//     json_util::serialize(root,body);
//     DLOG("%s",body.c_str());
    
//     Json::Value val;
//     json_util::unserialize(body,val);
//     std::cout<<"姓名:"<<val["姓名"].asString()<<std::endl;
//     std::cout<<"年龄:"<<val["年龄"].asInt()<<std::endl;
//     int sz=val["成绩"].size();
//     for(int i=0;i<sz;i++)
//         std::cout<<"成绩:"<<val["成绩"][i].asFloat()<<std::endl;
// }

// void string_util_test()
// {
//     const std::string str=",,,...,,,123,456,789,,,,";
//     std::vector<std::string> res;
//     string_util::split(str,",",res);
//     for(auto& ch : res)
//         std::cout<<ch<<std::endl;
// }

// void file_test()
// {
//     std::string filename = "./makefile";
//     std::string body;
//     file_util::read(filename,body);
//     std::cout<<body<<std::endl;
// }

// void db_test()
// {
//     user_table ut(HOST,USER,PASS,DBNAME,PORT);
//     //ut.win(1);
//     ut.lose(1);
// }

// void online_test()
// {
//     online_game og;
//     wsserver_t::connection_ptr conn;
//     uint64_t uid=2;
//     og.join_game_room(uid,conn); 
//     if(og.is_in_game_room(uid)){
//         DLOG("hall is 存在");
//     }else {
//         DLOG("hall not 不存在!");
//     }
//     og.exit_game_room(uid);
//     if(og.is_in_game_room(uid)){
//         DLOG("hall is 存在");
//     }else {
//         DLOG("hall not 不存在!");
//     } 
// }

// void room_test()
// {
//     user_table tb(HOST,USER,PASS,DBNAME,PORT);
//     online_game og;
//     //room r(10,&tb,&og);
//     room_mangager rm(&tb,&og);
//     room_ptr rp=rm.create_room(10,20);        
// }

int main()
{
    //mysql_test();
    //json_test();
    //string_util_test();
    //file_test();
    //db_test();
    //online_test();
    //room_test();
    //user_table tb(HOST,USER,PASS,DBNAME,PORT);
    //online_game og;
    //room_mangager rm(&tb,&og);
    //matcher m(&rm,&tb,&og);
    gobang_server server(HOST,USER,PASS,DBNAME,PORT);
    server.start(8085);
    return 0;
}
