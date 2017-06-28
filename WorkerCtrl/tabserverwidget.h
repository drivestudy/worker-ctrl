#ifndef TABSERVERWIDGET_H
#define TABSERVERWIDGET_H

#include <QWidget>
#include <common.h>

namespace Ui {
class TabServerWidget;
}

class CUpdateThread;
class MonitorThread;
typedef QList<MonitorThread *> MonitorThreadList;

class TabServerWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit TabServerWidget(QWidget *parent = 0);
    ~TabServerWidget();

protected:
    void resizeEvent(QResizeEvent *event);

signals:
    //服务器增减
    void signalServerChanged();

    //监控线程的状态切换信号
    void signalChangeState(bool online, const QString &ip, const QString &version);
    
private slots:
    //tabServer
    void slotCurrentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void slotCellDoubleClicked(int row, int column);

    //右键菜单
    void slotCustomContextMenuRequested(const QPoint &pos);

    //菜单事件:全选
    void slotActionSelectAll();

    //菜单事件:反选
    void slotActionUnSelect();

    //添加
    void slotBtnSrvAdd();

    //修改
    void slotBtnSrvModify();

    //删除
    void slotBtnSrvDelete();

    //清空
    void slotBtnSrvClearList();

    //开始监控
    void slotBtnSrvStartMonitor();

    //停止监控
    void slotBtnSrvStopMonitor();

    //浏览文件
    void slotBtnScanFile();

    //升级
    void slotBtnUpdate();

    //重置
    void slotBtnReset();

    //监控线程
    //开始监控信号
    void slotStartMonitor(const QString &ip);

    //停止监控信号
    void slotStopMonitor(const QString &ip);

    //更新服务器信息
    void slotUpdateServerInfo(bool online, const ServerInfo &srvInfo);

    //版本升级线程
    //升级进度
    void slotUpdateProgress(const QString &ip, const QString &info);

    //线程结束
    void slotUpdateFinished();

private:
    //func
    //初始化变量
    void initVariable();

    //初始化ui
    void initSetupUi();

    //初始化layout
    void initLayout();

    //服务器列表编辑
    bool appendServer(const QString &ip, ushort srvPort, ushort ctrlPort,
                      int monitorPeriod, const QString &ctrlPwd);
    bool removeServer(const QString &ip);
    bool modifyServer(const QString &ip, ushort srvPort, ushort ctrlPort,
                      int monitorPeriod, const QString &ctrlPwd);
    int findItem(const QString &ip);

    //交互合法判断
    bool isValidData();

    //服务器写入xml
    void writeXml();

    //从xml读取服务器信息
    void readXml();

    //设置列表列宽度
    void setTableColumnWidth();

    //停止所有监控线程
    void stopAllMonitor();

    //获取列表选中的服务器ip
    QStringList selectedIpList();

    //控制程序升级的按钮
    void enabledUpdateWidget(bool enabled);

    //variable
    Ui::TabServerWidget *ui;

    //当前显示实时状态的服务器ip
    QString m_strCurRtSrvIP;

    //菜单
    QMenu m_srvMenu;
    QAction *m_pActionSelectAll;
    QAction *m_pActionUnSelect;
    CUpdateThread *m_pUpdateThread;
};

#endif // TABSERVERWIDGET_H
