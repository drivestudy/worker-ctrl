#include "tabsettingwidget.h"
#include "ui_tabsettingwidget.h"
#include <cdataware.h>
#include <cpublic.h>
#include <nofocusdelegate.h>
#include <cjson.h>
#include <ctcpsocket.h>
#include <inputpassworddialog.h>
#include <chooseurldialog.h>

TabSettingWidget::TabSettingWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabSettingWidget)
{
    ui->setupUi(this);

    initVariable();
    initLayout();
    initSetupUi();

    //读取xml配置文件
    readXml();

    //判断当前条件是否可以远程设置
    if (!isSetSqlParam() || countCheckedCkSrv()==0) {
        ui->btnRemoteSet->setEnabled(false);
    }
}

TabSettingWidget::~TabSettingWidget()
{
    delete ui;
//    DEBUG_FUNCTION
}

//服务器增减
void TabSettingWidget::slotServerChanged()
{
    QStringList strList = CDataWare::instance()->getServerIpList();
    strList.push_front(tr("全部"));
    ui->comboBoxServer->clear();
    ui->comboBoxServer->addItems(strList);
    ui->comboBoxServer->clearFocus();
}

//检测线程结束
void TabSettingWidget::slotThreadFinished()
{
    if (m_pChkCkSrvThread) {
        m_pChkCkSrvThread->deleteLater();
        m_pChkCkSrvThread = NULL;
    }
    enableWidgetForConfirm(true);
}

//处理结果
void TabSettingWidget::slotCkDealResult(int index, int total, const QString &ip, bool ok, const QString& strOk, const QString& strNo)
{
    ui->prgBarConfirm->setMaximum(total);
    ui->prgBarConfirm->setValue(index+1);
    ui->tableCookieServer->item(index, 2)->setText(ok ? strOk : strNo);
}

//本地设置
void TabSettingWidget::slotBtnLocalSet()
{
    if (!isSetSqlParam() || countCheckedCkSrv()==0) {
        QMessageBox::warning(this, tr("警告"), tr("请设置数据库参数，并且至少选择一个登录器！"));
    } else {
        //有设置数据库参数
        ui->btnRemoteSet->setEnabled(true);
    }
    //保存数据库参数
    CDataWare::instance()->setLogSqlParam(ui->lineSqlIp->text(), ui->lineSqlUser->text(),
                                          ui->lineSqlPwd->text(), ui->lineSqlDbName->text(),
                                          ui->lineSqlPort->text().toUShort());

    //设置开机启动项
    CPublic::setPowerBoot(ui->chkBoxPowerBoot->isChecked());

    //清空自动登录器表单的被选择的行
    int row = ui->tableCookieServer->rowCount();
    int col = ui->tableCookieServer->columnCount();
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            ui->tableCookieServer->item(i, j)->setSelected(false);
        }
    }

    ui->btnLocalSet->setEnabled(false);
    ui->btnModifySet->setEnabled(true);
    enableGroupBox(false);
    //写入xml
    writeXml();
}

//远程设置
void TabSettingWidget::slotBtnRemoteSet()
{
    //输入密码
    InputPasswordDialog dialog(this);
    dialog.setModal(true);
    if (dialog.exec() == QDialog::Accepted) {
        if (UI_OP_PASSWORD != dialog.getPassword()) {
            QMessageBox::warning(this, tr("警告"), tr("请输入正确的密码！"));
            return;
        }
    } else {
        return;
    }

    if (QMessageBox::Ok != QMessageBox::warning(this, tr("警告"), tr("确定要进行远程设置？"),
                                                QMessageBox::Ok | QMessageBox::Cancel)) {
        return;
    }

    if (countCheckedCkSrv() == 0) {
        QMessageBox::warning(this, tr("警告"), tr("请选择在线的太阳城自动登录器！"));
        return;
    }

    ui->prgBarSet->setValue(0);
    qApp->processEvents();

    //读取已选择的cookie服务器
    QStringList strCkSrvList;
    int row = ui->tableCookieServer->rowCount();
    for (int i = 0; i < row; i++) {
        if (ui->tableCookieServer->item(i, 0)->checkState() == Qt::Checked) {
            strCkSrvList.append(tr("%1:%2").arg(ui->tableCookieServer->item(i, 0)->text(),
                                                ui->tableCookieServer->item(i, 1)->text()));
        }
    }

    //生成控制命令
    int index = ui->comboBoxServer->currentIndex();
    Server *pSrv = NULL;
    CDataWare *pDw = CDataWare::instance();
    QString strErrInfo;
    QStringList strErrInfoList;

    if (0 == index) {
        //设置全部服务器
        ServerList srvList = pDw->getServerList();
        for (int i = 0; i < srvList.size(); i++) {
            setParamToServer(srvList.at(i), strCkSrvList, strErrInfo);
            strErrInfoList.append(tr("%1: %2").arg(srvList.at(i)->ip, strErrInfo));
            ui->prgBarSet->setMaximum(srvList.size());
            ui->prgBarSet->setValue(i+1);
            qApp->processEvents();
        }
    } else {
        //设置当前选中服务器
        pSrv = pDw->getServer(ui->comboBoxServer->currentText());
        if (pSrv) {
            setParamToServer(pSrv, strCkSrvList, strErrInfo);
            strErrInfoList.append(tr("%1: %2").arg(pSrv->ip, strErrInfo));
            ui->prgBarSet->setMaximum(1);
            ui->prgBarSet->setValue(1);
        }
    }

    //显示处理结果
    strErrInfo.clear();
    for (int i = 0; i < strErrInfoList.size(); i++) {
        strErrInfo.append(tr("%1\n").arg(strErrInfoList.at(i)));
    }
    QMessageBox::information(this, tr("处理结果"), strErrInfo);
}

//修改设置
void TabSettingWidget::slotBtnModifySet()
{
    enableGroupBox(true);
    ui->btnLocalSet->setEnabled(true);
    ui->btnModifySet->setEnabled(false);
    ui->btnRemoteSet->setEnabled(false);
}

//下载设置
void TabSettingWidget::slotBtnDownloadSet()
{
    //输入密码
    InputPasswordDialog dialog(this);
    dialog.setModal(true);
    if (dialog.exec() == QDialog::Accepted) {
        if (UI_OP_PASSWORD != dialog.getPassword()) {
            QMessageBox::warning(this, tr("警告"), tr("请输入正确的密码！"));
            return;
        }
    } else {
        return;
    }

    if (QMessageBox::Ok != QMessageBox::warning(this, tr("警告"), tr("确定要下载远程系统参数？"),
                                                QMessageBox::Ok | QMessageBox::Cancel)) {
        return;
    }

    //生成控制命令
    int index = ui->comboBoxServer->currentIndex();
    Server *pSrv = NULL;
    CDataWare *pDw = CDataWare::instance();
    QString strErrInfo;
    QString strTmp;
    CSqlParam sqlParam;
    QStringList strCkList;

    //初始化输出窗口
    ui->textParam->clear();
    qApp->processEvents();

    if (0 == index) {
        //获取全部服务器的参数
        ServerList srvList = pDw->getServerList();
        for (int i = 0; i < srvList.size(); i++) {
            pSrv = srvList.at(i);
            strTmp = "【" + pSrv->ip + "】: ";
            if (getParamFromServer(pSrv, sqlParam, strCkList, strErrInfo)) {
                strTmp += tr("在线\n"
                             "数据库参数: %1:%2 用户名(%3) 数据库(%4)\n"
                             "自动登录器: ")
                        .arg(sqlParam.strIp)
                        .arg(sqlParam.iPort)
                        .arg(sqlParam.strUser)
                        .arg(sqlParam.strDbname);
                for (int j = 0; j < strCkList.size(); j++) {
                    strTmp.append(tr("\n\t   %1: %2").arg(j+1).arg(strCkList.at(j)));
                }
            } else {
                strTmp += strErrInfo;
            }
            strTmp += "\n";
            ui->textParam->append(strTmp);
            qApp->processEvents();
        }
    } else {
        //设置当前选中服务器
        pSrv = pDw->getServer(ui->comboBoxServer->currentText());
        if (pSrv) {
            strTmp = "【" + pSrv->ip + "】: ";
            if (getParamFromServer(pSrv, sqlParam, strCkList, strErrInfo)) {
                strTmp += tr("在线\n"
                             "数据库参数: %1:%2 用户名(%3) 数据库(%4)\n"
                             "自动登录器: ")
                        .arg(sqlParam.strIp)
                        .arg(sqlParam.iPort)
                        .arg(sqlParam.strUser)
                        .arg(sqlParam.strDbname);
                for (int j = 0; j < strCkList.size(); j++) {
                    strTmp.append(tr("\n\t   %1: %2").arg(j+1).arg(strCkList.at(j)));
                }
            } else {
                strTmp += tr("离线 %1\n").arg(strErrInfo);
            }
            ui->textParam->append(strTmp);
            qApp->processEvents();
        }
    }

    ui->textParam->append(tr("\n下载远程系统参数完成"));
}

//默认
void TabSettingWidget::slotBtnDefault()
{
//    ui->lineSqlIp->setText("106.187.36.187");
//    ui->lineSqlPort->setText("33306");
//    ui->lineSqlUser->setText("remote_tw_003");
//    ui->lineSqlPwd->setText("q5EwqNOomgk");
//    ui->lineSqlDbName->setText("webhelper");

    ui->lineSqlIp->setText("124.150.131.3");
    ui->lineSqlPort->setText("60000");
    ui->lineSqlUser->setText("remote_tw_003");
    ui->lineSqlPwd->setText("q5EwqNOomgk");
    ui->lineSqlDbName->setText("webhelper");
}

//重置
void TabSettingWidget::slotBtnReset()
{
    ui->lineSqlIp->clear();
    ui->lineSqlPort->clear();
    ui->lineSqlUser->clear();
    ui->lineSqlPwd->clear();
    ui->lineSqlDbName->clear();
}

//连接测试
void TabSettingWidget::slotBtnTestConnect()
{
    if (!isValidData()) {
        return;
    }

    //特意加上该作用域
    {
        QSqlDatabase sqlDb = QSqlDatabase::addDatabase("QMYSQL", "_test_sql_");
        sqlDb.setHostName(ui->lineSqlIp->text());
        sqlDb.setDatabaseName(ui->lineSqlDbName->text());
        sqlDb.setUserName(ui->lineSqlUser->text());
        sqlDb.setPort(ui->lineSqlPort->text().toUShort());
        sqlDb.setPassword(ui->lineSqlPwd->text());
        if (sqlDb.open()) {
            QMessageBox::information(this, tr("连接成功"), tr("数据库连接成功！"));
            sqlDb.close();
        } else {
            QMessageBox::warning(this, tr("连接失败"), tr("数据库连接失败！"));
        }
    }
    QSqlDatabase::removeDatabase("_test_sql_");
}

//添加
void TabSettingWidget::slotBtnAdd()
{
    //ip地址
    if (!CPublic::isIpAddr(ui->lineCookieIp->text())) {
        QMessageBox::warning(this, tr("警告"), tr("请输入合法的IP地址！"));
        ui->lineCookieIp->setFocus();
        return;
    }

    //端口
    if (!CPublic::isDigit(ui->lineCookiePort->text())) {
        QMessageBox::warning(this, tr("警告"), tr("请输入合法的端口号！"));
        ui->lineCookiePort->setFocus();
        return;
    }

    //判断ip是否存在
    if (findIpFromTable(ui->lineCookieIp->text()) != -1) {
        QMessageBox::warning(this, tr("警告"), tr("请不要重复添加自动登录器！"));
        return;
    }

    //添加到表格里面
    appendCookieServer(ui->lineCookieIp->text(), ui->lineCookiePort->text(), Qt::Unchecked);

    ui->lineCookieIp->clear();
    ui->lineCookieIp->setFocus();
}

//删除
void TabSettingWidget::slotBtnDelete()
{
    if (QMessageBox::warning(this, tr("警告"), tr("是否确定删除以选中的自动登录器？"), QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok) {
        return;
    }

    QList<int> intList;
    int len = ui->tableCookieServer->rowCount();

    //将选中的项的行数添加到链表
    QTableWidgetItem *pItem = NULL;
    for (int i = 0; i < len; i++) {
        pItem = ui->tableCookieServer->item(i, 0);
        if (pItem->checkState() == Qt::Checked) {
            intList.append(i);
        }
    }

    //从后往前删除表格项
    len = intList.size();
    for (int i = len - 1; i >= 0; i--) {
        ui->tableCookieServer->removeRow(intList.at(i));
    }
}

//清空
void TabSettingWidget::slotBtnClear()
{
    if (QMessageBox::warning(this, tr("警告"), tr("是否确定清空所有自动登录器？"), QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok) {
        return;
    }
    ui->tableCookieServer->clearContents();
    ui->tableCookieServer->setRowCount(0);
    ui->prgBarConfirm->setValue(0);
}

//验证
void TabSettingWidget::slotBtnConfirm()
{
    if (countCheckedCkSrv() == 0) {
        QMessageBox::warning(this, tr("警告"), tr("请选择要验证的登录器！"));
        return;
    }

    //ui准备
    ui->prgBarConfirm->setValue(0);
    enableWidgetForConfirm(false);

    //创建线程
    m_pChkCkSrvThread = new CCheckCookieServerThread(this, ECK_Heart);
    connect(m_pChkCkSrvThread, SIGNAL(finished()), this, SLOT(slotThreadFinished()));
    connect(m_pChkCkSrvThread, SIGNAL(signalCkDealResult(int,int,QString,bool,QString,QString)),
            this, SLOT(slotCkDealResult(int,int,QString,bool,QString,QString)));

    //将要检测的ip加入到检测线程
    int len = ui->tableCookieServer->rowCount();
    QTableWidgetItem *pItem = NULL;
    for (int i = 0; i < len; i++) {
        pItem = ui->tableCookieServer->item(i, 0);
        if (pItem->checkState() == Qt::Checked) {
            ui->tableCookieServer->item(i, 2)->setText("检测中");
            m_pChkCkSrvThread->addCookieServer(i,
                                               pItem->text(),
                                               ui->tableCookieServer->item(i, 1)->text().toUShort());
        }
    }
    //启动线程开始检测
    m_pChkCkSrvThread->start();
}

//设置网址
void TabSettingWidget::slotBtnSetUrl()
{
    if (countCheckedCkSrv() == 0)
    {
        QMessageBox::warning(this, tr("警告"), tr("请选择要验证的登录器！"));
        return;
    }

    ChooseUrlDialog dlg(this);
    if (QDialog::Accepted == dlg.exec())
    {
        //ui准备
        ui->prgBarConfirm->setValue(0);
        enableWidgetForConfirm(false);

        //创建线程
        m_pChkCkSrvThread = new CCheckCookieServerThread(this, ECK_SetUrl);
        connect(m_pChkCkSrvThread, SIGNAL(finished()), this, SLOT(slotThreadFinished()));
        connect(m_pChkCkSrvThread, SIGNAL(signalCkDealResult(int,int,QString,bool,QString,QString)),
                this, SLOT(slotCkDealResult(int,int,QString,bool,QString,QString)));

        //将要检测的ip加入到检测线程
        int len = ui->tableCookieServer->rowCount();
        QTableWidgetItem *pItem = NULL;
        for (int i = 0; i < len; i++) {
            pItem = ui->tableCookieServer->item(i, 0);
            if (pItem->checkState() == Qt::Checked) {
                ui->tableCookieServer->item(i, 2)->setText("设置中");
                m_pChkCkSrvThread->addCookieServer(i,
                                                   pItem->text(),
                                                   ui->tableCookieServer->item(i, 1)->text().toUShort());
            }
        }
        m_pChkCkSrvThread->setUrlFlag(dlg.GetUrlFlag());
        //启动线程开始检测
        m_pChkCkSrvThread->start();
    }
}

//全选
void TabSettingWidget::slotBtnSelectAll()
{
    int len = ui->tableCookieServer->rowCount();
    for (int i = 0; i < len; i++) {
        ui->tableCookieServer->item(i, 0)->setCheckState(Qt::Checked);
    }
}

//反选
void TabSettingWidget::slotBtnUnselect()
{
    int len = ui->tableCookieServer->rowCount();
    QTableWidgetItem *pItem = NULL;
    for (int i = 0; i < len; i++) {
        pItem = ui->tableCookieServer->item(i, 0);
        pItem->setCheckState((pItem->checkState() == Qt::Checked) ? Qt::Unchecked : Qt::Checked);
    }
}

//初始化变量
void TabSettingWidget::initVariable()
{

}

//初始化ui
void TabSettingWidget::initSetupUi()
{
    //设置
    slotServerChanged();

    //日志数据库参数
    slotBtnReset();

    //太阳城自动登录器
    ui->tableCookieServer->setItemDelegate(new NoFocusDelegate(this));
    ui->tableCookieServer->verticalHeader()->setResizeMode(QHeaderView::Fixed);
    ui->tableCookieServer->setColumnWidth(0, 150);
    ui->tableCookieServer->setColumnWidth(1, 60);
    ui->tableCookieServer->clearContents();
    ui->tableCookieServer->setRowCount(0);

    //默认不使用控件
    enableGroupBox(false);

    //保存按钮禁用
    ui->btnLocalSet->setEnabled(false);

    //slot
    connect(ui->btnLocalSet, SIGNAL(clicked()), this, SLOT(slotBtnLocalSet()));
    connect(ui->btnRemoteSet, SIGNAL(clicked()), this, SLOT(slotBtnRemoteSet()));
    connect(ui->btnModifySet, SIGNAL(clicked()), this, SLOT(slotBtnModifySet()));
    connect(ui->btnDownloadSet, SIGNAL(clicked()), this, SLOT(slotBtnDownloadSet()));
    connect(ui->btnDefault, SIGNAL(clicked()), this, SLOT(slotBtnDefault()));
    connect(ui->btnReset, SIGNAL(clicked()), this, SLOT(slotBtnReset()));
    connect(ui->btnTestConnect, SIGNAL(clicked()), this, SLOT(slotBtnTestConnect()));
    connect(ui->btnAdd, SIGNAL(clicked()), this, SLOT(slotBtnAdd()));
    connect(ui->btnDelete, SIGNAL(clicked()), this, SLOT(slotBtnDelete()));
    connect(ui->btnClear, SIGNAL(clicked()), this, SLOT(slotBtnClear()));
    connect(ui->btnConfirm, SIGNAL(clicked()), this, SLOT(slotBtnConfirm()));
    connect(ui->btnSetUrl, SIGNAL(clicked()), this, SLOT(slotBtnSetUrl()));
    connect(ui->btnSelectAll, SIGNAL(clicked()), this, SLOT(slotBtnSelectAll()));
    connect(ui->btnUnselect, SIGNAL(clicked()), this, SLOT(slotBtnUnselect()));
    connect(ui->lineCookieIp, SIGNAL(returnPressed()), this, SLOT(slotBtnAdd()));
    connect(ui->lineCookiePort, SIGNAL(returnPressed()), this, SLOT(slotBtnAdd()));
}

//初始化layout
void TabSettingWidget::initLayout()
{
    //基本设置
    QGridLayout *pLayout1 = new QGridLayout(this);
    pLayout1->addWidget(ui->chkBoxPowerBoot, 0, 0);
    ui->grpBoxBaseSet->setLayout(pLayout1);

    //设置
    QGridLayout *pLayout3 = new QGridLayout(this);
    pLayout3->addWidget(ui->labelC0,        0,  0);
    pLayout3->addWidget(ui->labelC1,        1,  0);
    pLayout3->addWidget(ui->labelC2,        2,  0);

    pLayout3->addWidget(ui->comboBoxServer, 0,  1,  1,  3);
    pLayout3->addWidget(ui->prgBarSet,      1,  1,  1,  3);
    pLayout3->addWidget(ui->btnLocalSet,    2,  1,  1,  1);
    pLayout3->addWidget(ui->btnModifySet,   2,  2,  1,  1);
    pLayout3->addWidget(ui->btnRemoteSet,   2,  3,  1,  1);
    pLayout3->addWidget(ui->btnDownloadSet, 3,  1,  1,  1);
    ui->grpBoxSetting->setLayout(pLayout3);

    //太阳城自动登录器
    QGridLayout *pLayout4 = new QGridLayout(this);
    pLayout4->addWidget(ui->labelD0,            0,  0);
    pLayout4->addWidget(ui->lineCookieIp,       0,  1);
    pLayout4->addWidget(ui->lineCookiePort,     0,  2);
    pLayout4->addWidget(ui->tableCookieServer,  1,  0,  8,  3);
    pLayout4->addWidget(ui->btnAdd,             1,  3);
    pLayout4->addWidget(ui->btnDelete,          2,  3);
    pLayout4->addWidget(ui->btnClear,           3,  3);
    pLayout4->addWidget(ui->btnSelectAll,       4,  3);
    pLayout4->addWidget(ui->btnUnselect,        5,  3);
    pLayout4->addWidget(ui->btnConfirm,         6,  3);
    pLayout4->addWidget(ui->btnSetUrl,          7,  3);
    pLayout4->addWidget(ui->prgBarConfirm,      8,  3);
    pLayout4->setColumnStretch(1, 2);
    pLayout4->setColumnStretch(2, 1);
    ui->grpBoxCookieServer->setLayout(pLayout4);

    //日志数据库参数
    QGridLayout *pLayout5 = new QGridLayout(this);
    pLayout5->addWidget(ui->labelA0,        0,  0);
    pLayout5->addWidget(ui->labelA1,        1,  0);
    pLayout5->addWidget(ui->labelA2,        2,  0);
    pLayout5->addWidget(ui->labelA3,        3,  0);
    pLayout5->addWidget(ui->labelA4,        4,  0);
    pLayout5->addWidget(ui->lineSqlIp,      0,  1,  1,  3);
    pLayout5->addWidget(ui->lineSqlPort,    1,  1,  1,  3);
    pLayout5->addWidget(ui->lineSqlUser,    2,  1,  1,  3);
    pLayout5->addWidget(ui->lineSqlPwd,     3,  1,  1,  3);
    pLayout5->addWidget(ui->lineSqlDbName,  4,  1,  1,  3);
    pLayout5->addWidget(ui->btnDefault,     5,  1);
    pLayout5->addWidget(ui->btnReset,       5,  2);
    pLayout5->addWidget(ui->btnTestConnect, 5,  3);
    pLayout5->setColumnStretch(1, 1);
    pLayout5->setColumnStretch(2, 1);
    pLayout5->setColumnStretch(3, 1);
    ui->grpBoxSqlParam->setLayout(pLayout5);

    //系统参数
    QGridLayout *pLayout6 = new QGridLayout(this);
    pLayout6->addWidget(ui->textParam, 0, 0);
    ui->grpBoxDlRemoteParam->setLayout(pLayout6);

    //窗口布局
    QGridLayout *pLayoutA = new QGridLayout(this);
    pLayoutA->addWidget(ui->grpBoxSetting,      0, 0);
    pLayoutA->addWidget(ui->grpBoxBaseSet,      1, 0);
    pLayoutA->addWidget(ui->grpBoxSqlParam,     2, 0);
    pLayoutA->addWidget(ui->grpBoxCookieServer, 1, 1, 2, 1);
    pLayoutA->addWidget(ui->grpBoxDlRemoteParam,3, 0, 1, 2);
    pLayoutA->setRowStretch(0, 1);
    pLayoutA->setRowStretch(1, 1);
    pLayoutA->setRowStretch(2, 4);
    pLayoutA->setRowStretch(3, 6);
    setLayout(pLayoutA);
}

//交互合法判断
bool TabSettingWidget::isValidData()
{
    //ip地址
    if (!CPublic::isIpAddr(ui->lineSqlIp->text())) {
        QMessageBox::warning(this, tr("警告"), tr("请输入合法的IP地址！"));
        ui->lineSqlIp->setFocus();
        return false;
    }

    //端口
    if (!CPublic::isDigit(ui->lineSqlPort->text())) {
        QMessageBox::warning(this, tr("警告"), tr("请输入合法的端口号！"));
        ui->lineSqlPort->setFocus();
        return false;
    }

    return true;
}

//从table中寻找ip,返回row
int TabSettingWidget::findIpFromTable(const QString &ip)
{
    int index = -1;
    int len = ui->tableCookieServer->rowCount();
    for (int i = 0; i < len; i++) {
        if (ui->tableCookieServer->item(i, 0)->text() == ip) {
            index = i;
        }
    }
    return index;
}

//启用禁用控件
void TabSettingWidget::enableGroupBox(bool enabled)
{
    ui->grpBoxBaseSet->setEnabled(enabled);
    ui->grpBoxCookieServer->setEnabled(enabled);
    ui->grpBoxSqlParam->setEnabled(enabled);
}

//启用禁用控件:验证cookie服务器
void TabSettingWidget::enableWidgetForConfirm(bool enabled)
{
    ui->grpBoxSetting->setEnabled(enabled);
    ui->grpBoxBaseSet->setEnabled(enabled);
    ui->grpBoxSqlParam->setEnabled(enabled);
    ui->btnAdd->setEnabled(enabled);
    ui->btnDelete->setEnabled(enabled);
    ui->btnClear->setEnabled(enabled);
    ui->btnSelectAll->setEnabled(enabled);
    ui->btnUnselect->setEnabled(enabled);
    ui->btnConfirm->setEnabled(enabled);
    ui->btnSetUrl->setEnabled(enabled);
}

//添加cookie服务器
void TabSettingWidget::appendCookieServer(const QString &ip, const QString &port, Qt::CheckState chk)
{
    //添加到表格里面
    int row = ui->tableCookieServer->rowCount();
    QString strArr[] = {ip, port, ""};
    int len = sizeof(strArr)/sizeof(strArr[0]);

    QTableWidgetItem *pItem = NULL;
    ui->tableCookieServer->insertRow(row);
    for (int i = 0; i < len; i++) {
        pItem = new QTableWidgetItem(strArr[i]);
        if (0 == i) {
            pItem->setCheckState(chk);
        }
        ui->tableCookieServer->setItem(row, i, pItem);
    }
}

//计算cookie服务器被选中的个数
int TabSettingWidget::countCheckedCkSrv()
{
    int count = 0;
    int row = ui->tableCookieServer->rowCount();

    for (int i = 0; i < row; i++) {
        if (ui->tableCookieServer->item(i, 0)->checkState() == Qt::Checked) {
            ++count;
        }
    }
    return count;
}

//是否设置数据库参数
bool TabSettingWidget::isSetSqlParam()
{
    return !(ui->lineSqlIp->text().isEmpty() || ui->lineSqlPort->text().isEmpty()
            || ui->lineSqlUser->text().isEmpty() || ui->lineSqlPwd->text().isEmpty()
            || ui->lineSqlDbName->text().isEmpty());
}

//生成请求命令
string TabSettingWidget::GenCmdRequest(CMDTYPE cmdtype, const QString &pwd, const QStringList &ckSrvList, int aesLevel)
{
    Json::Value jsql, jcksrv, jopt;

    jsql["ip"] = ui->lineSqlIp->text().toStdString();
    jsql["port"] = ui->lineSqlPort->text().toInt();
    jsql["user"] = ui->lineSqlUser->text().toStdString();
    jsql["pwd"] = ui->lineSqlPwd->text().toStdString();
    jsql["dbname"] = ui->lineSqlDbName->text().toStdString();

    for (int i = 0; i < ckSrvList.size(); i++) {
        jcksrv.append(ckSrvList.at(i).toStdString());
    }

    jopt["sql"] = jsql;
    jopt["cookiesrv"] = jcksrv;

    Json::Value jobj;
    jobj["cmd"] = cmdtype;
    jobj["pwd"] = pwd.toStdString();
    jobj["opt"] = jopt;

    string strEncryptJson;
    if (0 != aesLevel) {
        CAes::EncryptToHexLevel(aesLevel, CJson::JsonWriteToString(jobj), strEncryptJson);
    } else {
        strEncryptJson = CJson::JsonWriteToString(jobj);
    }

    char buf[256];
    sprintf(buf, "%s-%d-", CMDREQ, aesLevel);
    string strReq = buf + strEncryptJson + CMDEND;
    return strReq;
}

string TabSettingWidget::GenCmdRequest(CMDTYPE cmdtype, const QString &pwd, int aesLevel)
{
    Json::Value jobj;
    jobj["cmd"] = cmdtype;
    jobj["pwd"] = pwd.toStdString();

    string strEncryptJson;
    if (0 != aesLevel) {
        CAes::EncryptToHexLevel(aesLevel, CJson::JsonWriteToString(jobj), strEncryptJson);
    } else {
        strEncryptJson = CJson::JsonWriteToString(jobj);
    }

    char buf[256];
    sprintf(buf, "%s-%d-", CMDREQ, aesLevel);
    string strReq = buf + strEncryptJson + CMDEND;
    return strReq;
}

//与服务器对话
bool TabSettingWidget::talkWithServer(const QString &ip, ushort srvPort, const string &cmd, QString &recv)
{
    QTcpSocket tcpSocket;

    tcpSocket.connectToHost(ip, srvPort);
    if (!tcpSocket.waitForConnected(4000)) {
        return false;
    }
    qApp->processEvents();

    tcpSocket.write(cmd.c_str());
    if (!tcpSocket.waitForBytesWritten(4000)) {
        return false;
    }
    qApp->processEvents();

    if (!tcpSocket.waitForReadyRead(4000)) {
        return false;
    }
    qApp->processEvents();
    recv = tcpSocket.readAll();
    tcpSocket.close();
    return true;
}

//设置参数到webhelper
void TabSettingWidget::setParamToServer(Server *pSrv, const QStringList &ckSrvList, QString &errInfo)
{
    QString strRecv;
    string strCmd = GenCmdRequest(CMD_SETPARAM, pSrv->ctrlPwd, ckSrvList);

    if (!talkWithServer(pSrv->ip, pSrv->srvPort, strCmd, strRecv)) {
        errInfo = tr("服务器已离线");
        return;
    }

    //解析
    QStringList strList = strRecv.split('-');
    if (strList.size() != 4) {
        errInfo = tr("数据解析失败");
        return;
    }

    //解密
    int iAesLevel = strList.at(1).toInt();
    string strDecrypt;
    CAes::DecryptFromHexLevel(iAesLevel, strList.at(2).toStdString(), strDecrypt);

    //解析json
    Json::Value jval;
    if (!CJson::ParseStringToJsonValue(strDecrypt, jval)) {
        errInfo = tr("数据解析失败");
        return;
    }

    if (CJson::JsonValueToInt(jval["result"]) == 0) {
        errInfo = tr("控制密码错误");
        return;
    }

    errInfo = tr("设置成功");
}

bool TabSettingWidget::getParamFromServer(Server *pSrv, CSqlParam &sqlParam, QStringList &ckSrvList, QString &errInfo)
{
    QString strRecv;
    string strCmd = GenCmdRequest(CMD_GETPARAM, pSrv->ctrlPwd, ckSrvList);

    if (!talkWithServer(pSrv->ip, pSrv->srvPort, strCmd, strRecv)) {
        errInfo = tr("服务器已离线");
        return false;
    }

    //解析
    QStringList strList = strRecv.split('-');
    if (strList.size() != 4) {
        errInfo = tr("数据解析失败");
        return false;
    }

    //解密
    int iAesLevel = strList.at(1).toInt();
    string strDecrypt;
    CAes::DecryptFromHexLevel(iAesLevel, strList.at(2).toStdString(), strDecrypt);

    //解析json
    Json::Value jval;
    if (!CJson::ParseStringToJsonValue(strDecrypt, jval)) {
        errInfo = tr("数据解析失败");
        return false;
    }

    if (CJson::JsonValueToInt(jval["result"]) == 0) {
        errInfo = tr("控制密码错误");
        return false;
    }

    //从json数据中把数据解析出来
    Json::Value jsql = jval["info"]["sql"];
    Json::Value jck = jval["info"]["cookiesrv"];

    if (!jsql.isObject() || !jck.isArray()) {
        errInfo = tr("Json数据解析失败");
        return false;
    }

    //解析数据库信息
    sqlParam.strIp = CJson::JsonValueToStringEx(jsql["ip"]).c_str();
    sqlParam.strUser = CJson::JsonValueToStringEx(jsql["user"]).c_str();
    sqlParam.strPwd = CJson::JsonValueToStringEx(jsql["pwd"]).c_str();
    sqlParam.strDbname = CJson::JsonValueToStringEx(jsql["dbname"]).c_str();
    sqlParam.iPort = CJson::JsonValueToInt(jsql["port"]);

    //解析cookie信息
    ckSrvList.clear();
    for (uint i = 0; i < jck.size(); i++) {
        ckSrvList.append(CJson::JsonValueToStringEx(jck[i]).c_str());
    }

    return true;
}

//读取xml文件
void TabSettingWidget::readXml()
{
    //载入xml文件
    QString strFileName = qApp->applicationDirPath() + '/' + SET_FILE;
    QFile file(strFileName);

    QDomDocument doc("setting");

    //存在
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    if (!doc.setContent(&file)) {
        file.close();
        return;
    }
    file.close();

    int iRes = 0;
    string strDecrypt;
    QDomNode node;
    QDomElement elementTabSetting = doc.elementsByTagName("tabsetting").at(0).toElement();

    //读取基本设置
    QDomElement elementBaseSet = elementTabSetting.elementsByTagName("baseset").at(0).toElement();
    iRes = elementBaseSet.elementsByTagName("powerboot").at(0).toElement().text().toInt();
    ui->chkBoxPowerBoot->setCheckState((Qt::CheckState)iRes);

    //读取数据库参数
    QDomNodeList nodeList = elementTabSetting.elementsByTagName("mysqlparam").at(0).toElement().childNodes();
    for (int i = 0; i < nodeList.size(); i++) {
        node = nodeList.at(i);
        if (node.nodeName() == "ip") {
            ui->lineSqlIp->setText(node.toElement().text());
        } else if (node.nodeName() == "port") {
            ui->lineSqlPort->setText(node.toElement().text());
        } else if (node.nodeName() == "user") {
            ui->lineSqlUser->setText(node.toElement().text());
        } else if (node.nodeName() == "pwd") {
            CAes::DecryptFromHexLevel(LEVEL_256, node.toElement().text().toStdString(), strDecrypt);
            ui->lineSqlPwd->setText(strDecrypt.c_str());
        } else if (node.nodeName() == "dbname") {
            ui->lineSqlDbName->setText(node.toElement().text());
        }
    }

    //保存数据库参数
    CDataWare::instance()->setLogSqlParam(ui->lineSqlIp->text(), ui->lineSqlUser->text(),
                                          ui->lineSqlPwd->text(), ui->lineSqlDbName->text(),
                                          ui->lineSqlPort->text().toUShort());

    //读取cookie服务器
    QDomNodeList childList;
    QString strIp, strPort;
    nodeList = elementTabSetting.elementsByTagName("cookieserver");
    for (int i = 0; i < nodeList.size(); i++) {
        childList = nodeList.at(i).childNodes();
        for (int j = 0; j < childList.size(); j++) {
            node = childList.at(j);
            if (node.nodeName() == "ip") {
                strIp = node.toElement().text();
            } else if (node.nodeName() == "port") {
                strPort = node.toElement().text();
            } else if (node.nodeName() == "check") {
                iRes = node.toElement().text().toInt();
            }
        }
        //添加到ui
        appendCookieServer(strIp, strPort, (Qt::CheckState)iRes);
    }
}

//写入xml文件
void TabSettingWidget::writeXml()
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
        //删除tabsetting标签
        root = doc.childNodes().at(1);
        QDomNode node = doc.elementsByTagName("tabsetting").at(0);
        root.removeChild(node);//删除
    } else {
        //不存在
        QDomNode xmlHeader = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
        doc.appendChild(xmlHeader);

        root = doc.createElement("setting");
        doc.appendChild(root);
    }

    //创建tabserver标签
    string strEncrypt;
    QDomNode node;
    QDomNode nodeTabSetting = doc.createElement("tabsetting");
    root.appendChild(nodeTabSetting);

    //保存基本设置
    QDomNode nodeBaseSet = doc.createElement("baseset");
    nodeTabSetting.appendChild(nodeBaseSet);

    node = doc.createElement("powerboot");
    node.appendChild(doc.createTextNode(QString::number(ui->chkBoxPowerBoot->checkState())));
    nodeBaseSet.appendChild(node);

    //保存数据库参数
    QDomNode nodeMysql = doc.createElement("mysqlparam");
    nodeTabSetting.appendChild(nodeMysql);

    node = doc.createElement("ip");
    node.appendChild(doc.createTextNode(ui->lineSqlIp->text()));
    nodeMysql.appendChild(node);

    node = doc.createElement("port");
    node.appendChild(doc.createTextNode(ui->lineSqlPort->text()));
    nodeMysql.appendChild(node);

    node = doc.createElement("user");
    node.appendChild(doc.createTextNode(ui->lineSqlUser->text()));
    nodeMysql.appendChild(node);

    node = doc.createElement("pwd");
    CAes::EncryptToHexLevel(LEVEL_256, ui->lineSqlPwd->text().toStdString(), strEncrypt);
    node.appendChild(doc.createTextNode(strEncrypt.c_str()));
    nodeMysql.appendChild(node);

    node = doc.createElement("dbname");
    node.appendChild(doc.createTextNode(ui->lineSqlDbName->text()));
    nodeMysql.appendChild(node);

    //保存cookie服务器
    int rows = ui->tableCookieServer->rowCount();
    QDomNode nodeCookieServer;
    for (int i = 0; i < rows; i++) {
        nodeCookieServer = doc.createElement("cookieserver");
        nodeTabSetting.appendChild(nodeCookieServer);

        node = doc.createElement("ip");
        node.appendChild(doc.createTextNode(ui->tableCookieServer->item(i, 0)->text()));
        nodeCookieServer.appendChild(node);

        node = doc.createElement("port");
        node.appendChild(doc.createTextNode(ui->tableCookieServer->item(i, 1)->text()));
        nodeCookieServer.appendChild(node);

        node = doc.createElement("check");
        node.appendChild(doc.createTextNode(QString::number(ui->tableCookieServer->item(i, 0)->checkState())));

        nodeCookieServer.appendChild(node);
    }

    //保存为xml
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return;
    }
    QTextStream textStream(&file);
    doc.save(textStream, 2);
    file.close();
}

