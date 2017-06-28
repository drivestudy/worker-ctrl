#ifndef CTCPSOCKET_H
#define CTCPSOCKET_H

#include <QtNetwork>

class CTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit CTcpSocket(QObject *parent = 0);
    bool waitForConnectedEx(int secs = 10);
    bool waitForReadyReadEx(int secs = 10);
    bool waitForBytesWrittenEx(int secs = 10);

private:
    static const int m_iPeriodMsecs = 50;
};

#endif // CTCPSOCKET_H
