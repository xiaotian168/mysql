#include "mysqltool.h"

mysqltool* mysqltool::instance = new mysqltool;

mysqltool::mysqltool()
{
    //实例化数据库对象
    mysql = mysql_init(NULL);
    //连接数据库；获取数据库对象
    mysql_real_connect(mysql, "127.0.0.1", "root", "itcast", "test", 3306, NULL, 0);
    if(NULL == mysql)
    {
        cout << "connect error" << endl;
    }

    //设置utf8编码格式
    mysql_query(mysql, "set names utf8");
}

mysqltool::mysqltool(mysqltool&)
{
}

mysqltool::~mysqltool()
{
    //关闭数据库连接
    mysql_close(mysql);
}

mysqltool* mysqltool::getInstance()
{
    return instance;
}

//用于登录验证查询；并且返回用户id给外部
int mysqltool::select(char *sql, int &id)
{
    if(mysql_query(mysql, sql))
    {
        return -1;
    }

    MYSQL_RES *result = mysql_store_result(mysql);  //注意释放结果集合 -->mysql_free_result(mysql)
    MYSQL_ROW row = mysql_fetch_row(result);
    if(row)
    {
        id = atoi(row[0]);
    }

    return 0;
}

//查询；并且返回结果
int mysqltool::select(char *sql, char *result_buf)
{
    if(mysql_query(mysql, sql))
    {
        return -1;
    }

    MYSQL_RES *result = mysql_store_result(mysql);
    MYSQL_ROW row;
    while(row = mysql_fetch_row(result))
    {
        sprintf(result_buf, "%s%s\n", result_buf, row[0]);  //这里，服务器的字段只有一个；即一行只有一列数据
    }
    mysql_free_result(result);

    return 0;
}

int mysqltool::insert(char *sql)
{
    return mysql_query(mysql, sql);
}
