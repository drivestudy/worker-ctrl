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
    //�ͷ�tabҳ
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

    //�ͷ�����ͼ��
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

//��ʼ��ui
void WorkerCtrlWidget::initSetupUi()
{
    //���ô�������
    setMinimumSize(800, 500);
    setWindowTitle(tr("%1 %2").arg(APP_NAME).arg(APP_VERSION));
    setWindowIcon(QIcon(":/images/logo.ico"));

    //��ʼ��tabҳ
    ui->tabWidget->clear();
    m_pTabServerWidget = new TabServerWidget(this);
    m_pTabLogWidget = new TabLogWidget(this);
    m_pTabRankListWidget = new TabRankListWidget(this);
    m_pTabSyncUserWidget = new TabSyncUserWidget(this);
    m_pTabSettingWidget = new TabSettingWidget(this);
    ui->tabWidget->addTab(m_pTabServerWidget, tr("������"));
    ui->tabWidget->addTab(m_pTabLogWidget, tr("��־����"));
    ui->tabWidget->addTab(m_pTabRankListWidget, tr("���а�"));
    ui->tabWidget->addTab(m_pTabSyncUserWidget, tr("ͬ���û�"));
    ui->tabWidget->addTab(m_pTabSettingWidget, tr("ϵͳ����"));

    //���Ӳ�ͬtabҳ���źŲ�
    connect(m_pTabServerWidget, SIGNAL(signalServerChanged()), m_pTabSettingWidget, SLOT(slotServerChanged()));
    connect(m_pTabServerWidget, SIGNAL(signalServerChanged()), m_pTabSyncUserWidget, SLOT(slotServerChanged()));
    connect(m_pTabServerWidget, SIGNAL(signalChangeState(bool,QString,QString)), this, SLOT(slotChangeState(bool,QString,QString)));
}

//��ʼ��layout
void WorkerCtrlWidget::initLayout()
{
    //���ô��ڲ���,�б���
    QGridLayout *pMainLayout = new QGridLayout(this);
    pMainLayout->addWidget(ui->tabWidget, 0, 0);
    setLayout(pMainLayout);
}

//��ʼ������
void WorkerCtrlWidget::initVariable()
{
    //tabҳ
    m_pTabServerWidget = NULL;
    m_pTabLogWidget = NULL;
    m_pTabSettingWidget = NULL;
    m_pTabRankListWidget = NULL;
    m_pTabSyncUserWidget = NULL;

    //ϵͳ����ͼ��
    m_pActionQuit = NULL;
    m_pSystrayMenu = NULL;
    m_pSysTrayIcon = NULL;

    //��ʼ�����ݲֿ�
    CDataWare::instance();
}

//��ʼ��ϵͳͼ��
void WorkerCtrlWidget::initSystrayIcon()
{
    m_pActionQuit = new QAction(tr("�˳�"), this);
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
        m_pSysTrayIcon->showMessage(tr("��ʾ"), tr("���������ں�̨����"), QSystemTrayIcon::Information);
    }
    event->ignore();
}

/****overwrite*****/

/****slots*****/
//ϵͳ����ͼ��
void WorkerCtrlWidget::slotSysTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (QSystemTrayIcon::Trigger == reason) {
        this->activateWindow();
        this->showNormal();
    }
}

//�˳��˵�
void WorkerCtrlWidget::slotActionQuit()
{
    showNormal();
    activateWindow();
    if (QMessageBox::Ok == QMessageBox::warning(this, tr("����"), tr("ȷ��Ҫ�˳�����"), QMessageBox::Ok | QMessageBox::Cancel)) {
        hide();
        qApp->quit();
    }
}

//�л�tabҳ���ź�
void WorkerCtrlWidget::slotSwitchTabWidget(int page)
{
    ui->tabWidget->setCurrentIndex(page);
}

//������ҳ�洫�ݹ����� ����̵߳�״̬�л��ź�
void WorkerCtrlWidget::slotChangeState(bool online, const QString &ip, const QString &version)
{
    m_pSysTrayIcon->showMessage(tr("webhelper״̬"),
                                tr("������: %1\n�汾��: %2\n״̬:  %3")
                                .arg(ip).arg(version).arg(online ? "����" : "����"),
                                QSystemTrayIcon::Information);
}

/****slots*****/
