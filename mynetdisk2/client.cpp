#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <openssl/md5.h>
#include <iomanip>

using namespace std;

//封装建立socket通信
int getSockServer()
{
    //建立socket通信
    int server = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9527);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    //连接服务器
    connect(server, (struct sockaddr*)&addr, sizeof(addr));

    return server;
}

//获取文件md5校验信息
void get_md5(const char *path, char *md5)
{
    //获取文件的md5
    MD5_CTX c;
    MD5_Init(&c);
    unsigned char md5_tmp[17] = {0};    //校验码长度为16
    int len = 0;
    char buffer[1024] = {0};
    int fd = open(path, O_RDONLY);
    while(len = read(fd, buffer, sizeof(buffer)))
    {
        MD5_Update(&c, buffer, len);
    }

    MD5_Final(md5_tmp, &c);
    close(fd);

    for(int i = 0; i < 16; ++i) //追加获取所有校验码
    {
        sprintf(md5, "%s%02x", md5, md5_tmp[i]);
        //cout << hex << setw(2) << setfill('0') << (int)md5_tmp[i];
    }
}

//./server.out username password
int main(int argc, char* argv[])
{
    //1.程序运行参数是否合法
    if(argc != 3)
    {
        cout << "argument error" << endl;
        return -1;
    }

    //2.获取建立与服务器通信的socket
    int server = getSockServer(); 
    if(server < 0)
    {
        cout << "server connect error\n";
        return -2;
    }

    //3.定义3个缓冲区；用来向服务器收发数据和保存登录成功后的命令行命令
    char send_buf[1024] = {0};
    char recv_buf[1024] = {0};
    char cmd[1024] = {0};    //获取控制台输入命令

    //4.发送本客户端向服务器登录的验证消息；
    //报文格式: login username password
    sprintf(send_buf, "login %s %s", argv[1], argv[2]);
    send(server, send_buf, strlen(send_buf), 0);
    //就收服务器的登录验证回应消息
    recv(server, recv_buf, sizeof(recv_buf), 0);
    cout << recv_buf << endl;


    //5.登录网盘服务器成功，进入仿终端界面
    if(strcmp(recv_buf, "login success") == 0)
    {
        while(1)
        {
            memset(recv_buf, 0, sizeof(recv_buf));
            cout << "server > ";
            fgets(cmd, sizeof(cmd), stdin);
            cmd[strlen(cmd)-1] = '\0';  //去除'\n'
            char *type = strtok(cmd, " ");
            if(strcmp(type, "list") == 0)    //查看网盘文件
            {
                strcpy(send_buf, "list");
                send(server, send_buf, strlen(send_buf), 0);
                recv(server, recv_buf, sizeof(recv_buf), 0);
                cout << recv_buf << endl;
            }
            else if(strcmp(type, "upload") == 0)    //上传文件到网盘
            {
                //upload 本地路径 网盘路径
                char *path = strtok(NULL, " ");  //本地路径
                char *src_path = strtok(NULL, " ");  //网盘路径
                char *file_name = rindex(src_path, '/') + 1; //得到文件名
                //文件属性结构体
                struct stat st;
                stat(path, &st); 
                int file_size = st.st_size;  //获得要上传文件的大小
                char md5[1024] = {0};
                get_md5(path, md5);  //或的文件的md5校验信息
                //拼接字符串，把文件信息发到服务器
                sprintf(send_buf, "upload %s %s %d %s", file_name, src_path, file_size, md5);
                send(server, send_buf, strlen(send_buf), 0);
                recv(server, recv_buf, sizeof(recv_buf), 0);
                cout << recv_buf << endl;
                if(strcmp(recv_buf, "upload success") == 0 || strcmp(recv_buf, "upload failed") == 0)
                {
                    //如果服务器已存在文件，直接转存，返回成功或失败
                    cout << recv_buf << endl;
                    continue;
                }
                else if(strcmp(recv_buf, "upload file") == 0)
                {
                    //如果服务器没有该文件，则发回upload file，要求客户端上传文件
                    int fd = open(path, O_RDONLY);   //获得要上传文件描述符

                    //开始向服务器上传文件
                    cout << "1\n";
                    while(file_size)
                    {
                        char buffer[1024] = {0};
                        cout << "1\n";
                        int ret = read(fd, buffer, sizeof(buffer));
                        cout << "2\n";
                        send(server, buffer, ret, 0);
                        file_size -= ret;
                    }
                    cout << "2\n";
                    close(fd);

                    memset(recv_buf, 0, sizeof(recv_buf));
                    recv(server, recv_buf, sizeof(recv_buf), 0);
                    if(strcmp(recv_buf, "upload success") == 0 || strcmp(recv_buf, "upload failed") == 0)
                    {
                        //如果服务器已存在文件，直接转存，返回成功或失败
                        cout << recv_buf << endl;
                        continue;
                    }

                }

            }
            else if(strcmp(type, "download") == 0)
            {
                //download 网盘路径 本地路径
                char *src_path = strtok(NULL, " ");
                char *path = strtok(NULL, " ");
                cout << src_path << endl;
                cout << path << endl;
            }
            else if(strcmp(type, "exit") == 0)
            {
                return 0;
            }

            memset(recv_buf, 0, sizeof(recv_buf));  //清空接收缓冲区内容
        }
    }
    else
    {

    }

    return 0;
}

