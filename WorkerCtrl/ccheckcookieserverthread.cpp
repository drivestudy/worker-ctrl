#include "ccheckcookieserverthread.h"

CCheckCookieServerThread::CCheckCookieServerThread(QObject *parent, ECookieOpType nEck) :
    QThread(parent)
{
    m_nUrlFlag = 0;
    m_eEck = nEck;
}

//添加自动登录器服务器列表
void CCheckCookieServerThread::addCookieServer(int index, const QString &ip, ushort port)
{
    m_ckSrvList.append(CookieServer(index, ip, port));
}

//设置网址标志
void CCheckCookieServerThread::setUrlFlag(int nFlag)
{
    m_nUrlFlag = nFlag;
}

void CCheckCookieServerThread::run()
{
    switch (m_eEck)
    {
    case ECK_Heart:
        {
            int size = m_ckSrvList.size();
            QTcpSocket tcpSocket;
            for (int i = 0; i < size; i++)
            {
                const CookieServer& rCk = m_ckSrvList.at(i);
                tcpSocket.connectToHost(rCk.ip, rCk.port);
                if (!tcpSocket.waitForConnected(4000)) {
                    emit signalCkDealResult(rCk.index, size, rCk.ip, false, "在线", "离线");
                    continue;
                }
                tcpSocket.write("API#HEART#*#");
                tcpSocket.waitForBytesWritten();
                tcpSocket.waitForReadyRead();
                emit signalCkDealResult(rCk.index, size, rCk.ip,
                                       tcpSocket.readAll().contains("OK"),
                                       "在线", "离线");
                tcpSocket.close();
            }
            m_ckSrvList.clear();
        }
        break;

    case ECK_SetUrl:
        {
            int size = m_ckSrvList.size();
            QTcpSocket tcpSocket;
            QString strUrlCmd = QString("API#SET_URL_LIST#%1*#").arg(m_nUrlFlag);
            for (int i = 0; i < size; i++)
            {
                const CookieServer& rCk = m_ckSrvList.at(i);
                tcpSocket.connectToHost(rCk.ip, rCk.port);
                if (!tcpSocket.waitForConnected(4000)) {
                    emit signalCkDealResult(rCk.index, size, rCk.ip, false, "成功", "失败");
                    continue;
                }
                tcpSocket.write(strUrlCmd.toLatin1());
                tcpSocket.waitForBytesWritten();
                tcpSocket.waitForReadyRead();
                emit signalCkDealResult(rCk.index, size, rCk.ip,
                                       tcpSocket.readAll().contains("RET#OK"),
                                       "成功", "失败");
                tcpSocket.close();
            }
            m_ckSrvList.clear();
        }
        break;

    default:
        break;
    }
}
