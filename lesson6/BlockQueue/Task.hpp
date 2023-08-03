#include <iostream>
#include <functional>
#include <string>
using namespace std;

class Task
{
public:
    typedef function<int(int,int,char)> fun_t;
    Task(){}
    Task(fun_t f,int x,int y,char op):_x(x),_y(y),_op(op),_f(f)
    {}
    std::string operator()()
    {
        int result=_f(_x,_y,_op);
        char buffer[1024]={0};
        snprintf(buffer,sizeof buffer,"%d %c %d = %d",_x,_op,_y,result);
        return buffer;
    }
    std::string Task_string()
    {
        char buffer[1024]={0};
        snprintf(buffer,sizeof buffer,"%d %c %d = ?",_x,_op,_y);
        return buffer;
    }
private:
    int _x;
    int _y;
    char _op;
    fun_t _f;
};
const std::string oper = "+-*/%";

int mymath(int x, int y, char op)
{
    int result = 0;
    switch (op)
    {
    case '+':
        result = x + y;
        break;
    case '-':
        result = x - y;
        break;
    case '*':
        result = x * y;
        break;
    case '/':
    {
        if (y == 0)
        {
            std::cerr << "div zero error!" << std::endl;
            result = -1;
        }
        else
            result = x / y;
    }
        break;
    case '%':
    {
        if (y == 0)
        {
            std::cerr << "mod zero error!" << std::endl;
            result = -1;
        }
        else
            result = x % y;
    }
        break;
    default:
        // do nothing
        break;
    }

    return result;
}
class SaveTask
{
    typedef std::function<void(const std::string&)> fun_t;
public:
    SaveTask(){}
    SaveTask(const std::string& message,fun_t func)
        :_message(message),_func(func)
    {}
    void operator()()
    {
        _func(_message);
    }
private:
    std::string _message;
    fun_t _func;
};

void Save(const std::string& message)
{
    const string target="./log.txt";
    FILE* fp=fopen(target.c_str(),"a+");
    if(!fp)
    {
        std::cout<<"fp fopen error!"<<std::endl;
        return;
    }
    fputs(message.c_str(),fp);
    fputs("\n",fp);
    fclose(fp);
}
