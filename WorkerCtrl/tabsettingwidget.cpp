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

    //��ȡxml�����ļ�
    readXml();

    //�жϵ�ǰ�����Ƿ����Զ������
    if (!isSetSqlParam() || countCheckedCkSrv()==0) {
        ui->btnRemoteSet->setEnabled(false);
    }
}

TabSettingWidget::~TabSettingWidget()
{
    delete ui;
//    DEBUG_FUNCTION
}

//����������
void TabSettingWidget::slotServerChanged()
{
    QStringList strList = CDataWare::instance()->getServerIpList();
    strList.push_front(tr("ȫ��"));
    ui->comboBoxServer->clear();
    ui->comboBoxServer->addItems(strList);
    ui->comboBoxServer->clearFocus();
}

//����߳̽���
void TabSettingWidget::slotThreadFinished()
{
    if (m_pChkCkSrvThread) {
        m_pChkCkSrvThread->deleteLater();
        m_pChkCkSrvThread = NULL;
    }
    enableWidgetForConfirm(true);
}

//������
void TabSettingWidget::slotCkDealResult(int index, int total, const QString &ip, bool ok, const QString& strOk, const QString& strNo)
{
    ui->prgBarConfirm->setMaximum(total);
    ui->prgBarConfirm->setValue(index+1);
    ui->tableCookieServer->item(index, 2)->setText(ok ? strOk : strNo);
}

//��������
void TabSettingWidget::slotBtnLocalSet()
{
    if (!isSetSqlParam() || countCheckedCkSrv()==0) {
        QMessageBox::warning(this, tr("����"), tr("���������ݿ��������������ѡ��һ����¼����"));
    } else {
        //���������ݿ����
        ui->btnRemoteSet->setEnabled(true);
    }
    //�������ݿ����
    CDataWare::instance()->setLogSqlParam(ui->lineSqlIp->text(), ui->lineSqlUser->text(),
                                          ui->lineSqlPwd->text(), ui->lineSqlDbName->text(),
                                          ui->lineSqlPort->text().toUShort());

    //���ÿ���������
    CPublic::setPowerBoot(ui->chkBoxPowerBoot->isChecked());

    //����Զ���¼�����ı�ѡ�����
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
    //д��xml
    writeXml();
}

//Զ������
void TabSettingWidget::slotBtnRemoteSet()
{
    //��������
    InputPasswordDialog dialog(this);
    dialog.setModal(true);
    if (dialog.exec() == QDialog::Accepted) {
        if (UI_OP_PASSWORD != dialog.getPassword()) {
            QMessageBox::warning(this, tr("����"), tr("��������ȷ�����룡"));
            return;
        }
    } else {
        return;
    }

    if (QMessageBox::Ok != QMessageBox::warning(this, tr("����"), tr("ȷ��Ҫ����Զ�����ã�"),
                                                QMessageBox::Ok | QMessageBox::Cancel)) {
        return;
    }

    if (countCheckedCkSrv() == 0) {
        QMessageBox::warning(this, tr("����"), tr("��ѡ�����ߵ�̫�����Զ���¼����"));
        return;
    }

    ui->prgBarSet->setValue(0);
    qApp->processEvents();

    //��ȡ��ѡ���cookie������
    QStringList strCkSrvList;
    int row = ui->tableCookieServer->rowCount();
    for (int i = 0; i < row; i++) {
        if (ui->tableCookieServer->item(i, 0)->checkState() == Qt::Checked) {
            strCkSrvList.append(tr("%1:%2").arg(ui->tableCookieServer->item(i, 0)->text(),
                                                ui->tableCookieServer->item(i, 1)->text()));
        }
    }

    //���ɿ�������
    int index = ui->comboBoxServer->currentIndex();
    Server *pSrv = NULL;
    CDataWare *pDw = CDataWare::instance();
    QString strErrInfo;
    QStringList strErrInfoList;

    if (0 == index) {
        //����ȫ��������
        ServerList srvList = pDw->getServerList();
        for (int i = 0; i < srvList.size(); i++) {
            setParamToServer(srvList.at(i), strCkSrvList, strErrInfo);
            strErrInfoList.append(tr("%1: %2").arg(srvList.at(i)->ip, strErrInfo));
            ui->prgBarSet->setMaximum(srvList.size());
            ui->prgBarSet->setValue(i+1);
            qApp->processEvents();
        }
    } else {
        //���õ�ǰѡ�з�����
        pSrv = pDw->getServer(ui->comboBoxServer->currentText());
        if (pSrv) {
            setParamToServer(pSrv, strCkSrvList, strErrInfo);
            strErrInfoList.append(tr("%1: %2").arg(pSrv->ip, strErrInfo));
            ui->prgBarSet->setMaximum(1);
            ui->prgBarSet->setValue(1);
        }
    }

    //��ʾ������
    strErrInfo.clear();
    for (int i = 0; i < strErrInfoList.size(); i++) {
        strErrInfo.append(tr("%1\n").arg(strErrInfoList.at(i)));
    }
    QMessageBox::information(this, tr("������"), strErrInfo);
}

//�޸�����
void TabSettingWidget::slotBtnModifySet()
{
    enableGroupBox(true);
    ui->btnLocalSet->setEnabled(true);
    ui->btnModifySet->setEnabled(false);
    ui->btnRemoteSet->setEnabled(false);
}

//��������
void TabSettingWidget::slotBtnDownloadSet()
{
    //��������
    InputPasswordDialog dialog(this);
    dialog.setModal(true);
    if (dialog.exec() == QDialog::Accepted) {
        if (UI_OP_PASSWORD != dialog.getPassword()) {
            QMessageBox::warning(this, tr("����"), tr("��������ȷ�����룡"));
            return;
        }
    } else {
        return;
    }

    if (QMessageBox::Ok != QMessageBox::warning(this, tr("����"), tr("ȷ��Ҫ����Զ��ϵͳ������"),
                                                QMessageBox::Ok | QMessageBox::Cancel)) {
        return;
    }

    //���ɿ�������
    int index = ui->comboBoxServer->currentIndex();
    Server *pSrv = NULL;
    CDataWare *pDw = CDataWare::instance();
    QString strErrInfo;
    QString strTmp;
    CSqlParam sqlParam;
    QStringList strCkList;

    //��ʼ���������
    ui->textParam->clear();
    qApp->processEvents();

    if (0 == index) {
        //��ȡȫ���������Ĳ���
        ServerList srvList = pDw->getServerList();
        for (int i = 0; i < srvList.size(); i++) {
            pSrv = srvList.at(i);
            strTmp = "��" + pSrv->ip + "��: ";
            if (getParamFromServer(pSrv, sqlParam, strCkList, strErrInfo)) {
                strTmp += tr("����\n"
                             "���ݿ����: %1:%2 �û���(%3) ���ݿ�(%4)\n"
                             "�Զ���¼��: ")
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
        //���õ�ǰѡ�з�����
        pSrv = pDw->getServer(ui->comboBoxServer->currentText());
        if (pSrv) {
            strTmp = "��" + pSrv->ip + "��: ";
            if (getParamFromServer(pSrv, sqlParam, strCkList, strErrInfo)) {
                strTmp += tr("����\n"
                             "���ݿ����: %1:%2 �û���(%3) ���ݿ�(%4)\n"
                             "�Զ���¼��: ")
                        .arg(sqlParam.strIp)
                        .arg(sqlParam.iPort)
                        .arg(sqlParam.strUser)
                        .arg(sqlParam.strDbname);
                for (int j = 0; j < strCkList.size(); j++) {
                    strTmp.append(tr("\n\t   %1: %2").arg(j+1).arg(strCkList.at(j)));
                }
            } else {
                strTmp += tr("���� %1\n").arg(strErrInfo);
            }
            ui->textParam->append(strTmp);
            qApp->processEvents();
        }
    }

    ui->textParam->append(tr("\n����Զ��ϵͳ�������"));
}

//Ĭ��
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

//����
void TabSettingWidget::slotBtnReset()
{
    ui->lineSqlIp->clear();
    ui->lineSqlPort->clear();
    ui->lineSqlUser->clear();
    ui->lineSqlPwd->clear();
    ui->lineSqlDbName->clear();
}

//���Ӳ���
void TabSettingWidget::slotBtnTestConnect()
{
    if (!isValidData()) {
        return;
    }

    //������ϸ�������
    {
        QSqlDatabase sqlDb = QSqlDatabase::addDatabase("QMYSQL", "_test_sql_");
        sqlDb.setHostName(ui->lineSqlIp->text());
        sqlDb.setDatabaseName(ui->lineSqlDbName->text());
        sqlDb.setUserName(ui->lineSqlUser->text());
        sqlDb.setPort(ui->lineSqlPort->text().toUShort());
        sqlDb.setPassword(ui->lineSqlPwd->text());
        if (sqlDb.open()) {
            QMessageBox::information(this, tr("���ӳɹ�"), tr("���ݿ����ӳɹ���"));
            sqlDb.close();
        } else {
            QMessageBox::warning(this, tr("����ʧ��"), tr("���ݿ�����ʧ�ܣ�"));
        }
    }
    QSqlDatabase::removeDatabase("_test_sql_");
}

//���
void TabSettingWidget::slotBtnAdd()
{
    //ip��ַ
    if (!CPublic::isIpAddr(ui->lineCookieIp->text())) {
        QMessageBox::warning(this, tr("����"), tr("������Ϸ���IP��ַ��"));
        ui->lineCookieIp->setFocus();
        return;
    }

    //�˿�
    if (!CPublic::isDigit(ui->lineCookiePort->text())) {
        QMessageBox::warning(this, tr("����"), tr("������Ϸ��Ķ˿ںţ�"));
        ui->lineCookiePort->setFocus();
        return;
    }

    //�ж�ip�Ƿ����
    if (findIpFromTable(ui->lineCookieIp->text()) != -1) {
        QMessageBox::warning(this, tr("����"), tr("�벻Ҫ�ظ�����Զ���¼����"));
        return;
    }

    //��ӵ��������
    appendCookieServer(ui->lineCookieIp->text(), ui->lineCookiePort->text(), Qt::Unchecked);

    ui->lineCookieIp->clear();
    ui->lineCookieIp->setFocus();
}

//ɾ��
void TabSettingWidget::slotBtnDelete()
{
    if (QMessageBox::warning(this, tr("����"), tr("�Ƿ�ȷ��ɾ����ѡ�е��Զ���¼����"), QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok) {
        return;
    }

    QList<int> intList;
    int len = ui->tableCookieServer->rowCount();

    //��ѡ�е����������ӵ�����
    QTableWidgetItem *pItem = NULL;
    for (int i = 0; i < len; i++) {
        pItem = ui->tableCookieServer->item(i, 0);
        if (pItem->checkState() == Qt::Checked) {
            intList.append(i);
        }
    }

    //�Ӻ���ǰɾ�������
    len = intList.size();
    for (int i = len - 1; i >= 0; i--) {
        ui->tableCookieServer->removeRow(intList.at(i));
    }
}

//���
void TabSettingWidget::slotBtnClear()
{
    if (QMessageBox::warning(this, tr("����"), tr("�Ƿ�ȷ����������Զ���¼����"), QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok) {
        return;
    }
    ui->tableCookieServer->clearContents();
    ui->tableCookieServer->setRowCount(0);
    ui->prgBarConfirm->setValue(0);
}

//��֤
void TabSettingWidget::slotBtnConfirm()
{
    if (countCheckedCkSrv() == 0) {
        QMessageBox::warning(this, tr("����"), tr("��ѡ��Ҫ��֤�ĵ�¼����"));
        return;
    }

    //ui׼��
    ui->prgBarConfirm->setValue(0);
    enableWidgetForConfirm(false);

    //�����߳�
    m_pChkCkSrvThread = new CCheckCookieServerThread(this, ECK_Heart);
    connect(m_pChkCkSrvThread, SIGNAL(finished()), this, SLOT(slotThreadFinished()));
    connect(m_pChkCkSrvThread, SIGNAL(signalCkDealResult(int,int,QString,bool,QString,QString)),
            this, SLOT(slotCkDealResult(int,int,QString,bool,QString,QString)));

    //��Ҫ����ip���뵽����߳�
    int len = ui->tableCookieServer->rowCount();
    QTableWidgetItem *pItem = NULL;
    for (int i = 0; i < len; i++) {
        pItem = ui->tableCookieServer->item(i, 0);
        if (pItem->checkState() == Qt::Checked) {
            ui->tableCookieServer->item(i, 2)->setText("�����");
            m_pChkCkSrvThread->addCookieServer(i,
                                               pItem->text(),
                                               ui->tableCookieServer->item(i, 1)->text().toUShort());
        }
    }
    //�����߳̿�ʼ���
    m_pChkCkSrvThread->start();
}

//������ַ
void TabSettingWidget::slotBtnSetUrl()
{
    if (countCheckedCkSrv() == 0)
    {
        QMessageBox::warning(this, tr("����"), tr("��ѡ��Ҫ��֤�ĵ�¼����"));
        return;
    }

    ChooseUrlDialog dlg(this);
    if (QDialog::Accepted == dlg.exec())
    {
        //ui׼��
        ui->prgBarConfirm->setValue(0);
        enableWidgetForConfirm(false);

        //�����߳�
        m_pChkCkSrvThread = new CCheckCookieServerThread(this, ECK_SetUrl);
        connect(m_pChkCkSrvThread, SIGNAL(finished()), this, SLOT(slotThreadFinished()));
        connect(m_pChkCkSrvThread, SIGNAL(signalCkDealResult(int,int,QString,bool,QString,QString)),
                this, SLOT(slotCkDealResult(int,int,QString,bool,QString,QString)));

        //��Ҫ����ip���뵽����߳�
        int len = ui->tableCookieServer->rowCount();
        QTableWidgetItem *pItem = NULL;
        for (int i = 0; i < len; i++) {
            pItem = ui->tableCookieServer->item(i, 0);
            if (pItem->checkState() == Qt::Checked) {
                ui->tableCookieServer->item(i, 2)->setText("������");
                m_pChkCkSrvThread->addCookieServer(i,
                                                   pItem->text(),
                                                   ui->tableCookieServer->item(i, 1)->text().toUShort());
            }
        }
        m_pChkCkSrvThread->setUrlFlag(dlg.GetUrlFlag());
        //�����߳̿�ʼ���
        m_pChkCkSrvThread->start();
    }
}

//ȫѡ
void TabSettingWidget::slotBtnSelectAll()
{
    int len = ui->tableCookieServer->rowCount();
    for (int i = 0; i < len; i++) {
        ui->tableCookieServer->item(i, 0)->setCheckState(Qt::Checked);
    }
}

//��ѡ
void TabSettingWidget::slotBtnUnselect()
{
    int len = ui->tableCookieServer->rowCount();
    QTableWidgetItem *pItem = NULL;
    for (int i = 0; i < len; i++) {
        pItem = ui->tableCookieServer->item(i, 0);
        pItem->setCheckState((pItem->checkState() == Qt::Checked) ? Qt::Unchecked : Qt::Checked);
    }
}

//��ʼ������
void TabSettingWidget::initVariable()
{

}

//��ʼ��ui
void TabSettingWidget::initSetupUi()
{
    //����
    slotServerChanged();

    //��־���ݿ����
    slotBtnReset();

    //̫�����Զ���¼��
    ui->tableCookieServer->setItemDelegate(new NoFocusDelegate(this));
    ui->tableCookieServer->verticalHeader()->setResizeMode(QHeaderView::Fixed);
    ui->tableCookieServer->setColumnWidth(0, 150);
    ui->tableCookieServer->setColumnWidth(1, 60);
    ui->tableCookieServer->clearContents();
    ui->tableCookieServer->setRowCount(0);

    //Ĭ�ϲ�ʹ�ÿؼ�
    enableGroupBox(false);

    //���水ť����
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

//��ʼ��layout
void TabSettingWidget::initLayout()
{
    //��������
    QGridLayout *pLayout1 = new QGridLayout(this);
    pLayout1->addWidget(ui->chkBoxPowerBoot, 0, 0);
    ui->grpBoxBaseSet->setLayout(pLayout1);

    //����
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

    //̫�����Զ���¼��
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

    //��־���ݿ����
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

    //ϵͳ����
    QGridLayout *pLayout6 = new QGridLayout(this);
    pLayout6->addWidget(ui->textParam, 0, 0);
    ui->grpBoxDlRemoteParam->setLayout(pLayout6);

    //���ڲ���
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

//�����Ϸ��ж�
bool TabSettingWidget::isValidData()
{
    //ip��ַ
    if (!CPublic::isIpAddr(ui->lineSqlIp->text())) {
        QMessageBox::warning(this, tr("����"), tr("������Ϸ���IP��ַ��"));
        ui->lineSqlIp->setFocus();
        return false;
    }

    //�˿�
    if (!CPublic::isDigit(ui->lineSqlPort->text())) {
        QMessageBox::warning(this, tr("����"), tr("������Ϸ��Ķ˿ںţ�"));
        ui->lineSqlPort->setFocus();
        return false;
    }

    return true;
}

//��table��Ѱ��ip,����row
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

//���ý��ÿؼ�
void TabSettingWidget::enableGroupBox(bool enabled)
{
    ui->grpBoxBaseSet->setEnabled(enabled);
    ui->grpBoxCookieServer->setEnabled(enabled);
    ui->grpBoxSqlParam->setEnabled(enabled);
}

//���ý��ÿؼ�:��֤cookie������
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

//���cookie������
void TabSettingWidget::appendCookieServer(const QString &ip, const QString &port, Qt::CheckState chk)
{
    //��ӵ��������
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

//����cookie��������ѡ�еĸ���
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

//�Ƿ��������ݿ����
bool TabSettingWidget::isSetSqlParam()
{
    return !(ui->lineSqlIp->text().isEmpty() || ui->lineSqlPort->text().isEmpty()
            || ui->lineSqlUser->text().isEmpty() || ui->lineSqlPwd->text().isEmpty()
            || ui->lineSqlDbName->text().isEmpty());
}

//������������
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

//��������Ի�
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

//���ò�����webhelper
void TabSettingWidget::setParamToServer(Server *pSrv, const QStringList &ckSrvList, QString &errInfo)
{
    QString strRecv;
    string strCmd = GenCmdRequest(CMD_SETPARAM, pSrv->ctrlPwd, ckSrvList);

    if (!talkWithServer(pSrv->ip, pSrv->srvPort, strCmd, strRecv)) {
        errInfo = tr("������������");
        return;
    }

    //����
    QStringList strList = strRecv.split('-');
    if (strList.size() != 4) {
        errInfo = tr("���ݽ���ʧ��");
        return;
    }

    //����
    int iAesLevel = strList.at(1).toInt();
    string strDecrypt;
    CAes::DecryptFromHexLevel(iAesLevel, strList.at(2).toStdString(), strDecrypt);

    //����json
    Json::Value jval;
    if (!CJson::ParseStringToJsonValue(strDecrypt, jval)) {
        errInfo = tr("���ݽ���ʧ��");
        return;
    }

    if (CJson::JsonValueToInt(jval["result"]) == 0) {
        errInfo = tr("�����������");
        return;
    }

    errInfo = tr("���óɹ�");
}

bool TabSettingWidget::getParamFromServer(Server *pSrv, CSqlParam &sqlParam, QStringList &ckSrvList, QString &errInfo)
{
    QString strRecv;
    string strCmd = GenCmdRequest(CMD_GETPARAM, pSrv->ctrlPwd, ckSrvList);

    if (!talkWithServer(pSrv->ip, pSrv->srvPort, strCmd, strRecv)) {
        errInfo = tr("������������");
        return false;
    }

    //����
    QStringList strList = strRecv.split('-');
    if (strList.size() != 4) {
        errInfo = tr("���ݽ���ʧ��");
        return false;
    }

    //����
    int iAesLevel = strList.at(1).toInt();
    string strDecrypt;
    CAes::DecryptFromHexLevel(iAesLevel, strList.at(2).toStdString(), strDecrypt);

    //����json
    Json::Value jval;
    if (!CJson::ParseStringToJsonValue(strDecrypt, jval)) {
        errInfo = tr("���ݽ���ʧ��");
        return false;
    }

    if (CJson::JsonValueToInt(jval["result"]) == 0) {
        errInfo = tr("�����������");
        return false;
    }

    //��json�����а����ݽ�������
    Json::Value jsql = jval["info"]["sql"];
    Json::Value jck = jval["info"]["cookiesrv"];

    if (!jsql.isObject() || !jck.isArray()) {
        errInfo = tr("Json���ݽ���ʧ��");
        return false;
    }

    //�������ݿ���Ϣ
    sqlParam.strIp = CJson::JsonValueToStringEx(jsql["ip"]).c_str();
    sqlParam.strUser = CJson::JsonValueToStringEx(jsql["user"]).c_str();
    sqlParam.strPwd = CJson::JsonValueToStringEx(jsql["pwd"]).c_str();
    sqlParam.strDbname = CJson::JsonValueToStringEx(jsql["dbname"]).c_str();
    sqlParam.iPort = CJson::JsonValueToInt(jsql["port"]);

    //����cookie��Ϣ
    ckSrvList.clear();
    for (uint i = 0; i < jck.size(); i++) {
        ckSrvList.append(CJson::JsonValueToStringEx(jck[i]).c_str());
    }

    return true;
}

//��ȡxml�ļ�
void TabSettingWidget::readXml()
{
    //����xml�ļ�
    QString strFileName = qApp->applicationDirPath() + '/' + SET_FILE;
    QFile file(strFileName);

    QDomDocument doc("setting");

    //����
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

    //��ȡ��������
    QDomElement elementBaseSet = elementTabSetting.elementsByTagName("baseset").at(0).toElement();
    iRes = elementBaseSet.elementsByTagName("powerboot").at(0).toElement().text().toInt();
    ui->chkBoxPowerBoot->setCheckState((Qt::CheckState)iRes);

    //��ȡ���ݿ����
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

    //�������ݿ����
    CDataWare::instance()->setLogSqlParam(ui->lineSqlIp->text(), ui->lineSqlUser->text(),
                                          ui->lineSqlPwd->text(), ui->lineSqlDbName->text(),
                                          ui->lineSqlPort->text().toUShort());

    //��ȡcookie������
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
        //��ӵ�ui
        appendCookieServer(strIp, strPort, (Qt::CheckState)iRes);
    }
}

//д��xml�ļ�
void TabSettingWidget::writeXml()
{
    //����xml�ļ�
    QString strFileName = qApp->applicationDirPath() + '/' + SET_FILE;
    QFile file(strFileName);

    QDomDocument doc("setting");
    QDomNode root;
    if (QFile::exists(strFileName)) {
        //����
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return;
        }
        if (!doc.setContent(&file)) {
            file.close();
            return;
        }
        file.close();
        //ɾ��tabsetting��ǩ
        root = doc.childNodes().at(1);
        QDomNode node = doc.elementsByTagName("tabsetting").at(0);
        root.removeChild(node);//ɾ��
    } else {
        //������
        QDomNode xmlHeader = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
        doc.appendChild(xmlHeader);

        root = doc.createElement("setting");
        doc.appendChild(root);
    }

    //����tabserver��ǩ
    string strEncrypt;
    QDomNode node;
    QDomNode nodeTabSetting = doc.createElement("tabsetting");
    root.appendChild(nodeTabSetting);

    //�����������
    QDomNode nodeBaseSet = doc.createElement("baseset");
    nodeTabSetting.appendChild(nodeBaseSet);

    node = doc.createElement("powerboot");
    node.appendChild(doc.createTextNode(QString::number(ui->chkBoxPowerBoot->checkState())));
    nodeBaseSet.appendChild(node);

    //�������ݿ����
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

    //����cookie������
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

    //����Ϊxml
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return;
    }
    QTextStream textStream(&file);
    doc.save(textStream, 2);
    file.close();
}

