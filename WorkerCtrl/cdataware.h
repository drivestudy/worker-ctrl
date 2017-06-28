#ifndef CDATAWARE_H
#define CDATAWARE_H

#include <common.h>
#include <monitorthread.h>

class CDataWare : public QObject
{
    Q_OBJECT

public:
    CDataWare();
    ~CDataWare();
    static CDataWare *instance();
    static void destroy();

    //设置日志数据库参数
    void setLogSqlParam(const QString &ip, const QString &user, const QString &pwd, const QString &dbname, ushort port);

    //获取日志数据库参数
    void getLogSqlParam(QString &ip, QString &user, QString &pwd, QString &dbname, ushort &port);

    //获取日志数据库对象
    bool getLogSqlDb(QSqlDatabase &sqlDb);

    //操作类型转换
    QString optypeString(uint type);

    //服务器增删改查
    MonitorThread *appendServer(const QString &ip, ushort srvPort, ushort ctrlPort, int monitorPeriod, const QString &ctrlPwd);
    bool modifyServer(const QString &ip, ushort srvPort, ushort ctrlPort, int monitorPeriod, const QString &ctrlPwd);
    bool removeServer(const QString &ip);
    MonitorThread *findServer(const QString &ip);
    bool hasServer(const QString &ip);

    //获取服务器信息
    Server *getServer(const QString &ip);

    //返回服务器信息链表
    ServerList getServerList();

    //获取线程列表
    MonitorThreadList getMonitorThreadList();

    //获取服务器列表
    QStringList getServerIpList();

private:    
    //variable
    MonitorThreadList m_monitorThreadList;//监控线程列表

    //日志数据库参数
    QString m_strSqlIp;
    QString m_strSqlUser;
    QString m_strSqlPwd;
    QString m_strSqlDbName;
    ushort  m_iSqlPort;

    //单例需要的变量
    static QMutex m_singleMutex;
    static CDataWare *m_pInstance;

    //垃圾工人,自动回收
    class CGarbo {
    public:
        ~CGarbo() {
            CDataWare::destroy();
        }
    };
    static CGarbo m_garbo;
};

#endif // CDATAWARE_H
