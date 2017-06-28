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

//��ť�źŲ�
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
    if (QMessageBox::Ok != QMessageBox::warning(this, tr("����"), tr("ȷ��Ҫ�ύ����"), QMessageBox::Ok | QMessageBox::Cancel)) {
        return;
    }
    //�����һ����������Ϣ
    ui->lineToken->clear();
    ui->lineTaskResult->clear();
    ui->lineUseTime->clear();

    //�ж��Ƿ�������Ϣ
    if (ui->comboBoxServer->currentIndex() == -1) {
        QMessageBox::warning(this, tr("����"), tr("�����һ�������õķ�����(webhelper)��"));
        ui->comboBoxServer->setFocus();
        return;
    }

    if (ui->lineMaster->text().isEmpty()) {
        QMessageBox::warning(this, tr("����"), tr("���������˺ţ�"));
        ui->lineMaster->setFocus();
        return;
    }
    if (ui->linePassword->text().isEmpty()) {
        QMessageBox::warning(this, tr("����"), tr("���������룡"));
        ui->linePassword->setFocus();
        return;
    }

    //��ʼ����
    Server *pSrv = CDataWare::instance()->getServer(ui->comboBoxServer->currentText());
    if (!pSrv) {
        return;
    }

    m_pSubmitTaskThread = new CSubmitTaskThread(pSrv->ip, pSrv->srvPort, ui->lineMaster->text(), ui->linePassword->text(), this);
    connect(m_pSubmitTaskThread, SIGNAL(signalDealResult(QString)), this, SLOT(slotDealResult(QString)), Qt::DirectConnection);
    connect(m_pSubmitTaskThread, SIGNAL(signalSendToken(QString)), this, SLOT(slotSendToken(QString)), Qt::DirectConnection);
    //�����ź�Ҫ��ӵ��յ��źţ������ͷ��̶߳���Ƚϰ�ȫ
    connect(m_pSubmitTaskThread, SIGNAL(signalThreadFinished()), this, SLOT(slotThreadFinished()));
    if (!m_pSubmitTaskThread->startThread()) {
        QMessageBox::warning(this, tr("����"), tr("�޷������̣߳���������ز�����"));
        m_pSubmitTaskThread->deleteLater();
        m_pSubmitTaskThread = NULL;
        return;
    } else {
        startTimer();
        //�������
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

//����������
void TabSyncUserWidget::slotServerChanged()
{
    QStringList strList = CDataWare::instance()->getServerIpList();
    ui->comboBoxServer->clear();
    ui->comboBoxServer->addItems(strList);
    ui->comboBoxServer->clearFocus();
}

//ʹ��ʱ�䶨ʱ��
void TabSyncUserWidget::slotTimeout()
{
    uint iNow = QDateTime::currentDateTime().toTime_t();
    ui->lineUseTime->setText(CPublic::secToFriendlyFormat(iNow - m_iTaskStartTime));
}

//����token
void TabSyncUserWidget::slotSendToken(const QString &token)
{
    ui->lineToken->setText(token);
}

//������
void TabSyncUserWidget::slotDealResult(const QString &result)
{
    ui->lineTaskResult->setText(result);
}

//�߳̽����ź�
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

//�б���
void TabSyncUserWidget::slotItemDoubleClicked(QTableWidgetItem *item)
{
    if (item) {
        qApp->clipboard()->setText(item->text());
    }
}

//��ʼ��ui
void TabSyncUserWidget::initSetupUi()
{
    //����
    slotServerChanged();
    slotBtnReset();
    ui->btnStop->setEnabled(false);

    //�źŲ�
    connect(ui->btnReset, SIGNAL(clicked()), this, SLOT(slotBtnReset()));
    connect(ui->btnSubmit, SIGNAL(clicked()), this, SLOT(slotBtnSubmit()));
    connect(ui->btnStop, SIGNAL(clicked()), this, SLOT(slotBtnStop()));

    //��ʱ��
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
}

//��ʼ��layout
void TabSyncUserWidget::initLayout()
{
    //������Ϣ
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

    //���ڲ���
    QGridLayout *pLayoutD = new QGridLayout(this);
    pLayoutD->addWidget(ui->grpBoxTaskInfo, 0, 0);
    pLayoutD->setRowStretch(0, 1);
    pLayoutD->setRowStretch(1, 4);
    setLayout(pLayoutD);
}

//��ʼ������
void TabSyncUserWidget::initVariable()
{
    m_iServerPort = 0;
    m_iTaskStartTime = 0;
    m_pSubmitTaskThread = NULL;
}

//������ʱ��ѭ����ȡ���
void TabSyncUserWidget::startTimer()
{
    m_iTaskStartTime = QDateTime::currentDateTime().toTime_t();
    m_timer.start(1000);
}

//�رն�ʱ��
void TabSyncUserWidget::stopTimer()
{
    m_timer.stop();
    m_iTaskStartTime = 0;
}


