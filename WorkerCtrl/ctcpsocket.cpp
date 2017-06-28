#include "ctcpsocket.h"

CTcpSocket::CTcpSocket(QObject *parent) :
    QTcpSocket(parent)
{
}

bool CTcpSocket::waitForConnectedEx(int secs)
{
    int times = secs*1000/m_iPeriodMsecs;

    for (int i = 0; i < times; i++) {
        if (waitForConnected(m_iPeriodMsecs)) {
            return true;
        }
        qApp->processEvents();
    }
    return false;
}

bool CTcpSocket::waitForReadyReadEx(int secs)
{
    int times = secs*1000/m_iPeriodMsecs;

    for (int i = 0; i < times; i++) {
        if (waitForReadyRead(m_iPeriodMsecs)) {
            return true;
        }
        qApp->processEvents();
    }
    return false;
}

bool CTcpSocket::waitForBytesWrittenEx(int secs)
{
    int times = secs*1000/m_iPeriodMsecs;

    for (int i = 0; i < times; i++) {
        if (waitForBytesWritten(m_iPeriodMsecs)) {
            return true;
        }
        qApp->processEvents();
    }
    return false;
}
