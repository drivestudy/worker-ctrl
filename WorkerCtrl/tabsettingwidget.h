#ifndef TABSETTINGWIDGET_H
#define TABSETTINGWIDGET_H

#include <QWidget>
#include <common.h>
#include <ccheckcookieserverthread.h>
#include <caes.h>

namespace Ui {
class TabSettingWidget;
}

class CSqlParam
{
public:
    QString strIp;
    QString strUser;
    QString strPwd;
    QString strDbname;
    ushort  iPort;
};

class TabSettingWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit TabSettingWidget(QWidget *parent = 0);
    ~TabSettingWidget();

private slots:
    //����������
    void slotServerChanged();
    //����߳̽���
    void slotThreadFinished();
    //������
    void slotCkDealResult(int index, int total, const QString &ip, bool ok, const QString& strOk, const QString& strNo);

    //��������
    void slotBtnLocalSet();
    //Զ������
    void slotBtnRemoteSet();
    //�޸�����
    void slotBtnModifySet();
    //��������
    void slotBtnDownloadSet();
    //Ĭ��
    void slotBtnDefault();
    //����
    void slotBtnReset();
    //���Ӳ���
    void slotBtnTestConnect();
    //���
    void slotBtnAdd();
    //ɾ��
    void slotBtnDelete();
    //���
    void slotBtnClear();
    //��֤
    void slotBtnConfirm();
    //������ַ
    void slotBtnSetUrl();
    //ȫѡ
    void slotBtnSelectAll();
    //��ѡ
    void slotBtnUnselect();
    
private:
    //function
    //��ʼ������
    void initVariable();

    //��ʼ��ui
    void initSetupUi();

    //��ʼ��layout
    void initLayout();

    //�����Ϸ��ж�
    bool isValidData();

    //��table��Ѱ��ip,����row
    int findIpFromTable(const QString &ip);

    //���ý��ÿؼ�
    void enableGroupBox(bool enabled);

    //���ý��ÿؼ�:��֤cookie������
    void enableWidgetForConfirm(bool enabled);

    //���cookie������
    void appendCookieServer(const QString &ip, const QString &port, Qt::CheckState chk);

    //����cookie��������ѡ�еĸ���
    int countCheckedCkSrv();

    //�Ƿ��������ݿ����
    bool isSetSqlParam();

    //������������
    string GenCmdRequest(CMDTYPE cmdtype, const QString &pwd, const QStringList &ckSrvList, int aesLevel=LEVEL_256);
    string GenCmdRequest(CMDTYPE cmdtype, const QString &pwd, int aesLevel=LEVEL_256);

    //��������Ի�
    bool talkWithServer(const QString &ip, ushort srvPort, const string &cmd, QString &recv);

    //���ò�����webhelper
    void setParamToServer(Server *pSrv, const QStringList &ckSrvList, QString &errInfo);

    //��webhelper��ȡ����
    bool getParamFromServer(Server *pSrv, CSqlParam &sqlParam, QStringList &ckSrvList, QString &errInfo);

    //��ȡxml�ļ�
    void readXml();

    //д��xml�ļ�
    void writeXml();

    //variable
    Ui::TabSettingWidget *ui;
    CCheckCookieServerThread *m_pChkCkSrvThread;
};

#endif // TABSETTINGWIDGET_H
