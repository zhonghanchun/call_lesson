#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

void SetNonBlock(int fd)
{
    int fl=fcntl(fd,F_GETFL);
    if(fl<0)
    {
        printf("fl failed\n");
        return;
    }
    fcntl(fd,F_SETFL,fl|O_NONBLOCK);
}
int main()
{
    SetNonBlock(0);
    while(1)
    {
        char buffer[1024]={0};
        ssize_t s=read(0,buffer,sizeof(buffer));
        if(s>0)
        {
            buffer[s]=0;
            write(1,buffer,strlen(buffer));
        }
        else 
        {
            printf("hello world\n");
        }
        sleep(1);
    }
    return 0;
}
