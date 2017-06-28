#include "workerctrlwidget.h"
#include "ui_workerctrlwidget.h"

#include <tabserverwidget.h>
#include <tablogwidget.h>
#include <tabranklistwidget.h>
#include <tabsettingwidget.h>
#include <tabsyncuserwidget.h>
#include <cdataware.h>
#include <json/json.h>
#include <cpublic.h>

WorkerCtrlWidget::WorkerCtrlWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WorkerCtrlWidget)
{
    ui->setupUi(this);

    initVariable();
    initLayout();
    initSetupUi();
    initSystrayIcon();
}

WorkerCtrlWidget::~WorkerCtrlWidget()
{
    //释放tab页
    if (m_pTabServerWidget) {
        delete m_pTabServerWidget;
    }
    if (m_pTabLogWidget) {
        delete m_pTabLogWidget;
    }
    if (m_pTabRankListWidget) {
        delete m_pTabRankListWidget;
    }
    if (m_pTabSyncUserWidget) {
        delete m_pTabSyncUserWidget;
    }
    if (m_pTabSettingWidget) {
        delete m_pTabSettingWidget;
    }

    //释放托盘图标
    if (m_pActionQuit) {
        delete m_pActionQuit;
    }
    if (m_pSystrayMenu) {
        delete m_pSystrayMenu;
    }
    if (m_pSysTrayIcon) {
        delete m_pSysTrayIcon;
    }

    delete ui;
}

//初始化ui
void WorkerCtrlWidget::initSetupUi()
{
    //设置窗口属性
    setMinimumSize(800, 500);
    setWindowTitle(tr("%1 %2").arg(APP_NAME).arg(APP_VERSION));
    setWindowIcon(QIcon(":/images/logo.ico"));

    //初始化tab页
    ui->tabWidget->clear();
    m_pTabServerWidget = new TabServerWidget(this);
    m_pTabLogWidget = new TabLogWidget(this);
    m_pTabRankListWidget = new TabRankListWidget(this);
    m_pTabSyncUserWidget = new TabSyncUserWidget(this);
    m_pTabSettingWidget = new TabSettingWidget(this);
    ui->tabWidget->addTab(m_pTabServerWidget, tr("服务器"));
    ui->tabWidget->addTab(m_pTabLogWidget, tr("日志管理"));
    ui->tabWidget->addTab(m_pTabRankListWidget, tr("排行榜"));
    ui->tabWidget->addTab(m_pTabSyncUserWidget, tr("同步用户"));
    ui->tabWidget->addTab(m_pTabSettingWidget, tr("系统设置"));

    //连接不同tab页的信号槽
    connect(m_pTabServerWidget, SIGNAL(signalServerChanged()), m_pTabSettingWidget, SLOT(slotServerChanged()));
    connect(m_pTabServerWidget, SIGNAL(signalServerChanged()), m_pTabSyncUserWidget, SLOT(slotServerChanged()));
    connect(m_pTabServerWidget, SIGNAL(signalChangeState(bool,QString,QString)), this, SLOT(slotChangeState(bool,QString,QString)));
}

//初始化layout
void WorkerCtrlWidget::initLayout()
{
    //设置窗口布局,行比例
    QGridLayout *pMainLayout = new QGridLayout(this);
    pMainLayout->addWidget(ui->tabWidget, 0, 0);
    setLayout(pMainLayout);
}

//初始化变量
void WorkerCtrlWidget::initVariable()
{
    //tab页
    m_pTabServerWidget = NULL;
    m_pTabLogWidget = NULL;
    m_pTabSettingWidget = NULL;
    m_pTabRankListWidget = NULL;
    m_pTabSyncUserWidget = NULL;

    //系统托盘图标
    m_pActionQuit = NULL;
    m_pSystrayMenu = NULL;
    m_pSysTrayIcon = NULL;

    //初始化数据仓库
    CDataWare::instance();
}

//初始化系统图标
void WorkerCtrlWidget::initSystrayIcon()
{
    m_pActionQuit = new QAction(tr("退出"), this);
    connect(m_pActionQuit, SIGNAL(triggered()), this, SLOT(slotActionQuit()));

    m_pSystrayMenu = new QMenu(this);
    m_pSystrayMenu->addAction(m_pActionQuit);

    m_pSysTrayIcon = new QSystemTrayIcon(QIcon(":/images/logo.ico"), this);
    m_pSysTrayIcon->setContextMenu(m_pSystrayMenu);
    connect(m_pSysTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(slotSysTrayIconActivated(QSystemTrayIcon::ActivationReason)));
    m_pSysTrayIcon->setToolTip(QString("%1 %2").arg(APP_NAME).arg(APP_VERSION));
    m_pSysTrayIcon->show();
}

/****overwrite*****/
void WorkerCtrlWidget::closeEvent(QCloseEvent *event)
{
    hide();
    if (m_pSysTrayIcon) {
        m_pSysTrayIcon->showMessage(tr("提示"), tr("程序隐藏在后台运行"), QSystemTrayIcon::Information);
    }
    event->ignore();
}

/****overwrite*****/

/****slots*****/
//系统托盘图标
void WorkerCtrlWidget::slotSysTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (QSystemTrayIcon::Trigger == reason) {
        this->activateWindow();
        this->showNormal();
    }
}

//退出菜单
void WorkerCtrlWidget::slotActionQuit()
{
    showNormal();
    activateWindow();
    if (QMessageBox::Ok == QMessageBox::warning(this, tr("警告"), tr("确定要退出程序？"), QMessageBox::Ok | QMessageBox::Cancel)) {
        hide();
        qApp->quit();
    }
}

//切换tab页的信号
void WorkerCtrlWidget::slotSwitchTabWidget(int page)
{
    ui->tabWidget->setCurrentIndex(page);
}

//服务器页面传递过来的 监控线程的状态切换信号
void WorkerCtrlWidget::slotChangeState(bool online, const QString &ip, const QString &version)
{
    m_pSysTrayIcon->showMessage(tr("webhelper状态"),
                                tr("服务器: %1\n版本号: %2\n状态:  %3")
                                .arg(ip).arg(version).arg(online ? "在线" : "离线"),
                                QSystemTrayIcon::Information);
}

/****slots*****/
