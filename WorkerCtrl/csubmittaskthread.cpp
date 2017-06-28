#include "csubmittaskthread.h"
#include <cjson.h>
#include <caes.h>
#include <cmd5.h>

CSubmitTaskThread::CSubmitTaskThread(QObject *parent) :
    QThread(parent)
{
    m_iPort = 0;
    m_bFinished = false;
}

CSubmitTaskThread::CSubmitTaskThread(const QString &ip, ushort port,
                                     const QString &master, const QString &pwd,
                                     QObject *parent) :
    QThread(parent)
{
    m_strIp = ip;
    m_iPort = port;
    m_strMaster = master;
    m_strPwd = pwd;
    m_bFinished = false;
}

CSubmitTaskThread::~CSubmitTaskThread()
{
}

//����ip�˿�
void CSubmitTaskThread::setIpPort(const QString &ip, ushort port)
{
    m_strIp = ip;
    m_iPort = port;
}

//�������˺�����
void CSubmitTaskThread::setMasterPwd(const QString &master, const QString &pwd)
{
    m_strMaster = master;
    m_strPwd = pwd;
}

//��ʼ����
bool CSubmitTaskThread::startThread()
{
    if (m_strIp.isEmpty() || m_strMaster.isEmpty()
            || m_strPwd.isEmpty() || 0 == m_iPort) {
        return false;
    }
    m_bFinished = true;
    start();
    return true;
}

//ֹͣ����
void CSubmitTaskThread::stopThread()
{
    m_bFinished = false;
}

void CSubmitTaskThread::run()
{
    //��������
    if (!sendTaskData()) {
        emit signalThreadFinished();
        return;
    }

    //ѭ����ȡ���
    QString strResult;
    while (m_bFinished) {
        if (getTaskResult(strResult)) {
            if ("failed" == strResult) {
                strResult = "������ʧ��";
                break;
            } else if ("success" == strResult) {
                strResult = "������ɹ�";
                break;
            } else if ("process" == strResult) {
                strResult = "�������ڴ����У����Ժ󡣡���";
            } else if ("unknown" == strResult) {
                strResult = "δ֪����";
                break;
            } else if ("notexist" == strResult) {
                strResult = "�����������ڸ�����Ľ��";
                break;
            }
        }
        emit signalDealResult(strResult);
        sleep(3);
    }
    emit signalDealResult(strResult);
    emit signalThreadFinished();
}

//������������
bool CSubmitTaskThread::sendTaskData()
{
    string strTaskInfo;
    QString strRecv, strError;

    //������������
    strTaskInfo = createTaskInfo();
    emit signalSendToken(m_strToken);

    //��������
    if (!talkWithServer(m_strIp, m_iPort, strTaskInfo, strRecv, strError)) {
        emit signalDealResult(strError);
        return false;
    } else {
        Json::Value jval;
        if (!CJson::ParseStringToJsonValue(strRecv.toStdString(), jval)) {
            emit signalDealResult("���������ص����ݽ���ʧ��");
            return false;
        }
        int status = CJson::JsonValueToInt(jval["status"]);
        if (1 == status) {
            emit signalDealResult("��������ɹ�����������ʼִ������");
        } else {
            emit signalDealResult("��������ɹ���������У��ʧ�ܣ�");
        }
    }

    return true;
}

//����json����
string CSubmitTaskThread::createTaskInfo()
{
    //��������
    Json::Value jInfo, jMainAccountData, jTask;

    //mainAccountData
    jMainAccountData["uname"] = m_strMaster.toStdString();
    jMainAccountData["pwd"] = m_strPwd.toStdString();

    //task
    jTask["__UNAME__"] = "";
    jTask["__PWD__"] = "";
    jTask["type"] = 2;//���˺����Ϳ϶��Ǵ���
    jTask["kind"] = 13;//13Ϊͬ���û���
    jTask["parentName"] = m_strMaster.toStdString();
    jTask["status"] = 0;
    jTask["sortNum"] = 0;
    jTask["callback"] = "";
    jTask["unactive"] = 0;//0��ʾ����������webhelper��ȡ������

    //info
    m_strToken = createUuid();
    jInfo["token"] = m_strToken.toStdString();
    jInfo["mainAccountData"] = jMainAccountData;
    jInfo["task"] = jTask;

    string strJson = CJson::JsonWriteToString(jInfo);
    string strEncrypt;

    //����
    CAes::EncryptToHexDefault(strJson, strEncrypt);
    strEncrypt += "-" + CMd5::GetMd5(strJson) + "-#FI#";
    return strEncrypt;
}

//����uuid
QString CSubmitTaskThread::createUuid()
{
    QStringList strList = QUuid::createUuid().toString().split('-');
    QString strUuid;
    for (int i = 0; i < strList.size(); i++) {
        strUuid.append(strList.at(i));
    }
    strUuid.remove('{');
    strUuid.remove('}');
    return "ctrl" + strUuid;
}

//�������ݸ�������
bool CSubmitTaskThread::talkWithServer(const QString &ip, ushort port, const string &send, QString &recv, QString &error)
{
    QTcpSocket tcpSocket;

    recv.clear();
    error.clear();

    tcpSocket.connectToHost(ip, port);
    if (!tcpSocket.waitForConnected(5000)) {
        error = tcpSocket.errorString();
        return false;
    }

    tcpSocket.write(send.c_str());
    if (!tcpSocket.waitForBytesWritten(5000)) {
        error = tcpSocket.errorString();
        return false;
    }

    if (!tcpSocket.waitForReadyRead(5000)){
        error = tcpSocket.errorString();
        return false;
    }

    recv = tcpSocket.readAll();

    return true;
}

//��ȡ������
bool CSubmitTaskThread::getTaskResult(QString &result)
{
    QString strRecv;
    QString strGetResult = tr("{\"getResult\":\"%1\"}-#FI#").arg(m_strToken);

    result.clear();

    //���������ȡ���
    if (talkWithServer(m_strIp, m_iPort, strGetResult.toLatin1().data(), strRecv, result)) {
        Json::Value jval;
        if (!CJson::ParseStringToJsonValue(strRecv.toStdString(), jval)) {
            result = "���������ص����ݽ���ʧ��";
            return false;
        } else {
            result = CJson::JsonValueToStringEx(jval["status"]).c_str();
            return true;
        }
    }

    return false;
}

