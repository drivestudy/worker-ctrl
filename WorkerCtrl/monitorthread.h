#ifndef MONITORTHREAD_H
#define MONITORTHREAD_H

#include <QThread>
#include <common.h>
#include <cjson.h>
#include <caes.h>

class MonitorThread : public QThread
{
    Q_OBJECT
public:
    explicit MonitorThread(QObject *parent = 0);
    explicit MonitorThread(const QString &ip, const QString &pwd,
                           ushort srvPort, ushort ctrlPort, int period);
    ~MonitorThread();

    //设置服务器信息
    void setServerInfo(const QString &ip, const QString &pwd,
                       ushort srvPort, ushort ctrlPort, int period);

    //修改服务器信息
    void modifyServerInfo(const QString &pwd, ushort srvPort, ushort ctrlPort, int period);

    //设置退出标志
    void setExitFlag();

    //是否监控
    bool isMonitor();

    //开始监控
    void startMonitor();

    //停止监控
    void stopMonitor();

    //公用数据信息
    ServerInfo m_srvInfo;
    
signals:
    //开始监控信号
    void signalStartMonitor(const QString &ip);

    //停止监控信号
    void signalStopMonitor(const QString &ip);

    //更新服务器信息
    void signalUpdateServerInfo(bool online, const ServerInfo &srvInfo);

    //状态切换
    void signalChangeState(bool online, const QString &ip, const QString &version);

protected:
    virtual void run();

private:
    //func
    //初始化变量
    void initVariable();

    //监控
    void monitorServer();

    //与服务器对话
    bool talkWithServer(const string &cmd, QString &recv);

    //生成请求命令
    string GenCmdRequest(CMDTYPE cmdtype, const QString &pwd, int aesLevel=LEVEL_256);

    //清空状态信息
    void clearServerState();

    //记录状态的切换:在线 离线
    void changeState(bool online);

    //variable
    bool m_bRun;//运行标志
    bool m_bOnline;//是否在线
    QMutex m_finishMutex;
};

typedef QList<MonitorThread *> MonitorThreadList;

#endif // MONITORTHREAD_H
