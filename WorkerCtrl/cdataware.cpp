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

//������־���ݿ����
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

//��ȡ��־���ݿ����
void CDataWare::getLogSqlParam(QString &ip, QString &user, QString &pwd, QString &dbname, ushort &port)
{
    ip = m_strSqlIp;
    user = m_strSqlUser;
    pwd = m_strSqlPwd;
    dbname = m_strSqlDbName;
    port = m_iSqlPort;
}

//��ȡ��־���ݿ����
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

//��������ת��
QString CDataWare::optypeString(uint type)
{
    //��1��ʼ
    static QString arr[] = {
        "��", "�û�ע��", "�༭�û���Ϣ", "�л��û�״̬", "�л�Ͷע״̬", "�������",
        "�������", "��ȡ�û�����", "��ȡ���ʺŵ���", "(��)�˺Ž���", "ע�ἤ��",
        "��֤�û���", "��֤����", "ͬ���û�����", "ͬ������", "��ȡ�û�/Ͷע״̬"
    };
    if (type < sizeof(arr)/sizeof(arr[0])) {
        return arr[type];
    } else {
        return "";
    }
}

//��������ɾ�Ĳ�
MonitorThread *CDataWare::appendServer(const QString &ip, ushort srvPort, ushort ctrlPort, int monitorPeriod, const QString &ctrlPwd)
{
    if (!hasServer(ip)) {
        //����һ���߳�
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
        //�޸ķ�������Ϣ
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

//��ȡ��������Ϣ
Server *CDataWare::getServer(const QString &ip)
{
    MonitorThread *pThr = findServer(ip);
    if (pThr) {
        return &(pThr->m_srvInfo.server);
    }
    return NULL;
}

//���ط�������Ϣ����
ServerList CDataWare::getServerList()
{
    ServerList srvList;
    for (int i = 0; i < m_monitorThreadList.size(); i++) {
        srvList.append(&(m_monitorThreadList.at(i)->m_srvInfo.server));
    }
    return srvList;
}

//��ȡ�߳��б�
MonitorThreadList CDataWare::getMonitorThreadList()
{
    return m_monitorThreadList;
}

//��ȡ�������б�
QStringList CDataWare::getServerIpList()
{
    QStringList strIpList;
    for (int i = 0; i < m_monitorThreadList.size(); i++) {
        strIpList.append(m_monitorThreadList.at(i)->m_srvInfo.server.ip);
    }
    return strIpList;
}
