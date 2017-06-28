#ifndef COMMON_H
#define COMMON_H
#include <QtCore>
#include <QtGui>
#include <QtSql>
#include <QtNetwork>
#include <QtXml>

#include <iostream>
#include <string>
using namespace std;

#define APP_NAME    "中控"
#define APP_VERSION "v2.2"
#define SET_FILE    "config.xml"

#define DEBUG_FUNCTION  qDebug() << __FUNCTION__;

//服务器信息结构体
typedef struct {
    QString     ip;
    ushort      srvPort;//服务端口号
    ushort      ctrlPort;//控制端口号
    QString     ctrlPwd;//控制密码
} Server;

typedef struct {
    //webhleper信息
    int         taskCount;//任务数量
    int         viewCount;//访问总数
    int         opTaskCount;//操作任务数量
    int         threadPoolSize;//线程池大小
    int         workThreadCount;//工作线程数
    int         runTime;//运行时间
    int         threadPoolLoad;//线程池负载
    int         unGetResultNum;//未获取结果数
    int         resultCacheCount;//结果缓存数
    uint        sendDataSize;//发送数据的大小
    uint        recvDataSize;//接收数据的大小
    QString     version;//版本
} RtInfo;

typedef struct {
    Server      server;
    RtInfo      rtInfo;

    QString     runState;//运行状态:在线 离线
    int         monitorPeriod;//监控周期
    QString     monitorState;//监控状态:监控中 未监控

} ServerInfo;

typedef QList<Server *> ServerList;

/*cmd protocol协议*/
typedef enum
{
    CMD_GETRTDATA,  //获取实时数据,
    CMD_HEARTBEAT,	//心跳测试
    CMD_CLOSE,		//关闭
    CMD_REFUSETASK, //拒绝接收任务
    CMD_SETPARAM,	//设置参数
    CMD_VERSION,	//获取版本信息
    CMD_UNGETRESULTNUM, //获取未被拉取的任务结果数
    CMD_RESULTCACHENUM,  //结果缓存数
    //新增协议,从最后面添加
    CMD_GETPARAM    //获取参数
}CMDTYPE;

//命令格式
#define CMDREQ	"cmdreq"
#define CMDRES	"cmdres"
#define CMDEND	"-#FI#"
//心跳测试程序的状态
#define STATE_START	"start"
#define STATE_STOP	"stop"
//命令响应的返回值
#define RES_OK	1
#define RES_NO	0
//操作密码
#define UI_OP_PASSWORD    "haoying123"

/**
协议:
请求:
{
    "cmd" : 命令,
    "pwd" : 控制密码,
    "opt" : {
        //可变区域,json对象
    }
}

请求带参数:
{
    "cmd" : 命令,
    "pwd" : 控制密码,
    "opt" : {
        "sql" : {
            "ip" : "192.168.1.1",
            "port" : 3306,
            "user" : "root",
            "pwd"  : "123456",
            "dbname" : "webhelper"
        },
        "cookiesrv" :
        ["192.168.1.1:26128", "192.168.1.1:26128", "192.168.1.1:26128", "192.168.1.1:26128"]
    }
}


响应:
{
    "info" : 信息,
    "result" : 1 | 0, //1成功 0失败
}

实时信息的格式:
{
    "info" : {
        "taskcount" : 0, //任务数量
        "viewcount" : 0, //访问总数
        "opTaskCount" : 0, //操作任务数量
        "threadpoolsize" : 0,//线程池大小
        "workthreadcount" : 0,//工作线程数
        "threadpoolload" : 0/1/2,//线程池负载
        "sendsize" : 1234,//发送数据大小
        "recvsize" : 1234,//接收数据大小
        "runtime" : 1234,//运行时间
        "ungetresultnum" : 12,//未获取结果数
        "resultcachecount" : 11,//结果缓存数
        "version" : "webhelper-v1.9"//版本
    },
    "result" : 1 | 0, //1成功 0失败
}

系统参数的格式:
{
    "info" : {
        "sql" : {
            "ip" : "192.168.1.1",
            "port" : 3306,
            "user" : "root",
            "pwd"  : "123456",
            "dbname" : "webhelper"
        },
        "cookiesrv" :
        ["192.168.1.1:26128", "192.168.1.1:26128", "192.168.1.1:26128", "192.168.1.1:26128"]
    },
    "result" : 1 | 0
}

*/


#endif // COMMON_H
