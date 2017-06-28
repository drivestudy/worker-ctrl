#ifndef CCHECKCOOKIESERVERTHREAD_H
#define CCHECKCOOKIESERVERTHREAD_H

#include <QThread>
#include <common.h>

enum ECookieOpType
{
    ECK_Heart, //cookie��������������
    ECK_SetUrl, //����cookie����������ַ�б�

    ECK_NUM
};

//����cookie��������Ϣ
class CookieServer
{
public:
    CookieServer(int _index, const QString &_ip, ushort _port)
        : index(_index), ip(_ip), port(_port) {
    }

    int index;
    QString ip;
    ushort port;
};
typedef QList<CookieServer> CookieServerList;


class CCheckCookieServerThread : public QThread
{
    Q_OBJECT
public:
    explicit CCheckCookieServerThread(QObject *parent, ECookieOpType nEck);

    //ͨ�ã�����Զ���¼���������б�
    void addCookieServer(int index, const QString &ip, ushort port);

    //������ַ��־
    void setUrlFlag(int nFlag);

protected:
    virtual void run();
    
signals:
    //֪ͨ������
    void signalCkDealResult(int index, int total, const QString& ip, bool ok, const QString& strOk, const QString& strNo);

private:
    //variable
    CookieServerList m_ckSrvList;//cookie�������б�
    int m_nUrlFlag;//��ַ��־������int�����n��λ����ʾ
    ECookieOpType m_eEck;
};

#endif // CCHECKCOOKIESERVERTHREAD_H
