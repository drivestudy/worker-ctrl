#include "tabsyncuserwidget.h"
#include "ui_tabsyncuserwidget.h"
#include <nofocusdelegate.h>
#include <cjson.h>
#include <caes.h>
#include <cmd5.h>
#include <ctcpsocket.h>
#include <cdataware.h>
#include <cpublic.h>
#include <csubmittaskthread.h>

TabSyncUserWidget::TabSyncUserWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabSyncUserWidget)
{
    ui->setupUi(this);

    initVariable();
    initLayout();
    initSetupUi();
}

TabSyncUserWidget::~TabSyncUserWidget()
{
    delete ui;
}

//按钮信号槽
void TabSyncUserWidget::slotBtnReset()
{
    ui->comboBoxServer->setCurrentIndex(0);
    ui->lineMaster->clear();
    ui->linePassword->clear();
    ui->lineToken->clear();
    ui->lineTaskResult->clear();
    ui->lineUseTime->clear();
}

void TabSyncUserWidget::slotBtnSubmit()
{
    if (QMessageBox::Ok != QMessageBox::warning(this, tr("警告"), tr("确定要提交任务？"), QMessageBox::Ok | QMessageBox::Cancel)) {
        return;
    }
    //清空上一次遗留的信息
    ui->lineToken->clear();
    ui->lineTaskResult->clear();
    ui->lineUseTime->clear();

    //判断是否输入信息
    if (ui->comboBoxServer->currentIndex() == -1) {
        QMessageBox::warning(this, tr("警告"), tr("请添加一个可以用的服务器(webhelper)！"));
        ui->comboBoxServer->setFocus();
        return;
    }

    if (ui->lineMaster->text().isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请输入主账号！"));
        ui->lineMaster->setFocus();
        return;
    }
    if (ui->linePassword->text().isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请输入密码！"));
        ui->linePassword->setFocus();
        return;
    }

    //开始处理
    Server *pSrv = CDataWare::instance()->getServer(ui->comboBoxServer->currentText());
    if (!pSrv) {
        return;
    }

    m_pSubmitTaskThread = new CSubmitTaskThread(pSrv->ip, pSrv->srvPort, ui->lineMaster->text(), ui->linePassword->text(), this);
    connect(m_pSubmitTaskThread, SIGNAL(signalDealResult(QString)), this, SLOT(slotDealResult(QString)), Qt::DirectConnection);
    connect(m_pSubmitTaskThread, SIGNAL(signalSendToken(QString)), this, SLOT(slotSendToken(QString)), Qt::DirectConnection);
    //结束信号要间接地收到信号，这样释放线程对象比较安全
    connect(m_pSubmitTaskThread, SIGNAL(signalThreadFinished()), this, SLOT(slotThreadFinished()));
    if (!m_pSubmitTaskThread->startThread()) {
        QMessageBox::warning(this, tr("警告"), tr("无法启动线程，请设置相关参数！"));
        m_pSubmitTaskThread->deleteLater();
        m_pSubmitTaskThread = NULL;
        return;
    } else {
        startTimer();
        //处理界面
        ui->btnSubmit->setEnabled(false);
        ui->btnReset->setEnabled(false);
        ui->btnStop->setEnabled(true);
    }
}

void TabSyncUserWidget::slotBtnStop()
{
    if (m_pSubmitTaskThread) {
        m_pSubmitTaskThread->stopThread();
        ui->btnStop->setEnabled(false);
    }
}

//服务器增减
void TabSyncUserWidget::slotServerChanged()
{
    QStringList strList = CDataWare::instance()->getServerIpList();
    ui->comboBoxServer->clear();
    ui->comboBoxServer->addItems(strList);
    ui->comboBoxServer->clearFocus();
}

//使用时间定时器
void TabSyncUserWidget::slotTimeout()
{
    uint iNow = QDateTime::currentDateTime().toTime_t();
    ui->lineUseTime->setText(CPublic::secToFriendlyFormat(iNow - m_iTaskStartTime));
}

//发送token
void TabSyncUserWidget::slotSendToken(const QString &token)
{
    ui->lineToken->setText(token);
}

//处理结果
void TabSyncUserWidget::slotDealResult(const QString &result)
{
    ui->lineTaskResult->setText(result);
}

//线程结束信号
void TabSyncUserWidget::slotThreadFinished()
{
    ui->btnSubmit->setEnabled(true);
    ui->btnReset->setEnabled(true);
    ui->btnStop->setEnabled(false);
    stopTimer();

    if (m_pSubmitTaskThread) {
        m_pSubmitTaskThread->deleteLater();
        m_pSubmitTaskThread = NULL;
    }
}

//列表复制
void TabSyncUserWidget::slotItemDoubleClicked(QTableWidgetItem *item)
{
    if (item) {
        qApp->clipboard()->setText(item->text());
    }
}

//初始化ui
void TabSyncUserWidget::initSetupUi()
{
    //设置
    slotServerChanged();
    slotBtnReset();
    ui->btnStop->setEnabled(false);

    //信号槽
    connect(ui->btnReset, SIGNAL(clicked()), this, SLOT(slotBtnReset()));
    connect(ui->btnSubmit, SIGNAL(clicked()), this, SLOT(slotBtnSubmit()));
    connect(ui->btnStop, SIGNAL(clicked()), this, SLOT(slotBtnStop()));

    //定时器
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
}

//初始化layout
void TabSyncUserWidget::initLayout()
{
    //任务信息
    QGridLayout *pLayoutA = new QGridLayout(this);
    pLayoutA->addWidget(ui->labelA0,        0, 0);
    pLayoutA->addWidget(ui->labelA1,        1, 0);
    pLayoutA->addWidget(ui->labelA2,        2, 0);
    pLayoutA->addWidget(ui->labelB0,        0, 2);
    pLayoutA->addWidget(ui->labelB1,        1, 2);
    pLayoutA->addWidget(ui->labelB2,        2, 2);
    pLayoutA->addWidget(ui->comboBoxServer, 0, 1);
    pLayoutA->addWidget(ui->lineMaster,     1, 1);
    pLayoutA->addWidget(ui->linePassword,   2, 1);
    pLayoutA->addWidget(ui->lineToken,      0, 3);
    pLayoutA->addWidget(ui->lineTaskResult, 1, 3);
    pLayoutA->addWidget(ui->lineUseTime,    2, 3);
    pLayoutA->addWidget(ui->btnSubmit,      0, 4);
    pLayoutA->addWidget(ui->btnReset,       1, 4);
    pLayoutA->addWidget(ui->btnStop,        2, 4);
    ui->grpBoxTaskInfo->setLayout(pLayoutA);

    //窗口布局
    QGridLayout *pLayoutD = new QGridLayout(this);
    pLayoutD->addWidget(ui->grpBoxTaskInfo, 0, 0);
    pLayoutD->setRowStretch(0, 1);
    pLayoutD->setRowStretch(1, 4);
    setLayout(pLayoutD);
}

//初始化变量
void TabSyncUserWidget::initVariable()
{
    m_iServerPort = 0;
    m_iTaskStartTime = 0;
    m_pSubmitTaskThread = NULL;
}

//开启定时器循环获取结果
void TabSyncUserWidget::startTimer()
{
    m_iTaskStartTime = QDateTime::currentDateTime().toTime_t();
    m_timer.start(1000);
}

//关闭定时器
void TabSyncUserWidget::stopTimer()
{
    m_timer.stop();
    m_iTaskStartTime = 0;
}


