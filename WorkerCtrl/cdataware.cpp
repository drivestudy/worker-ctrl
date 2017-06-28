#include "cdataware.h"

QMutex CDataWare::m_singleMutex;
CDataWare *CDataWare::m_pInstance = NULL;
CDataWare::CGarbo CDataWare::m_garbo;

CDataWare::CDataWare()
{
    m_iSqlPort = 0;
}

CDataWare::~CDataWare()
{
    for (int i = 0; i < m_monitorThreadList.size(); i++) {
        delete m_monitorThreadList.at(i);
    }
    m_monitorThreadList.clear();
}

CDataWare *CDataWare::instance()
{
    m_singleMutex.lock();
    if (!m_pInstance) {
        m_pInstance = new CDataWare();
    }
    m_singleMutex.unlock();
    return m_pInstance;
}

void CDataWare::destroy()
{
    m_singleMutex.lock();
    if (m_pInstance) {
        delete m_pInstance;
        m_pInstance = NULL;
    }
    m_singleMutex.unlock();
}

//设置日志数据库参数
void CDataWare::setLogSqlParam(const QString &ip, const QString &user, const QString &pwd, const QString &dbname, ushort port)
{
    m_strSqlIp = ip;
    m_strSqlUser = user;
    m_strSqlPwd = pwd;
    m_strSqlDbName = dbname;
    m_iSqlPort = port;

    QSqlDatabase sqlDb;
    getLogSqlDb(sqlDb);
    sqlDb.setHostName(m_strSqlIp);
    sqlDb.setDatabaseName(m_strSqlDbName);
    sqlDb.setUserName(m_strSqlUser);
    sqlDb.setPort(m_iSqlPort);
    sqlDb.setPassword(m_strSqlPwd);
}

//获取日志数据库参数
void CDataWare::getLogSqlParam(QString &ip, QString &user, QString &pwd, QString &dbname, ushort &port)
{
    ip = m_strSqlIp;
    user = m_strSqlUser;
    pwd = m_strSqlPwd;
    dbname = m_strSqlDbName;
    port = m_iSqlPort;
}

//获取日志数据库对象
bool CDataWare::getLogSqlDb(QSqlDatabase &sqlDb)
{
    if (0 == m_iSqlPort || m_strSqlIp.isEmpty() || m_strSqlUser.isEmpty()
            || m_strSqlPwd.isEmpty() || m_strSqlDbName.isEmpty()) {
        return false;
    }

    static QString strLogDbName = "_mysql_log_database_";
    if (QSqlDatabase::contains(strLogDbName)) {
        sqlDb = QSqlDatabase::database(strLogDbName);
    } else {
        sqlDb = QSqlDatabase::addDatabase("QMYSQL", strLogDbName);
        sqlDb.setHostName(m_strSqlIp);
        sqlDb.setDatabaseName(m_strSqlDbName);
        sqlDb.setUserName(m_strSqlUser);
        sqlDb.setPort(m_iSqlPort);
        sqlDb.setPassword(m_strSqlPwd);
    }
    return true;
}

//操作类型转换
QString CDataWare::optypeString(uint type)
{
    //从1开始
    static QString arr[] = {
        "无", "用户注册", "编辑用户信息", "切换用户状态", "切换投注状态", "存入点数",
        "提出点数", "获取用户点数", "获取主帐号点数", "(新)账号解锁", "注册激活",
        "验证用户名", "验证别名", "同步用户数据", "同步报表", "获取用户/投注状态"
    };
    if (type < sizeof(arr)/sizeof(arr[0])) {
        return arr[type];
    } else {
        return "";
    }
}

//服务器增删改查
MonitorThread *CDataWare::appendServer(const QString &ip, ushort srvPort, ushort ctrlPort, int monitorPeriod, const QString &ctrlPwd)
{
    if (!hasServer(ip)) {
        //创建一个线程
        MonitorThread *pThr = new MonitorThread(ip, ctrlPwd, srvPort, ctrlPort, monitorPeriod);
        m_monitorThreadList.append(pThr);
        return pThr;
    }
    return NULL;
}

bool CDataWare::modifyServer(const QString &ip, ushort srvPort, ushort ctrlPort, int monitorPeriod, const QString &ctrlPwd)
{
    MonitorThread *pThr = findServer(ip);
    if (pThr) {
        //修改服务器信息
        pThr->modifyServerInfo(ctrlPwd, srvPort, ctrlPort, monitorPeriod);
        return true;
    }
    return false;
}

bool CDataWare::removeServer(const QString &ip)
{
    MonitorThread *pThr = findServer(ip);
    if (pThr) {
        m_monitorThreadList.removeOne(pThr);
        return true;
    }
    return false;
}

MonitorThread *CDataWare::findServer(const QString &ip)
{
    for (int i = 0; i < m_monitorThreadList.size(); i++) {
        if (ip == m_monitorThreadList.at(i)->m_srvInfo.server.ip) {
            return m_monitorThreadList.at(i);
        }
    }
    return NULL;
}

bool CDataWare::hasServer(const QString &ip)
{
    return (NULL != findServer(ip));
}

//获取服务器信息
Server *CDataWare::getServer(const QString &ip)
{
    MonitorThread *pThr = findServer(ip);
    if (pThr) {
        return &(pThr->m_srvInfo.server);
    }
    return NULL;
}

//返回服务器信息链表
ServerList CDataWare::getServerList()
{
    ServerList srvList;
    for (int i = 0; i < m_monitorThreadList.size(); i++) {
        srvList.append(&(m_monitorThreadList.at(i)->m_srvInfo.server));
    }
    return srvList;
}

//获取线程列表
MonitorThreadList CDataWare::getMonitorThreadList()
{
    return m_monitorThreadList;
}

//获取服务器列表
QStringList CDataWare::getServerIpList()
{
    QStringList strIpList;
    for (int i = 0; i < m_monitorThreadList.size(); i++) {
        strIpList.append(m_monitorThreadList.at(i)->m_srvInfo.server.ip);
    }
    return strIpList;
}
