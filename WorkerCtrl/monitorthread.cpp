#include "monitorthread.h"

MonitorThread::MonitorThread(QObject *parent) :
    QThread(parent)
{
    initVariable();
}

MonitorThread::MonitorThread(const QString &ip, const QString &pwd,
                             ushort srvPort, ushort ctrlPort, int period)
{
    initVariable();
    setServerInfo(ip, pwd, srvPort, ctrlPort, period);
}

MonitorThread::~MonitorThread()
{
}

//���÷�������Ϣ
void MonitorThread::setServerInfo(const QString &ip, const QString &pwd,
                                  ushort srvPort, ushort ctrlPort, int period)
{
    m_srvInfo.server.ip = ip;
    m_srvInfo.server.ctrlPwd = pwd;
    m_srvInfo.server.srvPort = srvPort;
    m_srvInfo.server.ctrlPort = ctrlPort;
    m_srvInfo.monitorPeriod = period;
}

//�޸ķ�������Ϣ
void MonitorThread::modifyServerInfo(const QString &pwd, ushort srvPort, ushort ctrlPort, int period)
{
    m_srvInfo.server.ctrlPwd = pwd;
    m_srvInfo.server.srvPort = srvPort;
    m_srvInfo.server.ctrlPort = ctrlPort;
    m_srvInfo.monitorPeriod = period;
}

//�����˳���־
void MonitorThread::setExitFlag()
{
    m_bRun = false;
}

//�Ƿ���
bool MonitorThread::isMonitor()
{
    return isRunning();
}

//��ʼ���
void MonitorThread::startMonitor()
{
    if (!isMonitor()) {
        m_bRun = true;
        start();
    }
}

//ֹͣ���
void MonitorThread::stopMonitor()
{
    if (isMonitor()) {
        setExitFlag();
        while (1) {
            if (m_finishMutex.tryLock()) {
                m_finishMutex.unlock();
                break;
            }
            qApp->processEvents();
            msleep(50);
        }
    }
}

void MonitorThread::run()
{
    m_finishMutex.lock();

    //��ʼ�ź�
    emit signalStartMonitor(m_srvInfo.server.ip);

    //�״ο�ʼ���
    monitorServer();

    QElapsedTimer timer;
    timer.start();
    while (m_bRun) {
        if (timer.elapsed() >= m_srvInfo.monitorPeriod * 1000) {
            monitorServer();
            timer.restart();
        } else {
            sleep(1);
        }
    }

    //״̬��Ϣ���
    clearServerState();

    //�����ź�
    emit signalStopMonitor(m_srvInfo.server.ip);

    m_finishMutex.unlock();
}

//��ʼ������
void MonitorThread::initVariable()
{
    m_bRun = true;
    m_bOnline = false;

    //��ʼ����������Ϣ
    m_srvInfo.server.ip.clear();
    m_srvInfo.server.srvPort = 0;
    m_srvInfo.server.ctrlPort = 0;
    m_srvInfo.server.ctrlPwd.clear();

    m_srvInfo.monitorPeriod = 0;

    clearServerState();
}

//���
void MonitorThread::monitorServer()
{
    QString strRecv;
    string strCmd = GenCmdRequest(CMD_GETRTDATA, m_srvInfo.server.ctrlPwd);

    if (!talkWithServer(strCmd, strRecv)) {
        emit signalUpdateServerInfo(false, m_srvInfo);
        changeState(false);
        return;
    }

    //����
    QStringList strList = strRecv.split('-');
    if (strList.size() != 4) {
        emit signalUpdateServerInfo(false, m_srvInfo);
        changeState(false);
        return;
    }

    //����
    int iAesLevel = strList.at(1).toInt();
    string strDecrypt;
    CAes::DecryptFromHexLevel(iAesLevel, strList.at(2).toStdString(), strDecrypt);

    //����json
    Json::Value jval, jinfo;
    if (!CJson::ParseStringToJsonValue(strDecrypt, jval)) {
        emit signalUpdateServerInfo(false, m_srvInfo);
        changeState(false);
        return;
    }

    if (CJson::JsonValueToInt(jval["result"]) == 0) {
        m_srvInfo.monitorState = tr("�������");
        emit signalUpdateServerInfo(true, m_srvInfo);
        changeState(true);
        return;
    }

    jinfo = jval["info"];
    m_srvInfo.monitorState = tr("�����");
    m_srvInfo.rtInfo.taskCount = CJson::JsonValueToInt(jinfo["taskcount"]);
    m_srvInfo.rtInfo.viewCount = CJson::JsonValueToInt(jinfo["viewcount"]);
    m_srvInfo.rtInfo.opTaskCount = CJson::JsonValueToInt(jinfo["optaskcount"]);
    m_srvInfo.rtInfo.threadPoolSize = CJson::JsonValueToInt(jinfo["threadpoolsize"]);
    m_srvInfo.rtInfo.threadPoolLoad = CJson::JsonValueToInt(jinfo["threadpoolload"]);
    m_srvInfo.rtInfo.workThreadCount = CJson::JsonValueToInt(jinfo["workthreadcount"]);
    m_srvInfo.rtInfo.sendDataSize = CJson::JsonValueToInt(jinfo["sendsize"]);
    m_srvInfo.rtInfo.recvDataSize = CJson::JsonValueToInt(jinfo["recvsize"]);
    m_srvInfo.rtInfo.runTime = CJson::JsonValueToInt(jinfo["runtime"]);
    m_srvInfo.rtInfo.unGetResultNum = CJson::JsonValueToInt(jinfo["ungetresultnum"]);
    m_srvInfo.rtInfo.resultCacheCount = CJson::JsonValueToInt(jinfo["resultcachecount"]);
    m_srvInfo.rtInfo.version = QString("webhelper-") + CJson::JsonValueToStringEx(jinfo["version"]).c_str();

    emit signalUpdateServerInfo(true, m_srvInfo);
    changeState(true);
}

//��������Ի�
bool MonitorThread::talkWithServer(const string &cmd, QString &recv)
{
    QTcpSocket tcpSocket;

    tcpSocket.connectToHost(m_srvInfo.server.ip, m_srvInfo.server.srvPort);
    if (!tcpSocket.waitForConnected(4000)) {
        return false;
    }

    tcpSocket.write(cmd.c_str(), cmd.size());
    if (!tcpSocket.waitForBytesWritten(4000)) {
        return false;
    }

    recv.clear();
    while (1) {
        if (tcpSocket.state() != QAbstractSocket::ConnectedState) {
//            qDebug() << "invalid socket";
            break;
        }
        if (tcpSocket.waitForReadyRead(4000)) {
            recv.append(tcpSocket.readAll());
            //�����յ�"#FI#",���ʾ����
            if (recv.endsWith(CMDEND)) {
                break;
            }
        }
    }
    tcpSocket.close();
    return true;
}

//������������
string MonitorThread::GenCmdRequest(CMDTYPE cmdtype, const QString &pwd, int aesLevel)
{
    Json::Value jobj;
    jobj["cmd"] = cmdtype;
    jobj["pwd"] = pwd.toStdString();

    string strEncryptJson;
    if (0 != aesLevel) {
        CAes::EncryptToHexLevel(aesLevel, CJson::JsonWriteToString(jobj), strEncryptJson);
    } else {
        strEncryptJson = CJson::JsonWriteToString(jobj);
    }

    char buf[256];
    sprintf(buf, "%s-%d-", CMDREQ, aesLevel);
    string strReq = buf + strEncryptJson + CMDEND;
    return strReq;
}

//���״̬��Ϣ
void MonitorThread::clearServerState()
{
    m_srvInfo.runState.clear();
    m_srvInfo.monitorState = tr("δ���");

    m_srvInfo.rtInfo.taskCount = 0;
    m_srvInfo.rtInfo.viewCount = 0;
    m_srvInfo.rtInfo.opTaskCount = 0;
    m_srvInfo.rtInfo.threadPoolSize = 0;
    m_srvInfo.rtInfo.workThreadCount = 0;
    m_srvInfo.rtInfo.threadPoolLoad = 0;
    m_srvInfo.rtInfo.unGetResultNum = 0;
    m_srvInfo.rtInfo.resultCacheCount = 0;
    m_srvInfo.rtInfo.sendDataSize = 0;
    m_srvInfo.rtInfo.recvDataSize = 0;
    m_srvInfo.rtInfo.runTime = 0;
    m_srvInfo.rtInfo.version.clear();
}

//��¼״̬���л�:���� ����
void MonitorThread::changeState(bool online)
{
    if (m_bOnline != online) {
        m_bOnline = online;
        emit signalChangeState(online, m_srvInfo.server.ip, m_srvInfo.rtInfo.version);
    }
}

