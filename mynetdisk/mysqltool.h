#ifndef _MYSQLTOOL_H_
#define _MYSQLTOOL_H_
#include <iostream>
#include <stdio.h>
#include <mysql.h>

using namespace std;

//单例
//1.构造、拷贝构造、析构函数私有化
//2.一个静态的成员函数（公有；供外部调用）
//3.一个静态的成员变量（私有） 

class mysqltool
{
    private:
        mysqltool();
        mysqltool(mysqltool &);
        ~mysqltool();

    public:
        static mysqltool* getInstance();
        int select(char *sql);
        int select(char *sql, char *result_buf);

    private:
        static mysqltool *instance;
        MYSQL *mysql;
};

#endif
