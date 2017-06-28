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

//设置服务器列表
void CUpdateThread::setServerList(const ServerList &srvList)
{
    m_srvList = srvList;
}

//设置更新文件路径
void CUpdateThread::setUpdateFilePath(const QString &filePath)
{
    m_strFilePath = filePath;
}

//开始升级
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

        //1.控制webhelper拒绝接收任务,如果控制失败,说明webhelper没有启动,可以正常升级
        emit signalUpdateProgress(pSrv->ip, tr("停止接收任务"));
        monitorServer(pSrv->ip, pSrv->ctrlPwd, pSrv->srvPort, CMD_REFUSETASK, strError);

        //循环判断工作线程数是否为0
        while (pThr->m_srvInfo.rtInfo.workThreadCount != 0) {
            sleep(5);
        }

        //2.控制webupdate,开始进行更新前的准备
        //0 CMD_CLENUP  安装前的清理工作
        //1 CMD_INSTALL 编译安装
        emit signalUpdateProgress(pSrv->ip, tr("开始清理"));
        if (!monitorServer(pSrv->ip, pSrv->ctrlPwd, pSrv->ctrlPort, (CMDTYPE)0, strError)) {
            emit signalUpdateProgress(pSrv->ip, tr("开始清: ") + strError);
            continue;
        }

        //3.上传更新文件
        emit signalUpdateProgress(pSrv->ip, tr("上传文件"));
        if (!uploadFile(pSrv)) {
            emit signalUpdateProgress(pSrv->ip, tr("上传文件失败"));
            continue;
        }

        //4.控制webupdate,开始安装新版本
        emit signalUpdateProgress(pSrv->ip, tr("开始安装新版本"));
        if (!monitorServer(pSrv->ip, pSrv->ctrlPwd, pSrv->ctrlPort, (CMDTYPE)1, strError)) {
            emit signalUpdateProgress(pSrv->ip, tr("开始安装新版本: ") + strError);
            continue;
        }

        emit signalUpdateProgress(pSrv->ip, tr("升级完成"));
    }

    //线程结束
    emit signalUpdateFinished();
}

//监控
bool CUpdateThread::monitorServer(const QString &ip, const QString &pwd, ushort port, CMDTYPE cmdtype, QString &error)
{
    QString strRecv;
    string strCmd = GenCmdRequest(cmdtype, pwd);

    if (!talkWithServer(ip, port, strCmd, strRecv)) {
        error = tr("更新工具已离线");
        return false;
    }

    //解析
    QStringList strList = strRecv.split('-');
    if (strList.size() != 4) {
        error = tr("服务器响应的数据解析错误");
        return false;
    }

    //解密
    int iAesLevel = strList.at(1).toInt();
    string strDecrypt;
    CAes::DecryptFromHexLevel(iAesLevel, strList.at(2).toStdString(), strDecrypt);

    //解析json
    Json::Value jval;
    if (!CJson::ParseStringToJsonValue(strDecrypt, jval)) {
        error = tr("服务器响应的json数据解析错误");
        return false;
    }

    if (CJson::JsonValueToInt(jval["result"]) == 0) {
        error = tr("控制密码错误");
        return false;
    }

    return true;
}

//bool CUpdateThread::monitorServer(const Server *pSrv, CMDTYPE cmdtype, QString &error)
//{
//    QString strRecv;
//    string strCmd = GenCmdRequest(cmdtype, pSrv->ctrlPwd);

//    if (!talkWithServer(pSrv, strCmd, strRecv)) {
//        error = tr("更新工具已离线");
//        return false;
//    }

//    //解析
//    QStringList strList = strRecv.split('-');
//    if (strList.size() != 4) {
//        error = tr("服务器响应的数据解析错误");
//        return false;
//    }

//    //解密
//    int iAesLevel = strList.at(1).toInt();
//    string strDecrypt;
//    CAes::DecryptFromHexLevel(iAesLevel, strList.at(2).toStdString(), strDecrypt);

//    //解析json
//    Json::Value jval;
//    if (!CJson::ParseStringToJsonValue(strDecrypt, jval)) {
//        error = tr("服务器响应的json数据解析错误");
//        return false;
//    }

//    if (CJson::JsonValueToInt(jval["result"]) == 0) {
//        error = tr("控制密码错误");
//        return false;
//    }

//    return true;
//}

//与服务器对话
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

//生成请求命令
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

//上传文件
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
