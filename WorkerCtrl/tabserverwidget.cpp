#include "tabserverwidget.h"
#include "ui_tabserverwidget.h"
#include <cpublic.h>
#include <monitorthread.h>
#include <cdataware.h>
#include <caes.h>
#include <nofocusdelegate.h>
#include <cupdatethread.h>

TabServerWidget::TabServerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabServerWidget)
{
    ui->setupUi(this);
    initVariable();
    initLayout();
    initSetupUi();

    //读取xml配置文件
    readXml();
}

TabServerWidget::~TabServerWidget()
{
    //停止所有监控线程
    stopAllMonitor();

    //清空服务器列表
    ui->tableServer->clearContents();
    ui->tableServer->setRowCount(0);

    delete ui;
//    DEBUG_FUNCTION
}

void TabServerWidget::resizeEvent(QResizeEvent *event)
{
    setTableColumnWidth();
}

/*****slot******/
//tabServer的信号
void TabServerWidget::slotCurrentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    if (-1 != currentRow) {
        QString ip = ui->tableServer->item(currentRow, 0)->text();
        MonitorThread *pMt = CDataWare::instance()->findServer(ip);
        if (pMt) {
            //设置当前显示的服务器ip
            m_strCurRtSrvIP = ip;

            //设置界面数据
            ui->lineIpAddr->setText(pMt->m_srvInfo.server.ip);
            ui->lineServerPort->setText(QString::number(pMt->m_srvInfo.server.srvPort));
            ui->lineCtrlPort->setText(QString::number(pMt->m_srvInfo.server.ctrlPort));
            ui->spinBoxMonitorPeriod->setValue(pMt->m_srvInfo.monitorPeriod);
            ui->lineCtrlPwd->setText(pMt->m_srvInfo.server.ctrlPwd);
            ui->lineThreadPoolSize->setText(QString::number(pMt->m_srvInfo.rtInfo.threadPoolSize));
            ui->lineWorkThreadCount->setText(QString::number(pMt->m_srvInfo.rtInfo.workThreadCount));
            ui->lineSendSize->setText(CPublic::byteToFriendlyFormat(pMt->m_srvInfo.rtInfo.sendDataSize));
            ui->lineRecvSize->setText(CPublic::byteToFriendlyFormat(pMt->m_srvInfo.rtInfo.recvDataSize));
            ui->lineRunTime->setText(CPublic::secToFriendlyFormat(pMt->m_srvInfo.rtInfo.runTime));
            ui->lineUnGetResultNum->setText(QString::number(pMt->m_srvInfo.rtInfo.unGetResultNum));
            ui->lineResultCacheCount->setText(QString::number(pMt->m_srvInfo.rtInfo.resultCacheCount));

            QString loadArr[] = {tr("负载正常"), tr("负载高"), tr("负载低")};
            ui->lineThreadPoolLoad->setText(loadArr[pMt->m_srvInfo.rtInfo.threadPoolLoad]);
        }
    }
}

void TabServerWidget::slotCellDoubleClicked(int row, int column)
{
    if (-1 != row) {
        QTableWidgetItem *pItem = ui->tableServer->item(row, 0);
        pItem->setCheckState((pItem->checkState() == Qt::Checked) ? Qt::Unchecked : Qt::Checked);
    }
}

//右键菜单
void TabServerWidget::slotCustomContextMenuRequested(const QPoint &pos)
{
    m_srvMenu.exec(QCursor::pos());
}

//菜单事件:全选
void TabServerWidget::slotActionSelectAll()
{
    int row = ui->tableServer->rowCount();
    for (int i = 0; i < row; i++) {
        ui->tableServer->item(i, 0)->setCheckState(Qt::Checked);
    }
}

//菜单事件:反选
void TabServerWidget::slotActionUnSelect()
{
    int row = ui->tableServer->rowCount();
    QTableWidgetItem *pItem = NULL;
    for (int i = 0; i < row; i++) {
        pItem = ui->tableServer->item(i, 0);
        pItem->setCheckState((pItem->checkState() == Qt::Checked) ? Qt::Unchecked : Qt::Checked);
    }
}

//添加
void TabServerWidget::slotBtnSrvAdd()
{
    if (!isValidData()) {
        return;
    }
    if (!appendServer(ui->lineIpAddr->text(), ui->lineServerPort->text().toUShort(),
                      ui->lineCtrlPort->text().toUShort(),
                      ui->spinBoxMonitorPeriod->value(), ui->lineCtrlPwd->text())) {
        QMessageBox::warning(this, tr("警告"), tr("请不要重复添加服务器！"));
        return;
    } else {
        //写入xml
        writeXml();
    }
}

//修改
void TabServerWidget::slotBtnSrvModify()
{
    if (!isValidData()) {
        return;
    }
    if (!modifyServer(ui->lineIpAddr->text(), ui->lineServerPort->text().toUShort(),
                      ui->lineCtrlPort->text().toUShort(),
                      ui->spinBoxMonitorPeriod->value(), ui->lineCtrlPwd->text())) {
        QMessageBox::warning(this, tr("警告"), tr("不存在该服务器，无法修改参数！"));
        return;
    } else {
        //写入xml
        writeXml();
    }
}

//删除
void TabServerWidget::slotBtnSrvDelete()
{
    if (QMessageBox::Ok != QMessageBox::warning(this, tr("警告"), tr("是否要删除已选中的服务器？"), QMessageBox::Ok | QMessageBox::Cancel)) {
        return;
    }

    QStringList strIpList = selectedIpList();
    if (strIpList.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请先选中要删除的服务器，再进行删除操作！"));
        return;
    }
    for (int i = 0; i < strIpList.size(); i++) {
        removeServer(strIpList.at(i));
    }

    //写入xml
    writeXml();
}

//清空
void TabServerWidget::slotBtnSrvClearList()
{
    if (QMessageBox::Ok == QMessageBox::warning(this, tr("警告"), tr("是否清空服务器列表？"), QMessageBox::Ok | QMessageBox::Cancel)) {
        //停止所有监控线程
        stopAllMonitor();
        //清空界面信息
        ui->tableServer->clearContents();
        ui->tableServer->setRowCount(0);
        ui->lineIpAddr->clear();
        ui->lineServerPort->clear();
        ui->lineCtrlPort->clear();
        ui->lineCtrlPwd->clear();
        ui->spinBoxMonitorPeriod->setValue(5);

        ui->lineThreadPoolSize->clear();
        ui->lineWorkThreadCount->clear();
        ui->lineThreadPoolLoad->clear();
        ui->lineUnGetResultNum->clear();
        ui->lineResultCacheCount->clear();
        ui->lineSendSize->clear();
        ui->lineRecvSize->clear();
        ui->lineRunTime->clear();

        ui->lineUpdatingServer->clear();
    }
}

//开始监控
void TabServerWidget::slotBtnSrvStartMonitor()
{
    if (QMessageBox::Ok != QMessageBox::warning(this, tr("警告"), tr("是否监控已选中的服务器？"), QMessageBox::Ok | QMessageBox::Cancel)) {
        return;
    }

    QStringList strIpList = selectedIpList();
    if (strIpList.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请先选中要监控的服务器，再进行监控操作！"));
        return;
    }

    CDataWare *pDw = CDataWare::instance();
    MonitorThread *pThr = NULL;
    for (int i = 0; i < strIpList.size(); i++) {
        pThr = pDw->findServer(strIpList.at(i));
        if (pThr) {
            pThr->startMonitor();
        }
    }
}

//停止监控
void TabServerWidget::slotBtnSrvStopMonitor()
{
    if (QMessageBox::Ok != QMessageBox::warning(this, tr("警告"), tr("是否停止监控已选中的服务器？"), QMessageBox::Ok | QMessageBox::Cancel)) {
        return;
    }

    QStringList strIpList = selectedIpList();
    if (strIpList.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请先选中要监控的服务器，再进行停止监控操作！"));
        return;
    }

    CDataWare *pDw = CDataWare::instance();
    MonitorThread *pThr = NULL;
    for (int i = 0; i < strIpList.size(); i++) {
        pThr = pDw->findServer(strIpList.at(i));
        if (pThr) {
            pThr->stopMonitor();
        }
    }
}

//浏览文件
void TabServerWidget::slotBtnScanFile()
{
    static QString strDirName = QDir::rootPath();
    QString strFileName = QFileDialog::getOpenFileName(this,
                                                       tr("webhelper升级文件"),
                                                       strDirName,
                                                       tr("webhelper (*.tar.bz2 *.bz2);;所有文件 (*.*)"));
    if (!strFileName.isEmpty()) {
        ui->lineUpdateFilePath->setText(QDir::toNativeSeparators(strFileName));
        QDir dir(strFileName);
        strDirName = strFileName.remove(dir.dirName());
    }

    ui->lineUpdateFilePath->setCursorPosition(0);
}

//升级
void TabServerWidget::slotBtnUpdate()
{
    ui->lineUpdateState->clear();
    qApp->processEvents();

    //判断是否有选择服务器
    QStringList strIpList = selectedIpList();
    if (strIpList.empty()) {
        QMessageBox::warning(this, tr("警告"), tr("请选择服务器！"));
        return;
    }

    if (ui->lineUpdateFilePath->text().isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请选择更新文件！"));
        ui->lineUpdateFilePath->setFocus();
        return;
    }

    if (QMessageBox::Ok != QMessageBox::warning(this,
                                                tr("谨慎升级"),
                                                tr("升级过程中，请勿退出程序；\n确定要升级webhelper？"),
                                                QMessageBox::Ok | QMessageBox::Cancel)) {
        return;
    }

    //如果确定要升级的话，开启这几个监控线程
    ui->lineUpdatingServer->setText(strIpList.join(";"));

    //获取服务器信息
    ServerList srvList;
    MonitorThread *pThr = NULL;
    CDataWare *pDw = CDataWare::instance();
    for (int i = 0; i < strIpList.size(); i++) {
        pThr = pDw->findServer(strIpList.at(i));
        if (pThr) {
            pThr->startMonitor();//开启线程
            srvList.append(&(pThr->m_srvInfo.server));//添加服务器信息
        }
    }

    m_pUpdateThread = new CUpdateThread(srvList, ui->lineUpdateFilePath->text(), this);
    connect(m_pUpdateThread, SIGNAL(signalUpdateProgress(QString,QString)),
            this, SLOT(slotUpdateProgress(QString,QString)), Qt::DirectConnection);
    connect(m_pUpdateThread, SIGNAL(signalUpdateFinished()),
            this, SLOT(slotUpdateFinished()));
    if (!m_pUpdateThread->startUpdate()) {
        QMessageBox::warning(this, tr("警告"), tr("开启升级线程失败，请重试！"));
        return;
    }

    enabledUpdateWidget(false);
}

//重置
void TabServerWidget::slotBtnReset()
{
    ui->lineUpdatingServer->clear();
    ui->lineUpdateFilePath->clear();
    ui->lineUpdateState->clear();
}

//监控线程
//开始监控信号
void TabServerWidget::slotStartMonitor(const QString &ip)
{
    int row = findItem(ip);
    ui->tableServer->item(row, 8)->setText(tr("监控中"));
}

//停止监控信号
void TabServerWidget::slotStopMonitor(const QString &ip)
{
    int row = findItem(ip);
    ui->tableServer->item(row, 3)->setText("");
    ui->tableServer->item(row, 4)->setText("");
    ui->tableServer->item(row, 5)->setText("");
    ui->tableServer->item(row, 6)->setText("");
    ui->tableServer->item(row, 8)->setText(tr("未监控"));
    ui->tableServer->item(row, 9)->setText("");
}

//更新服务器信息
void TabServerWidget::slotUpdateServerInfo(bool online, const ServerInfo &srvInfo)
{
    //更新到服务器列表
    int row = findItem(srvInfo.server.ip);
    if (-1 == row) {
        return;
    }

    if (online) {
        ui->tableServer->item(row, 3)->setText(tr("在线"));
        ui->tableServer->item(row, 4)->setText(QString::number(srvInfo.rtInfo.taskCount));
        ui->tableServer->item(row, 5)->setText(QString::number(srvInfo.rtInfo.opTaskCount));
        ui->tableServer->item(row, 6)->setText(QString::number(srvInfo.rtInfo.viewCount));
        ui->tableServer->item(row, 8)->setText(srvInfo.monitorState);
        ui->tableServer->item(row, 9)->setText(srvInfo.rtInfo.version);

        if (ui->lineIpAddr->text() == srvInfo.server.ip) {
            ui->lineThreadPoolSize->setText(QString::number(srvInfo.rtInfo.threadPoolSize));
            ui->lineWorkThreadCount->setText(QString::number(srvInfo.rtInfo.workThreadCount));
            ui->lineSendSize->setText(CPublic::byteToFriendlyFormat(srvInfo.rtInfo.sendDataSize));
            ui->lineRecvSize->setText(CPublic::byteToFriendlyFormat(srvInfo.rtInfo.recvDataSize));
            ui->lineRunTime->setText(CPublic::secToFriendlyFormat(srvInfo.rtInfo.runTime));
            ui->lineUnGetResultNum->setText(QString::number(srvInfo.rtInfo.unGetResultNum));
            ui->lineResultCacheCount->setText(QString::number(srvInfo.rtInfo.resultCacheCount));
        }
    } else {
        ui->tableServer->item(row, 3)->setText(tr("离线"));
    }
}

//版本升级线程
//升级进度
void TabServerWidget::slotUpdateProgress(const QString &ip, const QString &info)
{
    ui->lineUpdatingServer->setText(ip);
    ui->lineUpdateState->setText(info);
    ui->lineUpdateState->setCursorPosition(0);
}

//处理结果
void TabServerWidget::slotUpdateFinished()
{
    enabledUpdateWidget(true);
    if (m_pUpdateThread) {
        m_pUpdateThread->deleteLater();
        m_pUpdateThread = NULL;
    }
}

/*****slot******/

//初始化变量
void TabServerWidget::initVariable()
{
    m_pUpdateThread = NULL;
    m_pActionSelectAll = NULL;
    m_pActionUnSelect = NULL;
}

//初始化ui
void TabServerWidget::initSetupUi()
{
    //菜单
    m_pActionSelectAll = new QAction(tr("全选"), this);
    connect(m_pActionSelectAll, SIGNAL(triggered()), this, SLOT(slotActionSelectAll()));
    m_pActionUnSelect = new QAction(tr("反选"), this);
    connect(m_pActionUnSelect, SIGNAL(triggered()), this, SLOT(slotActionUnSelect()));
    m_srvMenu.addAction(m_pActionSelectAll);
    m_srvMenu.addAction(m_pActionUnSelect);

    //表格属性处理
    ui->tableServer->clearContents();
    ui->tableServer->setRowCount(0);

    //设置宽度
    setTableColumnWidth();
    ui->tableServer->verticalHeader()->setResizeMode(QHeaderView::Fixed);
    ui->tableServer->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableServer->setItemDelegate(new NoFocusDelegate(this));

    connect(ui->tableServer, SIGNAL(currentCellChanged(int,int,int,int)),
            this, SLOT(slotCurrentCellChanged(int,int,int,int)));
    connect(ui->tableServer, SIGNAL(cellDoubleClicked(int,int)),
            this, SLOT(slotCellDoubleClicked(int,int)));
    //自定义菜单
    connect(ui->tableServer, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotCustomContextMenuRequested(QPoint)));

    //编辑操作按钮
    connect(ui->btnSrvAdd, SIGNAL(clicked()), this, SLOT(slotBtnSrvAdd()));
    connect(ui->btnSrvModify, SIGNAL(clicked()), this, SLOT(slotBtnSrvModify()));
    connect(ui->btnSrvDelete, SIGNAL(clicked()), this, SLOT(slotBtnSrvDelete()));
    connect(ui->btnSrvClearList, SIGNAL(clicked()), this, SLOT(slotBtnSrvClearList()));
    connect(ui->btnSrvStartMonitor, SIGNAL(clicked()), this, SLOT(slotBtnSrvStartMonitor()));
    connect(ui->btnSrvStopMonitor, SIGNAL(clicked()), this, SLOT(slotBtnSrvStopMonitor()));

    //程序升级按钮
    connect(ui->btnScanFile, SIGNAL(clicked()), this, SLOT(slotBtnScanFile()));
    connect(ui->btnUpdate, SIGNAL(clicked()), this, SLOT(slotBtnUpdate()));
    connect(ui->btnReset, SIGNAL(clicked()), this, SLOT(slotBtnReset()));

    //编辑框
    connect(ui->lineIpAddr, SIGNAL(returnPressed()), this, SLOT(slotBtnSrvAdd()));

//    for (int i = 0; i < 40; i++) {
//        appendServer(tr("192.168.1.%1").arg(i+1), 26121, 30001, 10, "123456");
//    }
//    appendServer("192.168.111.163", 26121, 30001, 10, "123456");
//    appendServer("192.168.111.165", 26122, 30002, 11, "123456");
}

//初始化layout
void TabServerWidget::initLayout()
{
    //设置服务器编辑组框的布局
    QGridLayout *pLayoutA = new QGridLayout(this);
    pLayoutA->addWidget(ui->labelA0,                0, 0);
    pLayoutA->addWidget(ui->labelA1,                1, 0);
    pLayoutA->addWidget(ui->labelA2,                2, 0);
    pLayoutA->addWidget(ui->labelA3,                3, 0);
    pLayoutA->addWidget(ui->labelA4,                4, 0);
    pLayoutA->addWidget(ui->labelA5,                5, 0);
    pLayoutA->addWidget(ui->lineIpAddr,             0, 1, 1, 3);
    pLayoutA->addWidget(ui->lineServerPort,         1, 1, 1, 3);
    pLayoutA->addWidget(ui->lineCtrlPort,           2, 1, 1, 3);
    pLayoutA->addWidget(ui->spinBoxMonitorPeriod,   3, 1, 1, 3);
    pLayoutA->addWidget(ui->lineCtrlPwd,            4, 1, 1, 3);
    pLayoutA->addWidget(ui->btnSrvAdd,              5, 1);
    pLayoutA->addWidget(ui->btnSrvModify,           5, 2);
    pLayoutA->addWidget(ui->btnSrvDelete,           6, 1);
    pLayoutA->addWidget(ui->btnSrvClearList,        6, 2);
    pLayoutA->addWidget(ui->btnSrvStartMonitor,     7, 1);
    pLayoutA->addWidget(ui->btnSrvStopMonitor,      7, 2);
    pLayoutA->setColumnStretch(1, 1);
    pLayoutA->setColumnStretch(2, 1);
    //设置行比例
    for (int i = 0; i < 8; i++) {
        pLayoutA->setRowStretch(i, 1);
    }
    pLayoutA->setRowStretch(8, 5);
    ui->grpBoxServerEdit->setLayout(pLayoutA);

    //设置webhelper实时状态组框的布局
    QGridLayout *pLayoutB = new QGridLayout(this);
    pLayoutB->addWidget(ui->labelB0,                0, 0);
    pLayoutB->addWidget(ui->labelB1,                1, 0);
    pLayoutB->addWidget(ui->labelB2,                2, 0);
    pLayoutB->addWidget(ui->labelB3,                3, 0);
    pLayoutB->addWidget(ui->labelB4,                4, 0);
    pLayoutB->addWidget(ui->labelB5,                5, 0);
    pLayoutB->addWidget(ui->labelB6,                6, 0);
    pLayoutB->addWidget(ui->labelB7,                7, 0);
    pLayoutB->addWidget(ui->lineThreadPoolSize,     0, 1);
    pLayoutB->addWidget(ui->lineWorkThreadCount,    1, 1);
    pLayoutB->addWidget(ui->lineThreadPoolLoad,     2, 1);
    pLayoutB->addWidget(ui->lineUnGetResultNum,     3, 1);
    pLayoutB->addWidget(ui->lineResultCacheCount,   4, 1);
    pLayoutB->addWidget(ui->lineSendSize,           5, 1);
    pLayoutB->addWidget(ui->lineRecvSize,           6, 1);
    pLayoutB->addWidget(ui->lineRunTime,            7, 1);
    //设置行比例
    for (int i = 0; i < 9; i++) {
        pLayoutB->setRowStretch(i, 1);
    }
    pLayoutB->setRowStretch(9, 5);
    ui->grpBoxRtState->setLayout(pLayoutB);

    //设置程序升级组框的布局
    QGridLayout *pLayoutC = new QGridLayout(this);
    pLayoutC->addWidget(ui->labelC0,                0, 0);
    pLayoutC->addWidget(ui->labelC1,                1, 0);
    pLayoutC->addWidget(ui->labelC2,                2, 0);

    pLayoutC->addWidget(ui->lineUpdatingServer,     0, 1, 1, 2);
    pLayoutC->addWidget(ui->lineUpdateFilePath,     1, 1, 1, 2);
    pLayoutC->addWidget(ui->lineUpdateState,        2, 1, 1, 2);
    pLayoutC->addWidget(ui->btnScanFile,            3, 1);
    pLayoutC->addWidget(ui->btnUpdate,              3, 2);
    pLayoutC->addWidget(ui->btnReset,               4, 1);
    pLayoutC->setColumnStretch(1, 1);
    pLayoutC->setColumnStretch(2, 1);
    //设置行比例
    for (int i = 0; i < 6; i++) {
        pLayoutC->setRowStretch(i, 1);
    }
    pLayoutC->setRowStretch(5, 5);
    ui->grpBoxUpdate->setLayout(pLayoutC);

    //设置服务器列表组框的布局
    QGridLayout *pLayoutD = new QGridLayout(this);
    pLayoutD->addWidget(ui->tableServer, 0, 0);
    ui->grpBoxServer->setLayout(pLayoutD);

    //设置窗口的布局
    QGridLayout *pLayout = new QGridLayout(this);
    pLayout->addWidget(ui->grpBoxServer,     0, 0, 1, 3);
    pLayout->addWidget(ui->grpBoxServerEdit, 1, 0);
    pLayout->addWidget(ui->grpBoxRtState,    1, 1);
    pLayout->addWidget(ui->grpBoxUpdate,     1, 2);
    pLayout->setRowStretch(0, 3);
    pLayout->setRowStretch(1, 1);
    pLayout->setColumnStretch(0, 1);
    pLayout->setColumnStretch(1, 1);
    pLayout->setColumnStretch(2, 1);
    setLayout(pLayout);
}

//服务器列表编辑
bool TabServerWidget::appendServer(const QString &ip, ushort srvPort, ushort ctrlPort,
                                   int monitorPeriod, const QString &ctrlPwd)
{
    MonitorThread *pThr = CDataWare::instance()->appendServer(ip, srvPort, ctrlPort, monitorPeriod, ctrlPwd);
    if (pThr) {
        //创建成功
        connect(pThr, SIGNAL(signalStartMonitor(QString)), this, SLOT(slotStartMonitor(QString)));
        connect(pThr, SIGNAL(signalStopMonitor(QString)), this, SLOT(slotStopMonitor(QString)));
        connect(pThr, SIGNAL(signalUpdateServerInfo(bool,ServerInfo)),
                this, SLOT(slotUpdateServerInfo(bool,ServerInfo)), Qt::DirectConnection);
        connect(pThr, SIGNAL(signalChangeState(bool,QString,QString)),
                this, SIGNAL(signalChangeState(bool,QString,QString)));

        //table
        int rows = ui->tableServer->rowCount();
        QTableWidgetItem *pItem = NULL;
        QString strArr[] = {
            ip, QString::number(srvPort), QString::number(ctrlPort),
            "", "", "", "", QString::number(monitorPeriod),
            pThr->m_srvInfo.monitorState, ""
        };
        int len = sizeof(strArr)/sizeof(strArr[0]);
        ui->tableServer->insertRow(rows);
        for (int i = 0; i < len; i++) {
            pItem = new QTableWidgetItem(strArr[i]);
            ui->tableServer->setItem(rows, i, pItem);
            if (0 != i) {
                pItem->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);//右对齐
            } else {
                pItem->setCheckState(Qt::Unchecked);
            }
        }

        //发信号
        emit signalServerChanged();

        return true;
    } else {
        //创建失败
        return false;
    }
}

bool TabServerWidget::removeServer(const QString &ip)
{
    CDataWare *pDw = CDataWare::instance();
    MonitorThread *pThr = pDw->findServer(ip);
    if (pThr) {
        pThr->stopMonitor();//停止
        pDw->removeServer(ip);//删除

        //table
        int row = findItem(ip);
        if (-1 != row) {
            ui->tableServer->removeRow(row);
        }

        //lineedit
        if (0 == ui->tableServer->rowCount()) {
            ui->lineIpAddr->clear();
            ui->lineServerPort->clear();
            ui->lineCtrlPort->clear();
            ui->spinBoxMonitorPeriod->setValue(10);
            ui->lineCtrlPwd->clear();
        }

        //发信号
        emit signalServerChanged();
        return true;
    } else {
        return false;
    }
}

bool TabServerWidget::modifyServer(const QString &ip, ushort srvPort, ushort ctrlPort,
                                   int monitorPeriod, const QString &ctrlPwd)
{
    if (CDataWare::instance()->modifyServer(ip, srvPort, ctrlPort, monitorPeriod, ctrlPwd)) {
        //table
        int row = findItem(ip);
        if (-1 != row) {
            ui->tableServer->item(row, 1)->setText(QString::number(srvPort));
            ui->tableServer->item(row, 2)->setText(QString::number(ctrlPort));
            ui->tableServer->item(row, 7)->setText(QString::number(monitorPeriod));
        }
        //发信号
        emit signalServerChanged();
        return true;
    } else {
        return false;
    }
}

int TabServerWidget::findItem(const QString &ip)
{
    int rows = ui->tableServer->rowCount();
    for (int i = 0; i < rows; i++) {
        if (ip == ui->tableServer->item(i, 0)->text()) {
            return i;
        }
    }
    return -1;
}

//交互合法判断
bool TabServerWidget::isValidData()
{
    if (!CPublic::isIpAddr(ui->lineIpAddr->text())) {
        QMessageBox::warning(this, tr("警告"), tr("请输入合法的IP地址！"));
        ui->lineIpAddr->setFocus();
        return false;
    }
    if (!CPublic::isDigit(ui->lineServerPort->text())) {
        QMessageBox::warning(this, tr("警告"), tr("请输入合法的端口号！"));
        ui->lineServerPort->setFocus();
        return false;
    }
    if (!CPublic::isDigit(ui->lineCtrlPort->text())) {
        QMessageBox::warning(this, tr("警告"), tr("请输入合法的端口号！"));
        ui->lineCtrlPort->setFocus();
        return false;
    }
    if (ui->lineCtrlPwd->text().isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("控制密码不能为空！"));
        ui->lineCtrlPwd->setFocus();
        return false;
    }
    return true;
}

//服务器写入xml
void TabServerWidget::writeXml()
{
    //载入xml文件
    QString strFileName = qApp->applicationDirPath() + '/' + SET_FILE;
    QFile file(strFileName);

    QDomDocument doc("setting");
    QDomNode root;
    if (QFile::exists(strFileName)) {
        //存在
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return;
        }
        if (!doc.setContent(&file)) {
            file.close();
            return;
        }
        file.close();
        //删除tabserver标签
        root = doc.childNodes().at(1);
        QDomNode node = doc.elementsByTagName("tabserver").at(0);
        root.removeChild(node);//删除
    } else {
        //不存在
        QDomNode xmlHeader = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
        doc.appendChild(xmlHeader);

        root = doc.createElement("setting");
        doc.appendChild(root);
    }

    //添加服务器
    //创建tabserver标签
    QDomNode node, nodeServer;
    QDomNode nodeTabServer = doc.createElement("tabserver");
    root.appendChild(nodeTabServer);

    MonitorThread *pThr = NULL;
    MonitorThreadList mtList = CDataWare::instance()->getMonitorThreadList();
    string strEncrypt;
    for (int i = 0; i < mtList.size(); i++) {
        pThr = mtList.at(i);

        nodeServer = doc.createElement("webhelper");
        nodeTabServer.appendChild(nodeServer);

        node = doc.createElement("ip");
        node.appendChild(doc.createTextNode(pThr->m_srvInfo.server.ip));
        nodeServer.appendChild(node);

        node = doc.createElement("srvport");
        node.appendChild(doc.createTextNode(QString::number(pThr->m_srvInfo.server.srvPort)));
        nodeServer.appendChild(node);

        node = doc.createElement("ctrlport");
        node.appendChild(doc.createTextNode(QString::number(pThr->m_srvInfo.server.ctrlPort)));
        nodeServer.appendChild(node);

        //加密
        node = doc.createElement("ctrlpwd");
        CAes::EncryptToHexLevel(LEVEL_256, pThr->m_srvInfo.server.ctrlPwd.toStdString(), strEncrypt);
        node.appendChild(doc.createTextNode(strEncrypt.c_str()));
        nodeServer.appendChild(node);

        node = doc.createElement("monitorperiod");
        node.appendChild(doc.createTextNode(QString::number(pThr->m_srvInfo.monitorPeriod)));
        nodeServer.appendChild(node);
    }

    //保存为xml
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return;
    }
    QTextStream textStream(&file);
    doc.save(textStream, 2);
    file.close();
}

//从xml读取服务器信息
void TabServerWidget::readXml()
{
    //载入xml文件
    QString strFileName = qApp->applicationDirPath() + '/' + SET_FILE;
    QFile file(strFileName);

    QDomDocument doc("setting");

    //打开文件
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    if (!doc.setContent(&file)) {
        file.close();
        return;
    }
    file.close();

    //读取服务器
    string strDecrypt;
    QString strIp, strCtrlPwd;
    ushort iSrvPort = 0, iCtrlPort = 0;
    int iMonitorPeriod = 0;
    QDomNode node;
    QDomNodeList nodeList = doc.elementsByTagName("tabserver").at(0).toElement().elementsByTagName("webhelper");
    for (int i = 0; i < nodeList.size(); i++) {
        QDomNodeList childList = nodeList.at(i).childNodes();
        for (int j = 0; j < childList.size(); j++) {
            node = childList.at(j);
            if (node.nodeName() == "ip") {
                strIp = node.toElement().text();
            } else if (node.nodeName() == "srvport") {
                iSrvPort = node.toElement().text().toShort();
            } else if (node.nodeName() == "ctrlport") {
                iCtrlPort = node.toElement().text().toShort();
            } else if (node.nodeName() == "ctrlpwd") {
                strCtrlPwd = node.toElement().text();
                CAes::DecryptFromHexLevel(LEVEL_256, strCtrlPwd.toStdString(), strDecrypt);
            } else if (node.nodeName() == "monitorperiod") {
                iMonitorPeriod = node.toElement().text().toInt();
            }
        }
        appendServer(strIp, iSrvPort, iCtrlPort, iMonitorPeriod, strDecrypt.c_str());
    }
}

//设置列表列宽度
void TabServerWidget::setTableColumnWidth()
{
    float ratioArr[] = {0.17, 0.08, 0.08, 0.08, 0.08, 0.08, 0.1, 0.08, 0.08, 0.08, 0.08, 0.08, 0.08};
    int cols = ui->tableServer->columnCount();
    int width = ui->tableServer->width();
    for (int i = 0; i < cols; i++) {
        ui->tableServer->setColumnWidth(i, ratioArr[i]*width);
    }

//    //tabServer:898
//    ui->tableServer->setColumnWidth(0, 110);//0.15
//    ui->tableServer->setColumnWidth(1, 60);//0.08
//    ui->tableServer->setColumnWidth(2, 60);//0.08
//    ui->tableServer->setColumnWidth(3, 60);//0.08
//    ui->tableServer->setColumnWidth(4, 60);//0.08
//    ui->tableServer->setColumnWidth(5, 60);//0.08
//    ui->tableServer->setColumnWidth(6, 60);//0.08
//    ui->tableServer->setColumnWidth(7, 60);//0.08
//    ui->tableServer->setColumnWidth(7, 60);//0.08
//    ui->tableServer->setColumnWidth(8, 60);//0.08
//    ui->tableServer->setColumnWidth(9, 60);//0.08
//    ui->tableServer->setColumnWidth(10, 188);//
}

//停止所有监控线程
void TabServerWidget::stopAllMonitor()
{
    MonitorThreadList mtList = CDataWare::instance()->getMonitorThreadList();
    for (int i = 0; i < mtList.size(); i++) {
        mtList.at(i)->stopMonitor();
    }
}

//获取列表选中的服务器ip
QStringList TabServerWidget::selectedIpList()
{
    QStringList strIpList;
    int row = ui->tableServer->rowCount();

    for (int i = 0; i < row; i++) {
        if (ui->tableServer->item(i, 0)->checkState() == Qt::Checked) {
            strIpList.append(ui->tableServer->item(i, 0)->text());
        }
    }
    return strIpList;
}

//控制程序升级的按钮
void TabServerWidget::enabledUpdateWidget(bool enabled)
{
    ui->lineUpdateFilePath->setEnabled(enabled);
    ui->btnScanFile->setEnabled(enabled);
    ui->btnUpdate->setEnabled(enabled);
    ui->btnReset->setEnabled(enabled);

    ui->grpBoxServerEdit->setEnabled(enabled);
}

