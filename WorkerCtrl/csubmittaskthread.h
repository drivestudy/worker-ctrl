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

    //����ip�˿�
    void setIpPort(const QString &ip, ushort port);

    //�������˺�����
    void setMasterPwd(const QString &master, const QString &pwd);

    //��ʼ����
    bool startThread();

    //ֹͣ����
    void stopThread();

protected:
    virtual void run();

signals:
    //����token
    void signalSendToken(const QString &token);

    //������
    void signalDealResult(const QString &result);

    //�߳̽����ź�
    void signalThreadFinished();

private:
    //function
    //������������
    bool sendTaskData();

    //����json����
    string createTaskInfo();

    //����uuid
    QString createUuid();

    //�������ݸ�������
    bool talkWithServer(const QString &ip, ushort port, const string &send, QString &recv, QString &error);

    //��ȡ������
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
