#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/wait.h>
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


    //获取mysqltool工具
    mysqltool *mysql = mysqltool::getInstance();


    while(1)
    {
        //接收客户端连接
        int client = accept(server, NULL, NULL);
        if(client >= 0)
        {
            cout << "已同意客户端连接请求" << endl;
        }

        //子进程处理每个客户端的连接
        pid_t pidClient = fork();
        if(pidClient == 0)
        {
            char recv_buf[1024] = {0};    //用于接收客户端消息
            char send_buf[1024] = {0};    //用户给客户端发消息
            char sql_buf[1024] = {0};   //用于构建sql命令
            char result_buf[1024] = {0};    //用户查询命令返回结果集
            char username[1024] = {0}; //保存客户端用户名
            int userid = 0;

            while(1)    //处理客户端访问数据库指令
            {

                //接收用户操作数据库指令信息
                recv(client, recv_buf, sizeof(recv_buf), 0);
                char *type = strtok(recv_buf, " "); //获取用户操作类型
                cout << type << endl;

                //验证登录指令
                if(strcmp(type, "login") == 0)
                {
                    strcpy(username, strtok(NULL, " "));
                    char *password = strtok(NULL, " ");
                    sprintf(sql_buf, "select * from user where username = '%s' and password = '%s'", username, password);
                    cout << sql_buf << endl;
                    if(mysql->select(sql_buf, userid))  //sql语句执行失败，登录验证失败
                    {
                        strcpy(send_buf, "login failed");
                    }
                    else    //sql语句执行成功的情况
                    {
                        if(userid)
                            strcpy(send_buf, "login success");
                        else
                            strcpy(send_buf, "login login failed");
                    }

                    //回复客户端登录验证结果
                    send(client, send_buf, strlen(send_buf), 0);
                }
                else if(strcmp(type, "list") == 0)  //查看网盘文件指令
                {
                    sprintf(sql_buf, "select file_name from user_file where userid = (select id from user where username = '%s')", username);
                    if(mysql->select(sql_buf, result_buf))  //sql语句执行失败情况
                    {
                        strcpy(send_buf, "list failed");
                        send(client, send_buf, strlen(send_buf), 0); 
                    }
                    else
                    {
                        if(strlen(result_buf) == 0)
                            strcpy(send_buf, "empty set");
                        //发送查看结果给客户端
                        send(client, result_buf, strlen(result_buf), 0); 
                    }
                }
                else if(strcmp(type, "upload") == 0)    //上传文件指令
                {
                    //upload 文件名 网盘路径 文件大小 md5
                    char *file_name = strtok(NULL, " ");
                    char *src_path = strtok(NULL, " ");
                    int file_size = atoi(strtok(NULL, " "));
                    char *md5 = strtok(NULL, " ");
                    cout << file_name << " " << src_path << " " << file_size << " " << md5 << endl;

                    sprintf(sql_buf, "select dst_path from user_file where md5 = '%s'", md5);

                    if(mysql->select(sql_buf, result_buf))
                    {
                        cout << "upload select error" << endl;
                    }
                    else
                    {
                        if(strlen(result_buf))
                        {
                            //说明服务器上有文件，返回具体路径
                            result_buf[strlen(result_buf)-1] = '\0';
                        }
                        else
                        {
                            //说明服务器上没有和上传相同的文件，手动指定服务器路径
                            strcpy(send_buf, "upload file");
                            send(client, send_buf, strlen(send_buf), 0);
                            memset(result_buf, 0, sizeof(result_buf));
                            sprintf(result_buf, "./data/%s_%s", username, file_name);
                            int fd = open(result_buf, O_WRONLY | O_CREAT, 0777);
                            cout << "fd = " << fd << endl;
                            cout << result_buf << endl;
                            cout << "1\n";
                            int tmp = file_size;
                            cout << tmp << endl;
                            char buffer[1024] = {0};
                            while(tmp)  //接收客户端上传的文件到本地服务器
                            {
                                cout << 7 << endl;
                                int ret = recv(client, buffer, sizeof(buffer), 0);
                                cout << 8 << endl;
                                write(fd, buffer, ret);
                                tmp -= ret;
                                memset(buffer, 0, sizeof(recv_buf));
                            }
                            cout << "2\n";

                            close(fd);
                        }
                        memset(sql_buf, 0, sizeof(sql_buf));

                        sprintf(sql_buf, "insert into user_file values(NULL, %d, '%s', '%s', NULL, %d, '%s', '%s')", userid, file_name, src_path, file_size, result_buf, md5);
                        cout << sql_buf << endl;
                        if(mysql->insert(sql_buf))
                        {
                            strcpy(send_buf, "upload failed");
                        }
                        else
                        {
                            strcpy(send_buf, "upload success");
                        }

                        send(client, send_buf, strlen(send_buf), 0);
                        continue;
                    }
                }
                else if(strcmp(type, "download") == 0)
                {
                    //download 网盘路径
                    char *src_path = strtok(NULL, " ");

                }
                else 
                {
                    send(client, "无法解析的外部命令", 27, 0);
                }

                //重要：清空缓存
                memset(recv_buf, 0, sizeof(recv_buf));
                memset(send_buf, 0, sizeof(send_buf));
                memset(result_buf, 0, sizeof(result_buf));
            }
        }
    }

    //创建一个子进程，用于回收其他子进程
    pid_t pid = fork();
    if(pid == 0)    //循环阻塞等待回收其它进程
    {
        while(1)
            wait(NULL);
    }

    return 0;
}
