#include <iostream>
#include <string>
#include <cstring>
#include <cerrno>
#include <cassert>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
using namespace std;
#define MYFILE "/home/zhc/cal_106/lesson2/proc_pool/myfifo"

bool create_fifo(const string& path)
{
    umask(0);
    int n=mkfifo(path.c_str(),0600);
    if(n==0)
        return true;
    cout<<"myfifo fail!"<<endl;
    return false;
}
void remove_fifo(const string& path)
{
    int n=unlink(path.c_str());
    assert(n==0);
    (void)n;
}

