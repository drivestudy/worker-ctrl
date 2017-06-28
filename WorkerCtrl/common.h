#ifndef COMMON_H
#define COMMON_H
#include <QtCore>
#include <QtGui>
#include <QtSql>
#include <QtNetwork>
#include <QtXml>

#include <iostream>
#include <string>
using namespace std;

#define APP_NAME    "�п�"
#define APP_VERSION "v2.2"
#define SET_FILE    "config.xml"

#define DEBUG_FUNCTION  qDebug() << __FUNCTION__;

//��������Ϣ�ṹ��
typedef struct {
    QString     ip;
    ushort      srvPort;//����˿ں�
    ushort      ctrlPort;//���ƶ˿ں�
    QString     ctrlPwd;//��������
} Server;

typedef struct {
    //webhleper��Ϣ
    int         taskCount;//��������
    int         viewCount;//��������
    int         opTaskCount;//������������
    int         threadPoolSize;//�̳߳ش�С
    int         workThreadCount;//�����߳���
    int         runTime;//����ʱ��
    int         threadPoolLoad;//�̳߳ظ���
    int         unGetResultNum;//δ��ȡ�����
    int         resultCacheCount;//���������
    uint        sendDataSize;//�������ݵĴ�С
    uint        recvDataSize;//�������ݵĴ�С
    QString     version;//�汾
} RtInfo;

typedef struct {
    Server      server;
    RtInfo      rtInfo;

    QString     runState;//����״̬:���� ����
    int         monitorPeriod;//�������
    QString     monitorState;//���״̬:����� δ���

} ServerInfo;

typedef QList<Server *> ServerList;

/*cmd protocolЭ��*/
typedef enum
{
    CMD_GETRTDATA,  //��ȡʵʱ����,
    CMD_HEARTBEAT,	//��������
    CMD_CLOSE,		//�ر�
    CMD_REFUSETASK, //�ܾ���������
    CMD_SETPARAM,	//���ò���
    CMD_VERSION,	//��ȡ�汾��Ϣ
    CMD_UNGETRESULTNUM, //��ȡδ����ȡ����������
    CMD_RESULTCACHENUM,  //���������
    //����Э��,����������
    CMD_GETPARAM    //��ȡ����
}CMDTYPE;

//�����ʽ
#define CMDREQ	"cmdreq"
#define CMDRES	"cmdres"
#define CMDEND	"-#FI#"
//�������Գ����״̬
#define STATE_START	"start"
#define STATE_STOP	"stop"
//������Ӧ�ķ���ֵ
#define RES_OK	1
#define RES_NO	0
//��������
#define UI_OP_PASSWORD    "haoying123"

/**
Э��:
����:
{
    "cmd" : ����,
    "pwd" : ��������,
    "opt" : {
        //�ɱ�����,json����
    }
}

���������:
{
    "cmd" : ����,
    "pwd" : ��������,
    "opt" : {
        "sql" : {
            "ip" : "192.168.1.1",
            "port" : 3306,
            "user" : "root",
            "pwd"  : "123456",
            "dbname" : "webhelper"
        },
        "cookiesrv" :
        ["192.168.1.1:26128", "192.168.1.1:26128", "192.168.1.1:26128", "192.168.1.1:26128"]
    }
}


��Ӧ:
{
    "info" : ��Ϣ,
    "result" : 1 | 0, //1�ɹ� 0ʧ��
}

ʵʱ��Ϣ�ĸ�ʽ:
{
    "info" : {
        "taskcount" : 0, //��������
        "viewcount" : 0, //��������
        "opTaskCount" : 0, //������������
        "threadpoolsize" : 0,//�̳߳ش�С
        "workthreadcount" : 0,//�����߳���
        "threadpoolload" : 0/1/2,//�̳߳ظ���
        "sendsize" : 1234,//�������ݴ�С
        "recvsize" : 1234,//�������ݴ�С
        "runtime" : 1234,//����ʱ��
        "ungetresultnum" : 12,//δ��ȡ�����
        "resultcachecount" : 11,//���������
        "version" : "webhelper-v1.9"//�汾
    },
    "result" : 1 | 0, //1�ɹ� 0ʧ��
}

ϵͳ�����ĸ�ʽ:
{
    "info" : {
        "sql" : {
            "ip" : "192.168.1.1",
            "port" : 3306,
            "user" : "root",
            "pwd"  : "123456",
            "dbname" : "webhelper"
        },
        "cookiesrv" :
        ["192.168.1.1:26128", "192.168.1.1:26128", "192.168.1.1:26128", "192.168.1.1:26128"]
    },
    "result" : 1 | 0
}

*/


#endif // COMMON_H
