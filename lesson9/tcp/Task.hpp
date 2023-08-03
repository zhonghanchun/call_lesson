#include <iostream>
#include <functional>
#include <string>
using namespace std;

namespace Task_call
{

    void serviceIO(int sock)
    {
        char buffer[1024]={0};
        while(true)
        {
           ssize_t s=read(sock,buffer,sizeof(buffer));
            if(s>0)
            {
               buffer[s]=0;
               cout<<"Client# "<<buffer<<endl;

                string res="Server# ";
                res+=buffer;
                write(sock,res.c_str(),res.size());
            }
            else if(s==0)
            {
                cout<<"Client quuit!"<<endl;
                break;
            } 
        }
        close(sock);
    }
    class Tasks
    {
    public:
        //typedef function<int(int,int,char)> fun_t;
        typedef function<void(int)> fun_t;
        Tasks(){}
        Tasks(fun_t f,int sock):_sock(sock),_f(f)
        {}
        void operator()()
        {
            _f(_sock); 
        }
    private:
        int _sock;
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
}
