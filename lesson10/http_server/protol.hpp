#include <iostream>
#include <string>
#include <sstream>
#include "Util.hpp"
using namespace std;
const string sep="\r\n";
const string root="./wwwroot";
const string home_page="index.html";
class Reques 
{
public:
    void parse()
    {
        string line=Util::get_one_line(req,sep);
        if(line.empty())return;
        std::stringstream ss(line);

        ss>>method>>url>>httpversion;
    
        path+=root;
        path+=url;
        if(path[path.size()-1]=='/')path+=home_page;
    }
public:
    string req;

    string method;
    string url;
    string httpversion;
    string path;
};


class Reponse
{
public:
    string rep;
};
