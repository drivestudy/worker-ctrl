#ifndef CUPDATETHREAD_H
#define CUPDATETHREAD_H

#include <common.h>
#include <caes.h>

class CUpdateThread : public QThread
{
    Q_OBJECT
public:
    explicit CUpdateThread(QObject *parent = 0);
    explicit CUpdateThread(const ServerList &srvList, const QString &filePath, QObject *parent = 0);

    //���÷������б�
    void setServerList(const ServerList &srvList);

    //���ø����ļ�·��
    void setUpdateFilePath(const QString &filePath);

    //��ʼ����
    bool startUpdate();

protected:
    virtual void run();
    
signals:
    //��������
    void signalUpdateProgress(const QString &ip, const QString &info);

    //���½���
    void signalUpdateFinished();

private:
    //function
    //���
    bool monitorServer(const QString &ip, const QString &pwd, ushort port, CMDTYPE cmdtype, QString &error);
//    bool monitorServer(const Server *pSrv, CMDTYPE cmdtype, QString &error);

    //��������Ի�
    bool talkWithServer(const QString &ip, ushort port, const string &cmd, QString &recv);
//    bool talkWithServer(const Server *pSrv, const string &cmd, QString &recv);

    //������������
    string GenCmdRequest(CMDTYPE cmdtype, const QString &pwd, int aesLevel=LEVEL_256);

    //�ϴ��ļ�
    bool uploadFile(const Server *pSrv);

    //variable
    QString m_strFilePath;
    ServerList m_srvList;
};

#endif // CUPDATETHREAD_H
