#include "echo.hpp"

int main()
{
    EchoServer server(8085);
    server.Start();
    return 0;
}
