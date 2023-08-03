#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
using namespace std;

bool GetQueryString(std::string &query_string)
{
    bool result=false;
   cerr<<"Debug Test: "<<getenv("METHOD")<<endl;
    std::string method=getenv("METHOD");
    if(method=="GET")
    {
        query_string=getenv("QUERY_STRING");
        std::cerr<<"GET Debug query_string: "<<query_string<<std::endl;
        result=true;
    }
    else if(method=="POST")
    {
        std::cerr<<"Debug content_length: "<<getenv("CONTENT_LENGTH")<<std::endl;
        int content_length=atoi(getenv("CONTENT_LENGTH"));
        query_string=getenv("BODY");
       /* char ch=0;
        while(content_length--) 
        {
           read(0,&ch,1);
           query_string.push_back(ch);
        }*/
        std::cerr<<"POST Debug query_string: "<<query_string<<std::endl;
        result=true;
    }
    else 
    {
        result=false;
    } 
    return result;
}

void CutString(std::string& in,const std::string& op,std::string& out1,std::string& out2)
{
    size_t pos=in.find(op);
    if(pos!=std::string::npos)
    {
        out1=in.substr(0,pos);
        out2=in.substr(pos+op.size());
    }

}

int main()
{
    std::string query_string;
    GetQueryString(query_string);
    std::string str1;
    std::string str2;
    CutString(query_string,"&",str1,str2);
   
    std::string name1;
    std::string value1;
    CutString(str1,"=",name1,value1);

    std::string name2;
    std::string value2;
    CutString(str2,"=",name2,value2);
    
    cerr<<name1<<"->"<<value1<<endl;
    cerr<<name2<<"->"<<value2<<endl;

    cout<<name1<<"->"<<value1<<endl;
    cout<<name2<<"->"<<value2<<endl;
    return 0;
}
