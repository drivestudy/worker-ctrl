#include "tablogwidget.h"
#include "ui_tablogwidget.h"
#include <cpublic.h>
#include <selectcolumndialog.h>
#include <cdataware.h>
#include <inputpassworddialog.h>
#include <nofocusdelegate.h>

TabLogWidget::TabLogWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabLogWidget)
{
    ui->setupUi(this);

    initVariable();
    initLayout();
    initSetupUi();
}

TabLogWidget::~TabLogWidget()
{
    if (m_pSltColAction) {
        delete m_pSltColAction;
        m_pSltColAction = NULL;
    }

    delete ui;
//    DEBUG_FUNCTION
}

/*****slot*****/

//��ѯ��ť
void TabLogWidget::slotBtnSearch()
{
    if (!isValidData()) {
        return;
    }

    //��ӷ�������������
    addCurrentServerIp();

    QSqlDatabase sqlDb;
    //�ж����ݿ�����Ƿ����
    if (!CDataWare::instance()->getLogSqlDb(sqlDb)) {
        QMessageBox::warning(this, tr("����"), tr("��������ȷ�����ݿ������"));
        return;
    }

    //׼������
    setCtrlButtonEnabled(false);
    ui->tableSearchResult->setSortingEnabled(false);
    if (sqlDb.open()) {
        QSqlQuery query = sqlDb.exec(createSqlCmdSearch());
        if (query.lastError().isValid()) {
            //����
            QMessageBox::warning(this, tr("����"),
                                 tr("��־��ѯʧ��: %1").arg(query.lastError().text()));
        } else {
            //���ý����ui
            setSqlResultToTable(query);
            query.clear();
        }
        sqlDb.close();
    } else {
        QMessageBox::warning(this, tr("����"),
                             tr("���ݿ��ʧ��: %1").arg(sqlDb.lastError().text()));
    }

    //��������
    setCtrlButtonEnabled(true);
    ui->tableSearchResult->setSortingEnabled(true);
    ui->tableSearchResult->sortByColumn(23, Qt::AscendingOrder);//��ʼʱ������
}

//ɾ����ť
void TabLogWidget::slotBtnDelete()
{
    if (!isValidData()) {
        return;
    }

    //��ӷ�������������
    addCurrentServerIp();

    QSqlDatabase sqlDb;
    //�ж����ݿ�����Ƿ����
    if (!CDataWare::instance()->getLogSqlDb(sqlDb)) {
        QMessageBox::warning(this, tr("����"), tr("��������ȷ�����ݿ������"));
        return;
    }

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

    if (QMessageBox::Ok == QMessageBox::warning(this, tr("����"), tr("����ɾ�����޷��ָ����������"), QMessageBox::Ok | QMessageBox::Cancel)) {
        setCtrlButtonEnabled(false);
        if (sqlDb.open()) {
            QSqlQuery query = sqlDb.exec(createSqlCmdRemove());
            if (query.lastError().isValid()) {
                //����
                QMessageBox::warning(this, tr("����"),
                                     tr("��־ɾ��ʧ��: %1").arg(query.lastError().text()));
            } else {
                query.clear();
            }
            sqlDb.close();
        } else {
            QMessageBox::warning(this, tr("����"),
                                 tr("���ݿ��ʧ��: %1").arg(sqlDb.lastError().text()));
        }
        setCtrlButtonEnabled(true);
    }
}

//������ť
void TabLogWidget::slotBtnCount()
{
    if (!isValidData()) {
        return;
    }

    //��ӷ�������������
    addCurrentServerIp();

    QSqlDatabase sqlDb;
    //�ж����ݿ�����Ƿ����
    if (!CDataWare::instance()->getLogSqlDb(sqlDb)) {
        QMessageBox::warning(this, tr("����"), tr("��������ȷ�����ݿ������"));
        return;
    }

    //׼������
    setCtrlButtonEnabled(false);
    if (sqlDb.open()) {

        QSqlQuery query = sqlDb.exec(createSqlCmdCount());
        if (query.lastError().isValid()) {
            //����
            QMessageBox::warning(this, tr("����"),
                                 tr("��־��ѯʧ��: %1").arg(query.lastError().text()));
        } else {
            //���ý����ui
            if (query.next()) {
                int count = query.value(0).toInt();
                QMessageBox::information(this, tr("�������"),
                                         tr("��־���ݿ⺬��%1��ƥ���¼��").arg(count));
            }
            query.clear();
        }
        sqlDb.close();
    } else {
        QMessageBox::warning(this, tr("����"),
                             tr("���ݿ��ʧ��: %1").arg(sqlDb.lastError().text()));
    }

    //��������
    setCtrlButtonEnabled(true);
}

//����
void TabLogWidget::slotBtnReset()
{
    //�ӷ������б��뵽�����Ͽ�
    ui->comboBoxServerIp->clear();
    ui->comboBoxServerIp->addItems(CDataWare::instance()->getServerIpList());
    ui->comboBoxServerIp->clearEditText();

    ui->lineMaster->clear();
    ui->lineOpUser->clear();
    ui->lineClientIp->clear();
    ui->dateTimeFrom->setDateTime(QDateTime(QDate(2013, 1, 1), QTime(0, 0, 0)));
    ui->dateTimeTo->setDateTime(QDateTime::currentDateTime());
    ui->comboBoxUserType->setCurrentIndex(0);
    ui->comboBoxOpType->setCurrentIndex(0);
    ui->comboBoxResultMethod->setCurrentIndex(0);
    ui->comboBoxTaskResult->setCurrentIndex(0);
    ui->comboBoxErrorType->setCurrentIndex(0);
}

//��ղ�ѯ�����ť
void TabLogWidget::slotBtnClearResult()
{
    if (QMessageBox::Ok == QMessageBox::warning(this, tr("����"), tr("ȷ��Ҫ��ղ�ѯ�����"), QMessageBox::Ok | QMessageBox::Cancel)) {
        clearSearchResult();
    }
}

//��ѯ���˫���ź�
void TabLogWidget::slotItemDoubleClicked(QTableWidgetItem *item)
{
    if (item) {
        qApp->clipboard()->setText(item->text());
    }
}

//ˮƽ��ͷ���Ҽ��˵�����
void TabLogWidget::slotCustomContextMenuRequested(const QPoint &pos)
{
    m_tableMenu.exec(QCursor::pos());
}

//ѡ����Action
void TabLogWidget::slotSltColAction()
{
    SelectColumnDialog scd(this);
    QList<int> intList;

    //�������ļ���ȡ����
    readXml(intList);
    scd.setSelectColumn(intList);

    if (QDialog::Accepted == scd.exec()) {
        intList = scd.getSelectColumn();
        int minLen = (intList.size() <= ui->tableSearchResult->columnCount()) ?
                    intList.size() : ui->tableSearchResult->columnCount();

        for (int i = 0; i < minLen; i++) {
            ui->tableSearchResult->setColumnHidden(i, ((Qt::CheckState)intList.at(i) != Qt::Checked));
        }

        //д�������ļ�
        writeXml(intList);
    }
}

/*****slot*****/

//��ʼ������
void TabLogWidget::initVariable()
{
    m_pSltColAction = NULL;
}

//��ʼ��ui
void TabLogWidget::initSetupUi()
{
    /**���**/
    ui->tableSearchResult->setItemDelegate(new NoFocusDelegate(this));
    //���ͷ��
    ui->tableSearchResult->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableSearchResult->horizontalHeader(), SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotCustomContextMenuRequested(QPoint)));

    //�Ҽ��˵�
    m_pSltColAction = new QAction(tr("ѡ����..."), this);
    connect(m_pSltColAction, SIGNAL(triggered()), this, SLOT(slotSltColAction()));
    m_tableMenu.addAction(m_pSltColAction);

    ui->tableSearchResult->clearContents();
    ui->tableSearchResult->setRowCount(0);
    ui->tableSearchResult->resizeColumnsToContents();
    ui->tableSearchResult->verticalHeader()->setResizeMode(QHeaderView::Fixed);

    //���ñ���С
    setTableColumnWidth();

    //���˫���¼�
    connect(ui->tableSearchResult, SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
            this, SLOT(slotItemDoubleClicked(QTableWidgetItem*)));
    /**���**/

    /**��ť**/
    connect(ui->btnSearch, SIGNAL(clicked()), this, SLOT(slotBtnSearch()));
    connect(ui->btnDelete, SIGNAL(clicked()), this, SLOT(slotBtnDelete()));
    connect(ui->btnCount, SIGNAL(clicked()), this, SLOT(slotBtnCount()));
    connect(ui->btnReset, SIGNAL(clicked()), this, SLOT(slotBtnReset()));
    connect(ui->btnClearResult, SIGNAL(clicked()), this, SLOT(slotBtnClearResult()));
    /**��ť**/

    /**��ѯ����**/
    //��ʼ����־��ѯ����
    slotBtnReset();
    /**��ѯ����**/

    //���ò�ѯ�����
    //�������ļ���ȡ����
    QList<int> intList;
    readXml(intList);
    int minLen = (intList.size() <= ui->tableSearchResult->columnCount()) ?
                intList.size() : ui->tableSearchResult->columnCount();

    for (int i = 0; i < minLen; i++) {
        ui->tableSearchResult->setColumnHidden(i, ((Qt::CheckState)intList.at(i) != Qt::Checked));
    }
}

//��ʼ��layout
void TabLogWidget::initLayout()
{
    //��־��ѯ����
    QGridLayout *pLayout1 = new QGridLayout(this);
    pLayout1->addWidget(ui->labelA0,                0, 0);
    pLayout1->addWidget(ui->comboBoxServerIp,       0, 1);
    pLayout1->addWidget(ui->labelA1,                0, 2);
    pLayout1->addWidget(ui->lineMaster,             0, 3);
    pLayout1->addWidget(ui->labelA2,                0, 4);
    pLayout1->addWidget(ui->lineOpUser,             0, 5);

    pLayout1->addWidget(ui->labelB0,                1, 0);
    pLayout1->addWidget(ui->lineClientIp,           1, 1);
    pLayout1->addWidget(ui->labelB1,                1, 2);
    pLayout1->addWidget(ui->dateTimeFrom,           1, 3);
    pLayout1->addWidget(ui->labelB2,                1, 4);
    pLayout1->addWidget(ui->dateTimeTo,             1, 5);

    pLayout1->addWidget(ui->labelC0,                2, 0);
    pLayout1->addWidget(ui->comboBoxUserType,       2, 1);
    pLayout1->addWidget(ui->labelC1,                2, 2);
    pLayout1->addWidget(ui->comboBoxOpType,         2, 3);
    pLayout1->addWidget(ui->labelC2,                2, 4);
    pLayout1->addWidget(ui->comboBoxResultMethod,   2, 5);

    pLayout1->addWidget(ui->labelD0,                3, 0);
    pLayout1->addWidget(ui->comboBoxTaskResult,     3, 1);
    pLayout1->addWidget(ui->labelD1,                3, 2);
    pLayout1->addWidget(ui->comboBoxErrorType,      3, 3, 1, 3);

    ui->grpBoxLog->setLayout(pLayout1);

    //���ݿ����
    QHBoxLayout *pLayout2 = new QHBoxLayout(this);
    pLayout2->addWidget(ui->btnSearch);
    pLayout2->addWidget(ui->btnDelete);
    pLayout2->addWidget(ui->btnCount);
    pLayout2->addWidget(ui->btnReset);
    pLayout2->addWidget(ui->btnClearResult);
    pLayout2->addStretch(1);
    ui->grpBoxSqlOp->setLayout(pLayout2);

    //��ѯ���
    QGridLayout *pLayout3 = new QGridLayout(this);
    pLayout3->addWidget(ui->tableSearchResult, 0, 0);
    ui->grpBoxSearchResult->setLayout(pLayout3);

    //���ô��ڵĲ���
    QGridLayout *pLayout = new QGridLayout(this);
    pLayout->addWidget(ui->grpBoxLog, 0, 0);
    pLayout->addWidget(ui->grpBoxSqlOp, 1, 0);
    pLayout->addWidget(ui->grpBoxSearchResult, 2, 0);
    pLayout->setRowStretch(0, 2);
    pLayout->setRowStretch(1, 1);
    pLayout->setRowStretch(2, 10);
    setLayout(pLayout);
}

//�����Ϸ��ж�
bool TabLogWidget::isValidData()
{
    //������ip��ַ
//    if (!ui->lineServerIp->text().isEmpty() && !CPublic::isIpAddr(ui->lineServerIp->text())) {
//        QMessageBox::warning(this, tr("����"), tr("������Ϸ��ķ�������ַ��"));
//        ui->lineServerIp->setFocus();
//        return false;
//    }
    if (!ui->comboBoxServerIp->currentText().isEmpty()
            && !CPublic::isIpAddr(ui->comboBoxServerIp->currentText())) {
        QMessageBox::warning(this, tr("����"), tr("������Ϸ��ķ�������ַ��"));
        ui->comboBoxServerIp->setFocus();
        return false;
    }

    //�ͻ���ip��ַ
    if (!ui->lineClientIp->text().isEmpty() && !CPublic::isIpAddr(ui->lineClientIp->text())) {
        QMessageBox::warning(this, tr("����"), tr("������Ϸ��Ŀͻ��˵�ַ��"));
        ui->lineClientIp->setFocus();
        return false;
    }

    //ʱ���
    if (ui->dateTimeFrom->dateTime() >= ui->dateTimeTo->dateTime()) {
        QMessageBox::warning(this, tr("����"), tr("����ʱ�䲻�ܱȿ�ʼʱ���磡"));
        ui->dateTimeFrom->setFocus();
        return false;
    }

    return true;
}

//���ݽ����ṩ������������Ӧ��sql���,��ѯ
//��ѯ���ֶ�˳��������ѯ���QTableWidget�ı�ͷ˳��һ��
QString TabLogWidget::createSqlCmdSearch()
{
    return ("select token,server_ip,optype,child_task,master,opuser,user_type,"
            "user_point,draw_deposit_point,seq_num,page_num,home_url,cb_url,"
            "result_method,html_responce,err_file,err_function,err_line,"
            "html_filepath,err_info,curl_err_info,client_ip,client_port,"
            "start_time,stop_time,use_time,result from webhelper_log"
            + createSqlCondition());
}

//���ݽ����ṩ������������Ӧ��sql���,����
QString TabLogWidget::createSqlCmdCount()
{
    return ("select count(*) from webhelper_log" + createSqlCondition());
}

//���ݽ����ṩ������������Ӧ��sql���,ɾ��
QString TabLogWidget::createSqlCmdRemove()
{
    return ("delete from webhelper_log" + createSqlCondition());
}

//ƴ����������
QString TabLogWidget::createSqlCondition()
{
    QString strCond, strTmp, strElement;
    QStringList strList;

    //������
    strTmp = ui->comboBoxServerIp->currentText();
    if (!strTmp.isEmpty()) {
        strElement = tr("server_ip='%1'").arg(strTmp);
        strList.append(strElement);
    }

    //���˺�
    strTmp = ui->lineMaster->text();
    if (!strTmp.isEmpty()) {
        strElement = tr("master='%1'").arg(strTmp);
        strList.append(strElement);
    }

    //�����û�
    strTmp = ui->lineOpUser->text();
    if (!strTmp.isEmpty()) {
        strElement = tr("opuser='%1'").arg(strTmp);
        strList.append(strElement);
    }

    //�ͻ���
    strTmp = ui->lineClientIp->text();
    if (!strTmp.isEmpty()) {
        strElement = tr("client_ip='%1'").arg(strTmp);
        strList.append(strElement);
    }

    //��ʼʱ�� -> ��ʼʱ��
    uint fromDate = 0, toDate = 0;
    fromDate = ui->dateTimeFrom->dateTime().toTime_t();
    toDate = ui->dateTimeTo->dateTime().toTime_t();

    strElement = tr("start_time>=%1").arg(fromDate);
    strList.append(strElement);
    strElement = tr("start_time<=%1").arg(toDate);
    strList.append(strElement);

    //�û�����:ȫ�� ��Ա ����
    int index = ui->comboBoxUserType->currentIndex();
    if (0 != index) {
        //ѡ��Ĳ���'ȫ��'
        strElement = tr("user_type=%1").arg(index);
        strList.append(strElement);
    }

    //��������:ȫ�� xx  xx xx ...
    index = ui->comboBoxOpType->currentIndex();
    if (0 != index) {
        //ѡ��Ĳ���'ȫ��'
        strElement = tr("optype=%1").arg(index);
        strList.append(strElement);
    }

    //�����ȡ��ʽ:ȫ�� get post
    index = ui->comboBoxResultMethod->currentIndex();
    if (0 != index) {
        //ѡ��Ĳ���'ȫ��'
        if (1 == index) {
            strTmp = "get";
        } else if (2 == index) {
            strTmp = "post";
        }
        strElement = tr("result_method='%1'").arg(strTmp);
        strList.append(strElement);
    }

    //ִ�н��:ȫ�� �ɹ� ʧ��
    index = ui->comboBoxTaskResult->currentIndex();
    if (0 != index) {
        //ѡ��Ĳ���'ȫ��'
        if (1 == index) {
            index = 1;//�ɹ�
        } else if (2 == index) {
            index = 0;//ʧ��
        }
        strElement = tr("result=%1").arg(index);
        strList.append(strElement);
    }

    //��������:
    /*
    0 : "ȫ��"
    1 : "0.�ɹ�"
    2 : "1.ʧ��"
    3 : "2.���ӵ�cookie������ʧ��"
    4 : "3.�����û�������ʧ��"
    5 : "4.����cookieʧ��"
    6 : "5.cookie���������ص���Ϣ��ʽ����"
    7 : "6.AES�������ݸ�ʽ����"
    8 : "7.̫���ǵ�¼ʧ��"
    9 : "8.̫��������ά��"
    10 : "9.cookie����(System Error)"
    11 : "10.cookie����(404 - File or directory not found)"
    12 : "11.cookie����(This page uses frames)"
    13 : "12.cookie����(The requested URL could not be retrieved)"
    14 : "13.cookie����(̫���Ǵ���)"
    15 : "14.json��ʽ����"
    16 : "15.����ҳ��ʧ��"
    17 : "16.�ύ����ʧ��"
    18 : "17.curl��ʼ��ʧ��"
    19 : "18.д�ļ�ʧ��"
    20 : "19.ƥ�䲻������"
    21 : "20.�Ҳ��������û�"
    22 : "21.�������ʹ���"
    23 : "22.�û����ʹ���"
    24 : "23.�û�/Ͷע״̬����"
    25 : "24.�û�����Ϊ��"
    26 : "25.Ψһ��Ϊ��"
    27 : "26.�Զ���(��ҳ���صĴ�����Ϣ)"
    28 : "27.��ѯ���������Ϊ��"
    29 : "28.���ݿ��������"
    30 : "29.���ݿ��ʼ��ʧ��"
    31 : "30.���ݿ�����ʧ��"
    32 : "31.ע��ʧ��,���������û�"
    33 : "32.���ݿ�����ִ��ʧ��"
    34 : "33.��ָ�����"
    35 : "-1.δ֪����"
    */
    index = ui->comboBoxErrorType->currentIndex();
    if (0 != index) {
        //ѡ��Ĳ���'ȫ��'

        if (35 == index) {
            //δ֪����:-1
            index = -1;
        } else {
            index -= 1;
        }
        strElement = tr("err_info like '%#%1'").arg(index);
        strList.append(strElement);
    }

    //���������ٴ���
    if (!strList.isEmpty()) {
        strCond = " where ";
        for (int i = 0; i < strList.size()-1; i++) {
            strCond.append(strList.at(i));
            strCond.append(" and ");
        }
        strCond.append(strList.last());
    }

    return strCond;
}

//�������ݿ��ѯ�����ui
void TabLogWidget::setSqlResultToTable(QSqlQuery &query)
{
    int row = 0, cols = 0, recordCount = 0, index = 1, count = 0;
    QTableWidgetItem *pItem = NULL;
    QString strTmp, strResult;
    CDataWare *pDw = CDataWare::instance();

    //��վ�����
    clearSearchResult();

    //��¼����
    recordCount = query.size();

    //ȡ������
    QSqlRecord sqlRecord = query.record();
    cols = sqlRecord.count();

    //����groupBox����
    ui->grpBoxSearchResult->setTitle(tr("��ѯ���(0/%2)").arg(recordCount));

    while (query.next()) {
        //����һ��
        row = ui->tableSearchResult->rowCount();
        ui->tableSearchResult->insertRow(row);

        //�����ֶ���Ҫת��
        for (int i = 0; i < cols; i++) {
            strTmp = query.value(i).toString();
            switch (i) {
            case 2:
                //optype
                strResult = pDw->optypeString(strTmp.toUInt());
                break;
            case 3:
                //child_task
                strResult = (1 == strTmp.toInt()) ? tr("��") : tr("��");
                break;
            case 6:
                //user_type
                strResult = usertypeString(strTmp.toUInt());
                break;
            case 23:
            case 24:
                //start_time stop_time
                strResult = QDateTime::fromTime_t(strTmp.toUInt()).toString("yyyy-MM-dd hh:mm:ss");
                break;
            case 26:
                //result
                strResult = (1 == strTmp.toInt()) ? tr("�ɹ�") : tr("ʧ��");
                break;
            default:
                strResult = strTmp;
                break;
            }

            pItem = new QTableWidgetItem(strResult);
            if (0 != i) {
                pItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
            }
            ui->tableSearchResult->setItem(row, i, pItem);
        }

        //��ʱ��ʾ������ӵ���
//        ui->tableSearchResult->scrollToBottom();

        //����groupBox����
        ui->grpBoxSearchResult->setTitle(tr("��ѯ���(%1/%2)").arg(index++).arg(recordCount));

        //���ϸ���ui
        if (count++ % 200 == 0) {
            qApp->processEvents();
        }
    }
}

//�û�����ת��
QString TabLogWidget::usertypeString(uint type)
{
    static QString arr[] = {"��", "��Ա", "����"};

    if (type < sizeof(arr)/sizeof(arr[0])) {
        return arr[type];
    } else {
        return "";
    }
}

//���ñ���д�С
void TabLogWidget::setTableColumnWidth()
{
//    static int widths[] = {
//        290, 120, 110, 50, 90, 90, 60, 70, 90, 50, 50, 170,
//        370, 85, 350, 360, 110, 60, 520, 380, 120, 70, 130,
//        130, 60, 60};
    static int widths[] = {
        100, 100, 100, 50, 90, 90, 60, 70, 90, 50, 50, 170,
        70, 85, 80, 70, 70, 60, 80, 310, 100, 100, 70, 130,
        130, 60, 60
    };
    int len = sizeof(widths)/sizeof(widths[0]);
    for (int i = 0; i < len; i++) {
        ui->tableSearchResult->setColumnWidth(i, widths[i]);
    }
}

//���ư�ť�����������
void TabLogWidget::setCtrlButtonEnabled(bool enabled)
{
    ui->btnSearch->setEnabled(enabled);
    ui->btnDelete->setEnabled(enabled);
    ui->btnCount->setEnabled(enabled);
    ui->btnReset->setEnabled(enabled);
    ui->btnClearResult->setEnabled(enabled);
    qApp->processEvents();
}

//��ղ�ѯ���
void TabLogWidget::clearSearchResult()
{
    ui->tableSearchResult->clearContents();
    ui->tableSearchResult->setRowCount(0);
    ui->tableSearchResult->horizontalScrollBar()->setValue(0);
    //���ñ���С
    setTableColumnWidth();
    ui->grpBoxSearchResult->setTitle(tr("��ѯ���"));
}

//��ȡxml�����ļ�
void TabLogWidget::readXml(QList<int> &intList)
{
    //����xml�ļ�
    QString strFileName = qApp->applicationDirPath() + '/' + SET_FILE;
    QFile file(strFileName);

    //Ĭ����ʾ������
    for (int i = 0; i < ui->tableSearchResult->columnCount(); i++) {
        intList.append((int)Qt::Checked);
    }

    QDomDocument doc("setting");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    if (!doc.setContent(&file)) {
        file.close();
        return;
    }
    file.close();

    QDomElement elementTabLog = doc.elementsByTagName("tablog").at(0).toElement();
    QDomElement elementResCol = elementTabLog.elementsByTagName("resultcolumn").at(0).toElement();

    if (!elementResCol.isNull()) {
        QStringList strList = elementResCol.text().split('-');

        intList.clear();
        for (int i = 0; i < strList.size(); i++) {
            intList.append(strList.at(i).toInt());
        }
    }
}

//д��xml�����ļ�
void TabLogWidget::writeXml(const QList<int> &intList)
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
        //ɾ��tablog��ǩ
        root = doc.childNodes().at(1);
        QDomNode node = doc.elementsByTagName("tablog").at(0);
        root.removeChild(node);//ɾ��
    } else {
        //������
        QDomNode xmlHeader = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
        doc.appendChild(xmlHeader);

        root = doc.createElement("setting");
        doc.appendChild(root);
    }

    //����tablog��ǩ
    QDomNode nodeTabLog = doc.createElement("tablog");
    root.appendChild(nodeTabLog);

    QString strResCol;
    for (int i = 0; i < intList.size(); i++) {
        strResCol.append(tr("%1-").arg(intList.at(i)));
    }
    int len = strResCol.size();
    if (strResCol.at(len-1) == '-') {
        strResCol.remove(len-1, 1);
    }

    QDomNode node = doc.createElement("resultcolumn");
    node.appendChild(doc.createTextNode(strResCol));
    nodeTabLog.appendChild(node);

    //����Ϊxml
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return;
    }
    QTextStream textStream(&file);
    doc.save(textStream, 2);
    file.close();
}

//��ӵ�ǰ��������������
void TabLogWidget::addCurrentServerIp()
{
    QString strCurrentText = ui->comboBoxServerIp->currentText();
    if (!strCurrentText.isEmpty()) {
        if (ui->comboBoxServerIp->findText(strCurrentText) == -1) {
            ui->comboBoxServerIp->addItem(strCurrentText);
        }
    }
}

