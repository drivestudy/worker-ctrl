#include "cupdatethread.h"
#include <cjson.h>
#include <cdataware.h>

CUpdateThread::CUpdateThread(QObject *parent) :
    QThread(parent)
{
}

CUpdateThread::CUpdateThread(const ServerList &srvList, const QString &filePath, QObject *parent) :
    QThread(parent)
{
    m_srvList = srvList;
    m_strFilePath = filePath;
}

//���÷������б�
void CUpdateThread::setServerList(const ServerList &srvList)
{
    m_srvList = srvList;
}

//���ø����ļ�·��
void CUpdateThread::setUpdateFilePath(const QString &filePath)
{
    m_strFilePath = filePath;
}

//��ʼ����
bool CUpdateThread::startUpdate()
{
    if (!QFile::exists(m_strFilePath)) {
        return false;
    }
    start();
    return true;
}

void CUpdateThread::run()
{
    QString strError;
    Server *pSrv = NULL;
    CDataWare *pDw = CDataWare::instance();
    MonitorThread *pThr = NULL;

    sleep(2);
    for (int i = 0; i < m_srvList.size(); i++) {
        pSrv = m_srvList.at(i);

        pThr = pDw->findServer(pSrv->ip);
        if (!pThr) {
            continue;
        }

        //1.����webhelper�ܾ���������,�������ʧ��,˵��webhelperû������,������������
        emit signalUpdateProgress(pSrv->ip, tr("ֹͣ��������"));
        monitorServer(pSrv->ip, pSrv->ctrlPwd, pSrv->srvPort, CMD_REFUSETASK, strError);

        //ѭ���жϹ����߳����Ƿ�Ϊ0
        while (pThr->m_srvInfo.rtInfo.workThreadCount != 0) {
            sleep(5);
        }

        //2.����webupdate,��ʼ���и���ǰ��׼��
        //0 CMD_CLENUP  ��װǰ��������
        //1 CMD_INSTALL ���밲װ
        emit signalUpdateProgress(pSrv->ip, tr("��ʼ����"));
        if (!monitorServer(pSrv->ip, pSrv->ctrlPwd, pSrv->ctrlPort, (CMDTYPE)0, strError)) {
            emit signalUpdateProgress(pSrv->ip, tr("��ʼ��: ") + strError);
            continue;
        }

        //3.�ϴ������ļ�
        emit signalUpdateProgress(pSrv->ip, tr("�ϴ��ļ�"));
        if (!uploadFile(pSrv)) {
            emit signalUpdateProgress(pSrv->ip, tr("�ϴ��ļ�ʧ��"));
            continue;
        }

        //4.����webupdate,��ʼ��װ�°汾
        emit signalUpdateProgress(pSrv->ip, tr("��ʼ��װ�°汾"));
        if (!monitorServer(pSrv->ip, pSrv->ctrlPwd, pSrv->ctrlPort, (CMDTYPE)1, strError)) {
            emit signalUpdateProgress(pSrv->ip, tr("��ʼ��װ�°汾: ") + strError);
            continue;
        }

        emit signalUpdateProgress(pSrv->ip, tr("�������"));
    }

    //�߳̽���
    emit signalUpdateFinished();
}

//���
bool CUpdateThread::monitorServer(const QString &ip, const QString &pwd, ushort port, CMDTYPE cmdtype, QString &error)
{
    QString strRecv;
    string strCmd = GenCmdRequest(cmdtype, pwd);

    if (!talkWithServer(ip, port, strCmd, strRecv)) {
        error = tr("���¹���������");
        return false;
    }

    //����
    QStringList strList = strRecv.split('-');
    if (strList.size() != 4) {
        error = tr("��������Ӧ�����ݽ�������");
        return false;
    }

    //����
    int iAesLevel = strList.at(1).toInt();
    string strDecrypt;
    CAes::DecryptFromHexLevel(iAesLevel, strList.at(2).toStdString(), strDecrypt);

    //����json
    Json::Value jval;
    if (!CJson::ParseStringToJsonValue(strDecrypt, jval)) {
        error = tr("��������Ӧ��json���ݽ�������");
        return false;
    }

    if (CJson::JsonValueToInt(jval["result"]) == 0) {
        error = tr("�����������");
        return false;
    }

    return true;
}

//bool CUpdateThread::monitorServer(const Server *pSrv, CMDTYPE cmdtype, QString &error)
//{
//    QString strRecv;
//    string strCmd = GenCmdRequest(cmdtype, pSrv->ctrlPwd);

//    if (!talkWithServer(pSrv, strCmd, strRecv)) {
//        error = tr("���¹���������");
//        return false;
//    }

//    //����
//    QStringList strList = strRecv.split('-');
//    if (strList.size() != 4) {
//        error = tr("��������Ӧ�����ݽ�������");
//        return false;
//    }

//    //����
//    int iAesLevel = strList.at(1).toInt();
//    string strDecrypt;
//    CAes::DecryptFromHexLevel(iAesLevel, strList.at(2).toStdString(), strDecrypt);

//    //����json
//    Json::Value jval;
//    if (!CJson::ParseStringToJsonValue(strDecrypt, jval)) {
//        error = tr("��������Ӧ��json���ݽ�������");
//        return false;
//    }

//    if (CJson::JsonValueToInt(jval["result"]) == 0) {
//        error = tr("�����������");
//        return false;
//    }

//    return true;
//}

//��������Ի�
bool CUpdateThread::talkWithServer(const QString &ip, ushort port, const string &cmd, QString &recv)
{
    QTcpSocket tcpSocket;

    tcpSocket.connectToHost(ip, port);
    if (!tcpSocket.waitForConnected(5000)) {
        return false;
    }

    tcpSocket.write(cmd.c_str(), cmd.size());
    if (!tcpSocket.waitForBytesWritten(30000)) {
        return false;
    }

    if (!tcpSocket.waitForReadyRead(300000)) {
        return false;
    }
    recv = tcpSocket.readAll();
    tcpSocket.close();
    return true;
}

//bool CUpdateThread::talkWithServer(const Server *pSrv, const string &cmd, QString &recv)
//{
//    QTcpSocket tcpSocket;

//    tcpSocket.connectToHost(pSrv->ip, pSrv->srvPort);
//    if (!tcpSocket.waitForConnected(5000)) {
//        return false;
//    }

//    tcpSocket.write(cmd.c_str(), cmd.size());
//    if (!tcpSocket.waitForBytesWritten(30000)) {
//        return false;
//    }

//    if (!tcpSocket.waitForReadyRead(300000)) {
//        return false;
//    }
//    recv = tcpSocket.readAll();
//    tcpSocket.close();
//    return true;
//}

//������������
string CUpdateThread::GenCmdRequest(CMDTYPE cmdtype, const QString &pwd, int aesLevel)
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

//�ϴ��ļ�
bool CUpdateThread::uploadFile(const Server *pSrv)
{
    QFile file(m_strFilePath);

    if (!file.exists()) {
        return false;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QDir dir(m_strFilePath);
    QByteArray strContext = file.readAll();
    file.close();

    string strFileNameHex = CAes::ToHex(dir.dirName().toStdString());
    string strContextHex = CAes::ToHex(string(strContext.data(), strContext.size()));
    string strSend = "file-" + strFileNameHex + "-" + strContextHex + "-#FI#";

    QTcpSocket tcpSocket;
    tcpSocket.connectToHost(pSrv->ip, pSrv->ctrlPort);
    if (!tcpSocket.waitForConnected(5000)) {
        return false;
    }
    tcpSocket.write(strSend.c_str());
    if (!tcpSocket.waitForBytesWritten(60000)) {
        return false;
    }
    if (!tcpSocket.waitForDisconnected()) {
        return false;
    }
    tcpSocket.close();

    return true;
}
