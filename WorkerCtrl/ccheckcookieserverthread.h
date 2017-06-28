#ifndef CCHECKCOOKIESERVERTHREAD_H
#define CCHECKCOOKIESERVERTHREAD_H

#include <QThread>
#include <common.h>

enum ECookieOpType
{
    ECK_Heart, //cookie服务器心跳测试
    ECK_SetUrl, //设置cookie服务器的网址列表

    ECK_NUM
};

//保存cookie服务器信息
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

    //通用：添加自动登录器服务器列表
    void addCookieServer(int index, const QString &ip, ushort port);

    //设置网址标志
    void setUrlFlag(int nFlag);

protected:
    virtual void run();
    
signals:
    //通知处理结果
    void signalCkDealResult(int index, int total, const QString& ip, bool ok, const QString& strOk, const QString& strNo);

private:
    //variable
    CookieServerList m_ckSrvList;//cookie服务器列表
    int m_nUrlFlag;//网址标志，利用int里面的n个位来表示
    ECookieOpType m_eEck;
};

#endif // CCHECKCOOKIESERVERTHREAD_H
