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

    //������־���ݿ����
    void setLogSqlParam(const QString &ip, const QString &user, const QString &pwd, const QString &dbname, ushort port);

    //��ȡ��־���ݿ����
    void getLogSqlParam(QString &ip, QString &user, QString &pwd, QString &dbname, ushort &port);

    //��ȡ��־���ݿ����
    bool getLogSqlDb(QSqlDatabase &sqlDb);

    //��������ת��
    QString optypeString(uint type);

    //��������ɾ�Ĳ�
    MonitorThread *appendServer(const QString &ip, ushort srvPort, ushort ctrlPort, int monitorPeriod, const QString &ctrlPwd);
    bool modifyServer(const QString &ip, ushort srvPort, ushort ctrlPort, int monitorPeriod, const QString &ctrlPwd);
    bool removeServer(const QString &ip);
    MonitorThread *findServer(const QString &ip);
    bool hasServer(const QString &ip);

    //��ȡ��������Ϣ
    Server *getServer(const QString &ip);

    //���ط�������Ϣ����
    ServerList getServerList();

    //��ȡ�߳��б�
    MonitorThreadList getMonitorThreadList();

    //��ȡ�������б�
    QStringList getServerIpList();

private:    
    //variable
    MonitorThreadList m_monitorThreadList;//����߳��б�

    //��־���ݿ����
    QString m_strSqlIp;
    QString m_strSqlUser;
    QString m_strSqlPwd;
    QString m_strSqlDbName;
    ushort  m_iSqlPort;

    //������Ҫ�ı���
    static QMutex m_singleMutex;
    static CDataWare *m_pInstance;

    //��������,�Զ�����
    class CGarbo {
    public:
        ~CGarbo() {
            CDataWare::destroy();
        }
    };
    static CGarbo m_garbo;
};

#endif // CDATAWARE_H
