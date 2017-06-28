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

    //���÷�������Ϣ
    void setServerInfo(const QString &ip, const QString &pwd,
                       ushort srvPort, ushort ctrlPort, int period);

    //�޸ķ�������Ϣ
    void modifyServerInfo(const QString &pwd, ushort srvPort, ushort ctrlPort, int period);

    //�����˳���־
    void setExitFlag();

    //�Ƿ���
    bool isMonitor();

    //��ʼ���
    void startMonitor();

    //ֹͣ���
    void stopMonitor();

    //����������Ϣ
    ServerInfo m_srvInfo;
    
signals:
    //��ʼ����ź�
    void signalStartMonitor(const QString &ip);

    //ֹͣ����ź�
    void signalStopMonitor(const QString &ip);

    //���·�������Ϣ
    void signalUpdateServerInfo(bool online, const ServerInfo &srvInfo);

    //״̬�л�
    void signalChangeState(bool online, const QString &ip, const QString &version);

protected:
    virtual void run();

private:
    //func
    //��ʼ������
    void initVariable();

    //���
    void monitorServer();

    //��������Ի�
    bool talkWithServer(const string &cmd, QString &recv);

    //������������
    string GenCmdRequest(CMDTYPE cmdtype, const QString &pwd, int aesLevel=LEVEL_256);

    //���״̬��Ϣ
    void clearServerState();

    //��¼״̬���л�:���� ����
    void changeState(bool online);

    //variable
    bool m_bRun;//���б�־
    bool m_bOnline;//�Ƿ�����
    QMutex m_finishMutex;
};

typedef QList<MonitorThread *> MonitorThreadList;

#endif // MONITORTHREAD_H
