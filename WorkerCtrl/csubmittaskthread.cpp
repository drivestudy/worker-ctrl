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

//设置ip端口
void CSubmitTaskThread::setIpPort(const QString &ip, ushort port)
{
    m_strIp = ip;
    m_iPort = port;
}

//设置主账号密码
void CSubmitTaskThread::setMasterPwd(const QString &master, const QString &pwd)
{
    m_strMaster = master;
    m_strPwd = pwd;
}

//开始任务
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

//停止任务
void CSubmitTaskThread::stopThread()
{
    m_bFinished = false;
}

void CSubmitTaskThread::run()
{
    //发送任务
    if (!sendTaskData()) {
        emit signalThreadFinished();
        return;
    }

    //循环获取结果
    QString strResult;
    while (m_bFinished) {
        if (getTaskResult(strResult)) {
            if ("failed" == strResult) {
                strResult = "任务处理失败";
                break;
            } else if ("success" == strResult) {
                strResult = "任务处理成功";
                break;
            } else if ("process" == strResult) {
                strResult = "任务正在处理中，请稍后。。。";
            } else if ("unknown" == strResult) {
                strResult = "未知数据";
                break;
            } else if ("notexist" == strResult) {
                strResult = "服务器不存在该任务的结果";
                break;
            }
        }
        emit signalDealResult(strResult);
        sleep(3);
    }
    emit signalDealResult(strResult);
    emit signalThreadFinished();
}

//发送任务数据
bool CSubmitTaskThread::sendTaskData()
{
    string strTaskInfo;
    QString strRecv, strError;

    //生成任务数据
    strTaskInfo = createTaskInfo();
    emit signalSendToken(m_strToken);

    //发送任务
    if (!talkWithServer(m_strIp, m_iPort, strTaskInfo, strRecv, strError)) {
        emit signalDealResult(strError);
        return false;
    } else {
        Json::Value jval;
        if (!CJson::ParseStringToJsonValue(strRecv.toStdString(), jval)) {
            emit signalDealResult("服务器返回的数据解析失败");
            return false;
        }
        int status = CJson::JsonValueToInt(jval["status"]);
        if (1 == status) {
            emit signalDealResult("发送任务成功，服务器开始执行任务。");
        } else {
            emit signalDealResult("发送任务成功，服务器校验失败！");
        }
    }

    return true;
}

//生成json任务
string CSubmitTaskThread::createTaskInfo()
{
    //生成任务
    Json::Value jInfo, jMainAccountData, jTask;

    //mainAccountData
    jMainAccountData["uname"] = m_strMaster.toStdString();
    jMainAccountData["pwd"] = m_strPwd.toStdString();

    //task
    jTask["__UNAME__"] = "";
    jTask["__PWD__"] = "";
    jTask["type"] = 2;//主账号类型肯定是代理
    jTask["kind"] = 13;//13为同步用户名
    jTask["parentName"] = m_strMaster.toStdString();
    jTask["status"] = 0;
    jTask["sortNum"] = 0;
    jTask["callback"] = "";
    jTask["unactive"] = 0;//0表示程序主动向webhelper获取处理结果

    //info
    m_strToken = createUuid();
    jInfo["token"] = m_strToken.toStdString();
    jInfo["mainAccountData"] = jMainAccountData;
    jInfo["task"] = jTask;

    string strJson = CJson::JsonWriteToString(jInfo);
    string strEncrypt;

    //加密
    CAes::EncryptToHexDefault(strJson, strEncrypt);
    strEncrypt += "-" + CMd5::GetMd5(strJson) + "-#FI#";
    return strEncrypt;
}

//生成uuid
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

//发送数据给服务器
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

//获取任务结果
bool CSubmitTaskThread::getTaskResult(QString &result)
{
    QString strRecv;
    QString strGetResult = tr("{\"getResult\":\"%1\"}-#FI#").arg(m_strToken);

    result.clear();

    //向服务器获取结果
    if (talkWithServer(m_strIp, m_iPort, strGetResult.toLatin1().data(), strRecv, result)) {
        Json::Value jval;
        if (!CJson::ParseStringToJsonValue(strRecv.toStdString(), jval)) {
            result = "服务器返回的数据解析失败";
            return false;
        } else {
            result = CJson::JsonValueToStringEx(jval["status"]).c_str();
            return true;
        }
    }

    return false;
}

