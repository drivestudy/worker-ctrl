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

//查询按钮
void TabLogWidget::slotBtnSearch()
{
    if (!isValidData()) {
        return;
    }

    //添加服务器到下拉框
    addCurrentServerIp();

    QSqlDatabase sqlDb;
    //判断数据库对象是否可用
    if (!CDataWare::instance()->getLogSqlDb(sqlDb)) {
        QMessageBox::warning(this, tr("警告"), tr("请设置正确的数据库参数！"));
        return;
    }

    //准备工作
    setCtrlButtonEnabled(false);
    ui->tableSearchResult->setSortingEnabled(false);
    if (sqlDb.open()) {
        QSqlQuery query = sqlDb.exec(createSqlCmdSearch());
        if (query.lastError().isValid()) {
            //错误
            QMessageBox::warning(this, tr("警告"),
                                 tr("日志查询失败: %1").arg(query.lastError().text()));
        } else {
            //设置结果到ui
            setSqlResultToTable(query);
            query.clear();
        }
        sqlDb.close();
    } else {
        QMessageBox::warning(this, tr("警告"),
                             tr("数据库打开失败: %1").arg(sqlDb.lastError().text()));
    }

    //后续工作
    setCtrlButtonEnabled(true);
    ui->tableSearchResult->setSortingEnabled(true);
    ui->tableSearchResult->sortByColumn(23, Qt::AscendingOrder);//开始时间排序
}

//删除按钮
void TabLogWidget::slotBtnDelete()
{
    if (!isValidData()) {
        return;
    }

    //添加服务器到下拉框
    addCurrentServerIp();

    QSqlDatabase sqlDb;
    //判断数据库对象是否可用
    if (!CDataWare::instance()->getLogSqlDb(sqlDb)) {
        QMessageBox::warning(this, tr("警告"), tr("请设置正确的数据库参数！"));
        return;
    }

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

    if (QMessageBox::Ok == QMessageBox::warning(this, tr("警告"), tr("数据删除后无法恢复，请谨慎！"), QMessageBox::Ok | QMessageBox::Cancel)) {
        setCtrlButtonEnabled(false);
        if (sqlDb.open()) {
            QSqlQuery query = sqlDb.exec(createSqlCmdRemove());
            if (query.lastError().isValid()) {
                //错误
                QMessageBox::warning(this, tr("警告"),
                                     tr("日志删除失败: %1").arg(query.lastError().text()));
            } else {
                query.clear();
            }
            sqlDb.close();
        } else {
            QMessageBox::warning(this, tr("警告"),
                                 tr("数据库打开失败: %1").arg(sqlDb.lastError().text()));
        }
        setCtrlButtonEnabled(true);
    }
}

//计数按钮
void TabLogWidget::slotBtnCount()
{
    if (!isValidData()) {
        return;
    }

    //添加服务器到下拉框
    addCurrentServerIp();

    QSqlDatabase sqlDb;
    //判断数据库对象是否可用
    if (!CDataWare::instance()->getLogSqlDb(sqlDb)) {
        QMessageBox::warning(this, tr("警告"), tr("请设置正确的数据库参数！"));
        return;
    }

    //准备工作
    setCtrlButtonEnabled(false);
    if (sqlDb.open()) {

        QSqlQuery query = sqlDb.exec(createSqlCmdCount());
        if (query.lastError().isValid()) {
            //错误
            QMessageBox::warning(this, tr("警告"),
                                 tr("日志查询失败: %1").arg(query.lastError().text()));
        } else {
            //设置结果到ui
            if (query.next()) {
                int count = query.value(0).toInt();
                QMessageBox::information(this, tr("计数结果"),
                                         tr("日志数据库含有%1条匹配记录。").arg(count));
            }
            query.clear();
        }
        sqlDb.close();
    } else {
        QMessageBox::warning(this, tr("警告"),
                             tr("数据库打开失败: %1").arg(sqlDb.lastError().text()));
    }

    //后续工作
    setCtrlButtonEnabled(true);
}

//重置
void TabLogWidget::slotBtnReset()
{
    //从服务器列表导入到这个组合框
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

//清空查询结果按钮
void TabLogWidget::slotBtnClearResult()
{
    if (QMessageBox::Ok == QMessageBox::warning(this, tr("警告"), tr("确定要清空查询结果？"), QMessageBox::Ok | QMessageBox::Cancel)) {
        clearSearchResult();
    }
}

//查询结果双击信号
void TabLogWidget::slotItemDoubleClicked(QTableWidgetItem *item)
{
    if (item) {
        qApp->clipboard()->setText(item->text());
    }
}

//水平表头的右键菜单触发
void TabLogWidget::slotCustomContextMenuRequested(const QPoint &pos)
{
    m_tableMenu.exec(QCursor::pos());
}

//选择列Action
void TabLogWidget::slotSltColAction()
{
    SelectColumnDialog scd(this);
    QList<int> intList;

    //从配置文件读取设置
    readXml(intList);
    scd.setSelectColumn(intList);

    if (QDialog::Accepted == scd.exec()) {
        intList = scd.getSelectColumn();
        int minLen = (intList.size() <= ui->tableSearchResult->columnCount()) ?
                    intList.size() : ui->tableSearchResult->columnCount();

        for (int i = 0; i < minLen; i++) {
            ui->tableSearchResult->setColumnHidden(i, ((Qt::CheckState)intList.at(i) != Qt::Checked));
        }

        //写入配置文件
        writeXml(intList);
    }
}

/*****slot*****/

//初始化变量
void TabLogWidget::initVariable()
{
    m_pSltColAction = NULL;
}

//初始化ui
void TabLogWidget::initSetupUi()
{
    /**表格**/
    ui->tableSearchResult->setItemDelegate(new NoFocusDelegate(this));
    //表格头部
    ui->tableSearchResult->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableSearchResult->horizontalHeader(), SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotCustomContextMenuRequested(QPoint)));

    //右键菜单
    m_pSltColAction = new QAction(tr("选择列..."), this);
    connect(m_pSltColAction, SIGNAL(triggered()), this, SLOT(slotSltColAction()));
    m_tableMenu.addAction(m_pSltColAction);

    ui->tableSearchResult->clearContents();
    ui->tableSearchResult->setRowCount(0);
    ui->tableSearchResult->resizeColumnsToContents();
    ui->tableSearchResult->verticalHeader()->setResizeMode(QHeaderView::Fixed);

    //设置表格大小
    setTableColumnWidth();

    //表格双击事件
    connect(ui->tableSearchResult, SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
            this, SLOT(slotItemDoubleClicked(QTableWidgetItem*)));
    /**表格**/

    /**按钮**/
    connect(ui->btnSearch, SIGNAL(clicked()), this, SLOT(slotBtnSearch()));
    connect(ui->btnDelete, SIGNAL(clicked()), this, SLOT(slotBtnDelete()));
    connect(ui->btnCount, SIGNAL(clicked()), this, SLOT(slotBtnCount()));
    connect(ui->btnReset, SIGNAL(clicked()), this, SLOT(slotBtnReset()));
    connect(ui->btnClearResult, SIGNAL(clicked()), this, SLOT(slotBtnClearResult()));
    /**按钮**/

    /**查询条件**/
    //初始化日志查询内容
    slotBtnReset();
    /**查询条件**/

    //设置查询结果列
    //从配置文件读取设置
    QList<int> intList;
    readXml(intList);
    int minLen = (intList.size() <= ui->tableSearchResult->columnCount()) ?
                intList.size() : ui->tableSearchResult->columnCount();

    for (int i = 0; i < minLen; i++) {
        ui->tableSearchResult->setColumnHidden(i, ((Qt::CheckState)intList.at(i) != Qt::Checked));
    }
}

//初始化layout
void TabLogWidget::initLayout()
{
    //日志查询条件
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

    //数据库操作
    QHBoxLayout *pLayout2 = new QHBoxLayout(this);
    pLayout2->addWidget(ui->btnSearch);
    pLayout2->addWidget(ui->btnDelete);
    pLayout2->addWidget(ui->btnCount);
    pLayout2->addWidget(ui->btnReset);
    pLayout2->addWidget(ui->btnClearResult);
    pLayout2->addStretch(1);
    ui->grpBoxSqlOp->setLayout(pLayout2);

    //查询结果
    QGridLayout *pLayout3 = new QGridLayout(this);
    pLayout3->addWidget(ui->tableSearchResult, 0, 0);
    ui->grpBoxSearchResult->setLayout(pLayout3);

    //设置窗口的布局
    QGridLayout *pLayout = new QGridLayout(this);
    pLayout->addWidget(ui->grpBoxLog, 0, 0);
    pLayout->addWidget(ui->grpBoxSqlOp, 1, 0);
    pLayout->addWidget(ui->grpBoxSearchResult, 2, 0);
    pLayout->setRowStretch(0, 2);
    pLayout->setRowStretch(1, 1);
    pLayout->setRowStretch(2, 10);
    setLayout(pLayout);
}

//交互合法判断
bool TabLogWidget::isValidData()
{
    //服务器ip地址
//    if (!ui->lineServerIp->text().isEmpty() && !CPublic::isIpAddr(ui->lineServerIp->text())) {
//        QMessageBox::warning(this, tr("警告"), tr("请输入合法的服务器地址！"));
//        ui->lineServerIp->setFocus();
//        return false;
//    }
    if (!ui->comboBoxServerIp->currentText().isEmpty()
            && !CPublic::isIpAddr(ui->comboBoxServerIp->currentText())) {
        QMessageBox::warning(this, tr("警告"), tr("请输入合法的服务器地址！"));
        ui->comboBoxServerIp->setFocus();
        return false;
    }

    //客户端ip地址
    if (!ui->lineClientIp->text().isEmpty() && !CPublic::isIpAddr(ui->lineClientIp->text())) {
        QMessageBox::warning(this, tr("警告"), tr("请输入合法的客户端地址！"));
        ui->lineClientIp->setFocus();
        return false;
    }

    //时间段
    if (ui->dateTimeFrom->dateTime() >= ui->dateTimeTo->dateTime()) {
        QMessageBox::warning(this, tr("警告"), tr("结束时间不能比开始时间早！"));
        ui->dateTimeFrom->setFocus();
        return false;
    }

    return true;
}

//根据界面提供的条件生成响应的sql语句,查询
//查询的字段顺序必须跟查询结果QTableWidget的表头顺序一样
QString TabLogWidget::createSqlCmdSearch()
{
    return ("select token,server_ip,optype,child_task,master,opuser,user_type,"
            "user_point,draw_deposit_point,seq_num,page_num,home_url,cb_url,"
            "result_method,html_responce,err_file,err_function,err_line,"
            "html_filepath,err_info,curl_err_info,client_ip,client_port,"
            "start_time,stop_time,use_time,result from webhelper_log"
            + createSqlCondition());
}

//根据界面提供的条件生成响应的sql语句,计数
QString TabLogWidget::createSqlCmdCount()
{
    return ("select count(*) from webhelper_log" + createSqlCondition());
}

//根据界面提供的条件生成响应的sql语句,删除
QString TabLogWidget::createSqlCmdRemove()
{
    return ("delete from webhelper_log" + createSqlCondition());
}

//拼接条件部分
QString TabLogWidget::createSqlCondition()
{
    QString strCond, strTmp, strElement;
    QStringList strList;

    //服务器
    strTmp = ui->comboBoxServerIp->currentText();
    if (!strTmp.isEmpty()) {
        strElement = tr("server_ip='%1'").arg(strTmp);
        strList.append(strElement);
    }

    //主账号
    strTmp = ui->lineMaster->text();
    if (!strTmp.isEmpty()) {
        strElement = tr("master='%1'").arg(strTmp);
        strList.append(strElement);
    }

    //操作用户
    strTmp = ui->lineOpUser->text();
    if (!strTmp.isEmpty()) {
        strElement = tr("opuser='%1'").arg(strTmp);
        strList.append(strElement);
    }

    //客户端
    strTmp = ui->lineClientIp->text();
    if (!strTmp.isEmpty()) {
        strElement = tr("client_ip='%1'").arg(strTmp);
        strList.append(strElement);
    }

    //开始时间 -> 开始时间
    uint fromDate = 0, toDate = 0;
    fromDate = ui->dateTimeFrom->dateTime().toTime_t();
    toDate = ui->dateTimeTo->dateTime().toTime_t();

    strElement = tr("start_time>=%1").arg(fromDate);
    strList.append(strElement);
    strElement = tr("start_time<=%1").arg(toDate);
    strList.append(strElement);

    //用户类型:全部 会员 代理
    int index = ui->comboBoxUserType->currentIndex();
    if (0 != index) {
        //选择的不是'全部'
        strElement = tr("user_type=%1").arg(index);
        strList.append(strElement);
    }

    //操作类型:全部 xx  xx xx ...
    index = ui->comboBoxOpType->currentIndex();
    if (0 != index) {
        //选择的不是'全部'
        strElement = tr("optype=%1").arg(index);
        strList.append(strElement);
    }

    //结果获取方式:全部 get post
    index = ui->comboBoxResultMethod->currentIndex();
    if (0 != index) {
        //选择的不是'全部'
        if (1 == index) {
            strTmp = "get";
        } else if (2 == index) {
            strTmp = "post";
        }
        strElement = tr("result_method='%1'").arg(strTmp);
        strList.append(strElement);
    }

    //执行结果:全部 成功 失败
    index = ui->comboBoxTaskResult->currentIndex();
    if (0 != index) {
        //选择的不是'全部'
        if (1 == index) {
            index = 1;//成功
        } else if (2 == index) {
            index = 0;//失败
        }
        strElement = tr("result=%1").arg(index);
        strList.append(strElement);
    }

    //错误类型:
    /*
    0 : "全部"
    1 : "0.成功"
    2 : "1.失败"
    3 : "2.连接到cookie服务器失败"
    4 : "3.发送用户名密码失败"
    5 : "4.接收cookie失败"
    6 : "5.cookie服务器返回的信息格式错误"
    7 : "6.AES加密数据格式错误"
    8 : "7.太阳城登录失败"
    9 : "8.太阳城正在维护"
    10 : "9.cookie过期(System Error)"
    11 : "10.cookie过期(404 - File or directory not found)"
    12 : "11.cookie过期(This page uses frames)"
    13 : "12.cookie过期(The requested URL could not be retrieved)"
    14 : "13.cookie过期(太阳城错误)"
    15 : "14.json格式错误"
    16 : "15.下载页面失败"
    17 : "16.提交数据失败"
    18 : "17.curl初始化失败"
    19 : "18.写文件失败"
    20 : "19.匹配不到数据"
    21 : "20.找不到激活用户"
    22 : "21.操作类型错误"
    23 : "22.用户类型错误"
    24 : "23.用户/投注状态错误"
    25 : "24.用户密码为空"
    26 : "25.唯一码为空"
    27 : "26.自定义(网页返回的错误信息)"
    28 : "27.查询报表的日期为空"
    29 : "28.数据库参数错误"
    30 : "29.数据库初始化失败"
    31 : "30.数据库连接失败"
    32 : "31.注册失败,搜索不到用户"
    33 : "32.数据库命令执行失败"
    34 : "33.空指针错误"
    35 : "-1.未知错误"
    */
    index = ui->comboBoxErrorType->currentIndex();
    if (0 != index) {
        //选择的不是'全部'

        if (35 == index) {
            //未知错误:-1
            index = -1;
        } else {
            index -= 1;
        }
        strElement = tr("err_info like '%#%1'").arg(index);
        strList.append(strElement);
    }

    //存在条件再处理
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

//设置数据库查询结果到ui
void TabLogWidget::setSqlResultToTable(QSqlQuery &query)
{
    int row = 0, cols = 0, recordCount = 0, index = 1, count = 0;
    QTableWidgetItem *pItem = NULL;
    QString strTmp, strResult;
    CDataWare *pDw = CDataWare::instance();

    //清空旧数据
    clearSearchResult();

    //记录总数
    recordCount = query.size();

    //取得列数
    QSqlRecord sqlRecord = query.record();
    cols = sqlRecord.count();

    //设置groupBox标题
    ui->grpBoxSearchResult->setTitle(tr("查询结果(0/%2)").arg(recordCount));

    while (query.next()) {
        //插入一行
        row = ui->tableSearchResult->rowCount();
        ui->tableSearchResult->insertRow(row);

        //部分字段需要转换
        for (int i = 0; i < cols; i++) {
            strTmp = query.value(i).toString();
            switch (i) {
            case 2:
                //optype
                strResult = pDw->optypeString(strTmp.toUInt());
                break;
            case 3:
                //child_task
                strResult = (1 == strTmp.toInt()) ? tr("是") : tr("否");
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
                strResult = (1 == strTmp.toInt()) ? tr("成功") : tr("失败");
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

        //随时显示最新添加的行
//        ui->tableSearchResult->scrollToBottom();

        //设置groupBox标题
        ui->grpBoxSearchResult->setTitle(tr("查询结果(%1/%2)").arg(index++).arg(recordCount));

        //马上更新ui
        if (count++ % 200 == 0) {
            qApp->processEvents();
        }
    }
}

//用户类型转换
QString TabLogWidget::usertypeString(uint type)
{
    static QString arr[] = {"无", "会员", "代理"};

    if (type < sizeof(arr)/sizeof(arr[0])) {
        return arr[type];
    } else {
        return "";
    }
}

//设置表格列大小
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

//控制按钮的启用与禁用
void TabLogWidget::setCtrlButtonEnabled(bool enabled)
{
    ui->btnSearch->setEnabled(enabled);
    ui->btnDelete->setEnabled(enabled);
    ui->btnCount->setEnabled(enabled);
    ui->btnReset->setEnabled(enabled);
    ui->btnClearResult->setEnabled(enabled);
    qApp->processEvents();
}

//清空查询结果
void TabLogWidget::clearSearchResult()
{
    ui->tableSearchResult->clearContents();
    ui->tableSearchResult->setRowCount(0);
    ui->tableSearchResult->horizontalScrollBar()->setValue(0);
    //设置表格大小
    setTableColumnWidth();
    ui->grpBoxSearchResult->setTitle(tr("查询结果"));
}

//读取xml配置文件
void TabLogWidget::readXml(QList<int> &intList)
{
    //载入xml文件
    QString strFileName = qApp->applicationDirPath() + '/' + SET_FILE;
    QFile file(strFileName);

    //默认显示所有列
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

//写入xml配置文件
void TabLogWidget::writeXml(const QList<int> &intList)
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
        //删除tablog标签
        root = doc.childNodes().at(1);
        QDomNode node = doc.elementsByTagName("tablog").at(0);
        root.removeChild(node);//删除
    } else {
        //不存在
        QDomNode xmlHeader = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
        doc.appendChild(xmlHeader);

        root = doc.createElement("setting");
        doc.appendChild(root);
    }

    //创建tablog标签
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

    //保存为xml
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return;
    }
    QTextStream textStream(&file);
    doc.save(textStream, 2);
    file.close();
}

//添加当前服务器到下拉框
void TabLogWidget::addCurrentServerIp()
{
    QString strCurrentText = ui->comboBoxServerIp->currentText();
    if (!strCurrentText.isEmpty()) {
        if (ui->comboBoxServerIp->findText(strCurrentText) == -1) {
            ui->comboBoxServerIp->addItem(strCurrentText);
        }
    }
}

