#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include "mysqltool.h"

int main()
{
    //初始化socket通信
    int server = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9527);
    addr.sin_addr.s_addr = 0;   //0与INADDR_ANY 一样；都表示以任意网卡进行监听
    bind(server, (struct sockaddr*)&addr, sizeof(addr));
    listen(server, 250);  //监听；客户端连接数量最多250

    //接收客户端连接
    int client = accept(server, NULL, NULL);
    if(client >= 0)
    {
        cout << "已同意客户端连接请求" << endl;
    }

    //获取mysqltool工具
    mysqltool *mysql = mysqltool::getInstance();

    char recv_buf[1024] = {0};    //用于接收客户端消息
    char send_buf[1024] = {0};    //用户给客户端发消息
    char sql_buf[1024] = {0};   //用于构建sql命令
    char result_buf[1024] = {0};    //用户查询命令返回结果集
    char username[1024] = {0};  //保存客户端用户名

    while(1)
    {
        //重要：清空缓存
        memset(recv_buf, 0, sizeof(recv_buf));
        memset(send_buf, 0, sizeof(send_buf));
        //接收用户登录信息
        recv(client, recv_buf, sizeof(recv_buf), 0);
        cout << 1 << endl;
        cout << recv_buf << endl;
        char *type = strtok(recv_buf, " "); //获取用户操作类型
        //验证登录
        if(strcmp(type, "login") == 0)
        {
            strcpy(username, strtok(NULL, " "));
            char *password = strtok(NULL, " ");
            sprintf(sql_buf, "select * from user where username = '%s' and password = '%s'", username, password);
            cout << sql_buf << endl;
            if(mysql->select(sql_buf))
            {
                strcpy(send_buf, "login failed");
            }
            else
            {
                strcpy(send_buf, "login success");
            }

            send(client, send_buf, strlen(send_buf), 0);
        }
        else if(strcmp(type, "list") == 0)  //查看网盘文件
        {
            sprintf(sql_buf, "select file_name from user_file where userid = (select id from user where username = '%s')", username);
            if(mysql->select(sql_buf, result_buf))
            {
                strcpy(send_buf, "list failed");
                send(client, send_buf, strlen(send_buf), 0); 
            }
            else
            {
                cout << "test" << endl;
                send(client, result_buf, strlen(result_buf), 0);
            }
        }
        else
        {
            printf("3\n");
        }
    }

    return 0;
}
