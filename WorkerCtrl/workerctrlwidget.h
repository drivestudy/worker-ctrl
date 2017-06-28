#ifndef WORKERCTRLWIDGET_H
#define WORKERCTRLWIDGET_H

#include <QWidget>
#include <common.h>

//custom
class TabServerWidget;
class TabLogWidget;
class TabSettingWidget;
class TabRankListWidget;
class TabSyncUserWidget;

namespace Ui {
class WorkerCtrlWidget;
}

class WorkerCtrlWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit WorkerCtrlWidget(QWidget *parent = 0);
    ~WorkerCtrlWidget();

    //初始化ui
    void initSetupUi();

    //初始化layout
    void initLayout();

    //初始化变量
    void initVariable();

    //初始化系统图标
    void initSystrayIcon();

protected:
    void closeEvent(QCloseEvent *event);
    
private slots:
    //系统托盘图标
    void slotSysTrayIconActivated(QSystemTrayIcon::ActivationReason reason);

    //退出菜单
    void slotActionQuit();

    //切换tab页的信号
    void slotSwitchTabWidget(int page);

    //服务器页面传递过来的 监控线程的状态切换信号
    void slotChangeState(bool online, const QString &ip, const QString &version);

private:
    //variable
    Ui::WorkerCtrlWidget *ui;
    //tab页
    TabServerWidget *m_pTabServerWidget;
    TabLogWidget *m_pTabLogWidget;
    TabSettingWidget *m_pTabSettingWidget;
    TabRankListWidget *m_pTabRankListWidget;
    TabSyncUserWidget *m_pTabSyncUserWidget;

    //系统托盘图标
    QAction *m_pActionQuit;
    QMenu *m_pSystrayMenu;
    QSystemTrayIcon *m_pSysTrayIcon;
};

#endif // WORKERCTRLWIDGET_H
