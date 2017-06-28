#include "tabranklistwidget.h"
#include "ui_tabranklistwidget.h"
#include <cdataware.h>
#include <nofocusdelegate.h>

TabRankListWidget::TabRankListWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabRankListWidget)
{
    ui->setupUi(this);

    initVariable();
    initLayout();
    initSetupUi();
}

TabRankListWidget::~TabRankListWidget()
{
    delete ui;
}

/**slot**/

//����
void TabRankListWidget::slotBtnRank()
{
    //ʱ���
    if (ui->dateTimeFrom->dateTime() >= ui->dateTimeTo->dateTime()) {
        QMessageBox::warning(this, tr("����"), tr("����ʱ�䲻�ܱȿ�ʼʱ���磡"));
        ui->dateTimeFrom->setFocus();
        return;
    }

    int index = ui->comboBoxRankInfo->currentIndex();

    QSqlDatabase sqlDb;
    //�ж����ݿ�����Ƿ����
    if (!CDataWare::instance()->getLogSqlDb(sqlDb)) {
        QMessageBox::warning(this, tr("����"), tr("��������ȷ�����ݿ������"));
        return;
    }

    setCtrlButtonEnabled(false);
    if (sqlDb.open()) {
        int totalRow = 0;
        QSqlQuery query;

        //��ѯ����
        query = sqlDb.exec(tr("select count(*) from webhelper_log where start_time>=%1 and start_time<=%2")
                           .arg(ui->dateTimeFrom->dateTime().toTime_t())
                           .arg(ui->dateTimeTo->dateTime().toTime_t()));
//        query = sqlDb.exec("select count(*) from webhelper_log");
        if (query.lastError().isValid()) {
            //����
            QMessageBox::warning(this, tr("����"),
                                 tr("��ѯ����ʧ��: %1").arg(query.lastError().text()));
            sqlDb.close();
            setCtrlButtonEnabled(true);
            return;
        } else {
            if (query.next()) {
                totalRow = query.value(0).toInt();
            }
            query.clear();
        }

        //��ѯ���н��
        query = sqlDb.exec(createSqlCmd(index));
        if (query.lastError().isValid()) {
            //����
            QMessageBox::warning(this, tr("����"),
                                 tr("��ѯ���н��ʧ��: %1").arg(query.lastError().text()));
        } else {
            setSqlResultToUi(index, totalRow, query);
            query.clear();
        }
        sqlDb.close();
    } else {
        QMessageBox::warning(this, tr("����"),
                             tr("���ݿ��ʧ��: %1").arg(sqlDb.lastError().text()));
    }
    setCtrlButtonEnabled(true);
}

//����
void TabRankListWidget::slotBtnReset()
{
    ui->comboBoxRankInfo->setCurrentIndex(0);
    ui->dateTimeFrom->setDateTime(QDateTime(QDate(2013, 1, 1), QTime(0, 0, 0)));
    ui->dateTimeTo->setDateTime(QDateTime::currentDateTime());
    ui->lineOpUser->clear();
}

//��հ�
void TabRankListWidget::slotBtnClearBoard()
{
    if (QMessageBox::Ok == QMessageBox::warning(this, tr("����"), tr("ȷ��Ҫ��հ񵥣�"), QMessageBox::Ok | QMessageBox::Cancel)) {
        ui->lineA0->clear();
        ui->lineA1->clear();
        ui->lineA2->clear();
        ui->lineA3->clear();
        ui->lineA4->clear();

        ui->lineB0->clear();
        ui->lineB1->clear();
        ui->lineB2->clear();
        ui->lineB3->clear();
        ui->lineB4->clear();

        ui->lineC0->clear();
        ui->lineC1->clear();
        ui->lineC2->clear();
        ui->lineC3->clear();
        ui->lineC4->clear();
    }
}

//������а�
void TabRankListWidget::slotBtnClearRankList()
{
    if (QMessageBox::Ok == QMessageBox::warning(this, tr("����"), tr("ȷ��Ҫ������а�"), QMessageBox::Ok | QMessageBox::Cancel)) {
        ui->tableRankList->clearContents();
        ui->tableRankList->setRowCount(0);
        setTableColumnWidth();
    }
}

//������Ϣ������
void TabRankListWidget::slotCurrentIndexChanged(int index)
{
    //ֻ�����һ���Ż�ʹ�õ������û�����
    ui->lineOpUser->setEnabled(m_iRankIndex == index);
}

//˫�����Ƶ�Ԫ��
void TabRankListWidget::slotItemDoubleClicked(QTableWidgetItem *item)
{
    if (item) {
        qApp->clipboard()->setText(item->text());
    }
}

/**slot**/

//��ʼ������
void TabRankListWidget::initVariable()
{

}

//��ʼ��ui
void TabRankListWidget::initSetupUi()
{
    /*���������Ϣ*/
    ui->lineA0->clear();
    ui->lineA1->clear();
    ui->lineA2->clear();
    ui->lineA3->clear();
    ui->lineA4->clear();

    ui->lineB0->clear();
    ui->lineB1->clear();
    ui->lineB2->clear();
    ui->lineB3->clear();
    ui->lineB4->clear();

    ui->lineC0->clear();
    ui->lineC1->clear();
    ui->lineC2->clear();
    ui->lineC3->clear();
    ui->lineC4->clear();

    ui->lineOpUser->setEnabled(false);
    slotBtnReset();

    //���
    setTableColumnWidth();
    ui->tableRankList->verticalHeader()->setResizeMode(QHeaderView::Fixed);
    ui->tableRankList->setItemDelegate(new NoFocusDelegate(this));
    /*���������Ϣ*/

    /*slot*/
    connect(ui->btnRank, SIGNAL(clicked()), this, SLOT(slotBtnRank()));
    connect(ui->btnReset, SIGNAL(clicked()), this, SLOT(slotBtnReset()));
    connect(ui->btnClearBoard, SIGNAL(clicked()), this, SLOT(slotBtnClearBoard()));
    connect(ui->btnClearRankList, SIGNAL(clicked()), this, SLOT(slotBtnClearRankList()));
    connect(ui->comboBoxRankInfo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotCurrentIndexChanged(int)));
    connect(ui->tableRankList, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(slotItemDoubleClicked(QTableWidgetItem*)));
    /*slot*/
}

//��ʼ��layout
void TabRankListWidget::initLayout()
{
    //��ʼ��layout:��������
    QGridLayout *pLayout1 = new QGridLayout(this);
    pLayout1->addWidget(ui->labelA0,            0,  0);
    pLayout1->addWidget(ui->comboBoxRankInfo,   0,  1);
    pLayout1->addWidget(ui->labelA1,            0,  2);
    pLayout1->addWidget(ui->dateTimeFrom,       0,  3);
    pLayout1->addWidget(ui->labelA2,            0,  4);
    pLayout1->addWidget(ui->dateTimeTo,         0,  5);
    pLayout1->addWidget(ui->labelA3,            0,  6);
    pLayout1->addWidget(ui->lineOpUser,         0,  7);
    pLayout1->setColumnStretch(1, 1);
    pLayout1->setColumnStretch(3, 1);
    pLayout1->setColumnStretch(5, 1);
    pLayout1->setColumnStretch(7, 1);
    ui->grpBoxRankCond->setLayout(pLayout1);

    //��ʼ��layout:����
    QHBoxLayout *pLayout2 = new QHBoxLayout(this);
    pLayout2->addWidget(ui->btnRank);
    pLayout2->addWidget(ui->btnReset);
    pLayout2->addWidget(ui->btnClearBoard);
    pLayout2->addWidget(ui->btnClearRankList);
    pLayout2->addStretch(1);
    ui->grpBoxOp->setLayout(pLayout2);

    //��ʼ��layout:��
    QGridLayout *pLayout3 = new QGridLayout(this);
    pLayout3->addWidget(ui->labelServer,    0,  0);
    pLayout3->addWidget(ui->labelMaster,    0,  1);
    pLayout3->addWidget(ui->labelClient,    0,  2);
    pLayout3->addWidget(ui->labelOpUser,    0,  3);
    pLayout3->addWidget(ui->labelOpType,    0,  4);

    pLayout3->addWidget(ui->lineA0,         1,  0);
    pLayout3->addWidget(ui->lineA1,         1,  1);
    pLayout3->addWidget(ui->lineA2,         1,  2);
    pLayout3->addWidget(ui->lineA3,         1,  3);
    pLayout3->addWidget(ui->lineA4,         1,  4);

    pLayout3->addWidget(ui->lineB0,         2,  0);
    pLayout3->addWidget(ui->lineB1,         2,  1);
    pLayout3->addWidget(ui->lineB2,         2,  2);
    pLayout3->addWidget(ui->lineB3,         2,  3);
    pLayout3->addWidget(ui->lineB4,         2,  4);

    pLayout3->addWidget(ui->lineC0,         3,  0);
    pLayout3->addWidget(ui->lineC1,         3,  1);
    pLayout3->addWidget(ui->lineC2,         3,  2);
    pLayout3->addWidget(ui->lineC3,         3,  3);
    pLayout3->addWidget(ui->lineC4,         3,  4);
    ui->grpBoxBoard->setLayout(pLayout3);

    //��ʼ��layout:���а�
    QGridLayout *pLayout4 = new QGridLayout(this);
    pLayout4->addWidget(ui->tableRankList,  0,  0);
    ui->grpBoxRankList->setLayout(pLayout4);

    //��ʼ�����ڵ�layout
    QGridLayout *pLayout = new QGridLayout(this);
    pLayout->addWidget(ui->grpBoxRankCond,  0,  0);
    pLayout->addWidget(ui->grpBoxOp,        1,  0);
    pLayout->addWidget(ui->grpBoxBoard,     2,  0);
    pLayout->addWidget(ui->grpBoxRankList,  3,  0);

    pLayout->setRowStretch(0, 1);
    pLayout->setRowStretch(1, 1);
    pLayout->setRowStretch(2, 2);
    pLayout->setRowStretch(3, 9);
    setLayout(pLayout);
}

//���ñ���д�С
void TabRankListWidget::setTableColumnWidth()
{
    static int widths[] = {
        300, 100, 100
    };
    int len = sizeof(widths)/sizeof(widths[0]);
    for (int i = 0; i < len; i++) {
        ui->tableRankList->setColumnWidth(i, widths[i]);
    }
}

//����sql��ѯ����
QString TabRankListWidget::createSqlCmd(int index)
{
    static QString fieldArr[] = {
        "server_ip", "master", "client_ip", "opuser", "optype", "err_info"
    };
    QString strCmd, strWhere;
    QDateTime dtFrom = ui->dateTimeFrom->dateTime();
    QDateTime dtTo = ui->dateTimeTo->dateTime();

    if (m_iRankIndex == index) {
        //��������
        QString strOpUser = ui->lineOpUser->text().trimmed();
        if (!strOpUser.isEmpty()) {
            strWhere = tr(" where start_time>=%1 and start_time<=%2 and opuser='%3' ")
                    .arg(dtFrom.toTime_t())
                    .arg(dtTo.toTime_t())
                    .arg(strOpUser);
        } else {
            strWhere = tr(" where start_time>=%1 and start_time<=%2 ")
                    .arg(dtFrom.toTime_t())
                    .arg(dtTo.toTime_t());
        }
    } else {
        //����
        strWhere = tr(" where start_time>=%1 and start_time<=%2 ")
                .arg(dtFrom.toTime_t())
                .arg(dtTo.toTime_t());
    }

    strCmd = tr("select %1,count(*) as counts from webhelper_log %2 group by %3 order by counts desc")
            .arg(fieldArr[index])
            .arg(strWhere)
            .arg(fieldArr[index]);

    return strCmd;
}

//���ò�ѯ�����ui
void TabRankListWidget::setSqlResultToUi(int rankIndex, int totalRow, QSqlQuery &query)
{
    //������а���
    ui->tableRankList->clearContents();
    ui->tableRankList->setRowCount(0);
    setTableColumnWidth();

    //�������
    QTableWidgetItem *pItem = NULL;
    int row = 0;
    int count = 0;
    QString strInfo;
    CDataWare *pDw = CDataWare::instance();

    while (query.next()) {
        strInfo = query.value(0).toString();
        count = query.value(1).toInt();

        row = ui->tableRankList->rowCount();
        ui->tableRankList->insertRow(row);

        pItem = new QTableWidgetItem;
        if (m_iRankIndex == rankIndex) {
            pItem->setText(pDw->optypeString(strInfo.toUInt()));
        } else {
            if (strInfo.isEmpty()) {
                strInfo = tr("��");
            }
            pItem->setText(strInfo);
        }
        ui->tableRankList->setItem(row, 0, pItem);

        pItem = new QTableWidgetItem(QString::number(count));
        pItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->tableRankList->setItem(row, 1, pItem);

        pItem = new QTableWidgetItem(QString::number(totalRow));
        pItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->tableRankList->setItem(row, 2, pItem);

        pItem = new QTableWidgetItem(tr("%1%").arg(100.0*count/totalRow, 0, 'f', 2));
        pItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->tableRankList->setItem(row, 3, pItem);
    }

    //ȡǰ����
    row = ui->tableRankList->rowCount();
    if (row > 3) {
        row = 3;
    }
    QString strRankResult[3] = {"", "", ""};
    for (int i = 0; i < row; i++) {
        strRankResult[i] = ui->tableRankList->item(i, 0)->text();
    }

    //���õ���
    switch (rankIndex) {
    case 0:
        ui->lineA0->setText(strRankResult[0]);
        ui->lineB0->setText(strRankResult[1]);
        ui->lineC0->setText(strRankResult[2]);
        break;
    case 1:
        ui->lineA1->setText(strRankResult[0]);
        ui->lineB1->setText(strRankResult[1]);
        ui->lineC1->setText(strRankResult[2]);
        break;
    case 2:
        ui->lineA2->setText(strRankResult[0]);
        ui->lineB2->setText(strRankResult[1]);
        ui->lineC2->setText(strRankResult[2]);
        break;
    case 3:
        ui->lineA3->setText(strRankResult[0]);
        ui->lineB3->setText(strRankResult[1]);
        ui->lineC3->setText(strRankResult[2]);
        break;
    case 4:
        ui->lineA4->setText(strRankResult[0]);
        ui->lineB4->setText(strRankResult[1]);
        ui->lineC4->setText(strRankResult[2]);
        break;
    default:break;
    }
}

//���ð�ť��Ч
void TabRankListWidget::setCtrlButtonEnabled(bool enabled)
{
    ui->btnRank->setEnabled(enabled);
    ui->btnReset->setEnabled(enabled);
    ui->btnClearBoard->setEnabled(enabled);
    ui->btnClearRankList->setEnabled(enabled);
    qApp->processEvents();
}
