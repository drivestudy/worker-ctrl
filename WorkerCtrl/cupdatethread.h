#ifndef CUPDATETHREAD_H
#define CUPDATETHREAD_H

#include <common.h>
#include <caes.h>

class CUpdateThread : public QThread
{
    Q_OBJECT
public:
    explicit CUpdateThread(QObject *parent = 0);
    explicit CUpdateThread(const ServerList &srvList, const QString &filePath, QObject *parent = 0);

    //设置服务器列表
    void setServerList(const ServerList &srvList);

    //设置更新文件路径
    void setUpdateFilePath(const QString &filePath);

    //开始升级
    bool startUpdate();

protected:
    virtual void run();
    
signals:
    //升级进度
    void signalUpdateProgress(const QString &ip, const QString &info);

    //更新结束
    void signalUpdateFinished();

private:
    //function
    //监控
    bool monitorServer(const QString &ip, const QString &pwd, ushort port, CMDTYPE cmdtype, QString &error);
//    bool monitorServer(const Server *pSrv, CMDTYPE cmdtype, QString &error);

    //与服务器对话
    bool talkWithServer(const QString &ip, ushort port, const string &cmd, QString &recv);
//    bool talkWithServer(const Server *pSrv, const string &cmd, QString &recv);

    //生成请求命令
    string GenCmdRequest(CMDTYPE cmdtype, const QString &pwd, int aesLevel=LEVEL_256);

    //上传文件
    bool uploadFile(const Server *pSrv);

    //variable
    QString m_strFilePath;
    ServerList m_srvList;
};

#endif // CUPDATETHREAD_H
