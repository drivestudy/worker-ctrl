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
    //服务器增减
    void slotServerChanged();
    //检测线程结束
    void slotThreadFinished();
    //处理结果
    void slotCkDealResult(int index, int total, const QString &ip, bool ok, const QString& strOk, const QString& strNo);

    //本地设置
    void slotBtnLocalSet();
    //远程设置
    void slotBtnRemoteSet();
    //修改设置
    void slotBtnModifySet();
    //下载设置
    void slotBtnDownloadSet();
    //默认
    void slotBtnDefault();
    //重置
    void slotBtnReset();
    //连接测试
    void slotBtnTestConnect();
    //添加
    void slotBtnAdd();
    //删除
    void slotBtnDelete();
    //清空
    void slotBtnClear();
    //验证
    void slotBtnConfirm();
    //设置网址
    void slotBtnSetUrl();
    //全选
    void slotBtnSelectAll();
    //反选
    void slotBtnUnselect();
    
private:
    //function
    //初始化变量
    void initVariable();

    //初始化ui
    void initSetupUi();

    //初始化layout
    void initLayout();

    //交互合法判断
    bool isValidData();

    //从table中寻找ip,返回row
    int findIpFromTable(const QString &ip);

    //启用禁用控件
    void enableGroupBox(bool enabled);

    //启用禁用控件:验证cookie服务器
    void enableWidgetForConfirm(bool enabled);

    //添加cookie服务器
    void appendCookieServer(const QString &ip, const QString &port, Qt::CheckState chk);

    //计算cookie服务器被选中的个数
    int countCheckedCkSrv();

    //是否设置数据库参数
    bool isSetSqlParam();

    //生成请求命令
    string GenCmdRequest(CMDTYPE cmdtype, const QString &pwd, const QStringList &ckSrvList, int aesLevel=LEVEL_256);
    string GenCmdRequest(CMDTYPE cmdtype, const QString &pwd, int aesLevel=LEVEL_256);

    //与服务器对话
    bool talkWithServer(const QString &ip, ushort srvPort, const string &cmd, QString &recv);

    //设置参数到webhelper
    void setParamToServer(Server *pSrv, const QStringList &ckSrvList, QString &errInfo);

    //从webhelper获取参数
    bool getParamFromServer(Server *pSrv, CSqlParam &sqlParam, QStringList &ckSrvList, QString &errInfo);

    //读取xml文件
    void readXml();

    //写入xml文件
    void writeXml();

    //variable
    Ui::TabSettingWidget *ui;
    CCheckCookieServerThread *m_pChkCkSrvThread;
};

#endif // TABSETTINGWIDGET_H
