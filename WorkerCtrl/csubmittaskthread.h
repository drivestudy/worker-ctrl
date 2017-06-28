#ifndef CSUBMITTASKTHREAD_H
#define CSUBMITTASKTHREAD_H

#include <common.h>

class CSubmitTaskThread : public QThread
{
    Q_OBJECT
public:
    explicit CSubmitTaskThread(QObject *parent = 0);
    explicit CSubmitTaskThread(const QString &ip, ushort port,
                               const QString &master, const QString &pwd,
                               QObject *parent = 0);
    ~CSubmitTaskThread();

    //设置ip端口
    void setIpPort(const QString &ip, ushort port);

    //设置主账号密码
    void setMasterPwd(const QString &master, const QString &pwd);

    //开始任务
    bool startThread();

    //停止任务
    void stopThread();

protected:
    virtual void run();

signals:
    //发送token
    void signalSendToken(const QString &token);

    //处理结果
    void signalDealResult(const QString &result);

    //线程结束信号
    void signalThreadFinished();

private:
    //function
    //发送任务数据
    bool sendTaskData();

    //生成json任务
    string createTaskInfo();

    //生成uuid
    QString createUuid();

    //发送数据给服务器
    bool talkWithServer(const QString &ip, ushort port, const string &send, QString &recv, QString &error);

    //获取任务结果
    bool getTaskResult(QString &result);

    //variable
    QString m_strIp;
    ushort  m_iPort;
    QString m_strMaster;
    QString m_strPwd;
    QString m_strTaskData;
    QString m_strToken;
    bool    m_bFinished;
};

#endif // CSUBMITTASKTHREAD_H
