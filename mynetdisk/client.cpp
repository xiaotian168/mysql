#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

using namespace std;

//./server.out username password
int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        cout << "argument error" << endl;
        return -1;
    }

    //建立socket通信
    int server = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9527);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    //连接服务器
    connect(server, (struct sockaddr*)&addr, sizeof(addr));

    char send_buf[1024] = {0};
    char recv_buf[1024] = {0};
    char cmd[1024] = {0};    //获取控制台输入

    sprintf(send_buf, "login %s %s", argv[1], argv[2]);
    send(server, send_buf, strlen(send_buf), 0);
    recv(server, recv_buf, sizeof(recv_buf), 0);

    cout << recv_buf << endl;
    if(strcmp(recv_buf, "login success") == 0)
    {
        while(1)
        {
            memset(recv_buf, 0, sizeof(recv_buf));
            cout << "server > ";
            fgets(cmd, sizeof(cmd), stdin);
            cmd[strlen(cmd)-1] = '\0';
            if(strcmp(cmd, "list") == 0)
            {
                strcpy(send_buf, "list");
                send(server, send_buf, strlen(send_buf), 0);
                recv(server, recv_buf, sizeof(recv_buf), 0);
                cout << recv_buf << endl;
            }
        }
    }

    return 0;
}

