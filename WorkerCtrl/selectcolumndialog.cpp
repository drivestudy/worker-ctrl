#include "selectcolumndialog.h"
#include "ui_selectcolumndialog.h"

SelectColumnDialog::SelectColumnDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectColumnDialog)
{
    ui->setupUi(this);

    initSetupUi();
}

SelectColumnDialog::~SelectColumnDialog()
{
    delete ui;
}

//设置选择结果
void SelectColumnDialog::setSelectColumn(QList<int> intList)
{
    int minLen = (intList.size() <= ui->listColumn->count()) ? intList.size() : ui->listColumn->count();
    for (int i = 0; i < minLen; i++) {
        ui->listColumn->item(i)->setCheckState((Qt::CheckState)intList.at(i));
    }
}

//获取选择结果
QList<int> SelectColumnDialog::getSelectColumn()
{
    QList<int> intList;
    int rows = ui->listColumn->count();

    for (int i = 0; i < rows; i++) {
        intList.append((int)ui->listColumn->item(i)->checkState());
    }
    return intList;
}

//计算被选中的个数
int SelectColumnDialog::countSelectColumn()
{
    int count = 0;
    int row = ui->listColumn->count();

    for (int i = 0; i < row; i++) {
        if (ui->listColumn->item(i)->checkState() == Qt::Checked) {
            ++count;
        }
    }
    return count;
}

/*****slot*****/

//button box
void SelectColumnDialog::slotAccepted()
{
    if (0 == countSelectColumn()) {
        QMessageBox::warning(this, tr("警告"), tr("至少选择一列！"));
        return;
    }
    this->accept();
}

void SelectColumnDialog::slotRejected()
{
    this->reject();
}

/*****slot*****/

//初始化ui
void SelectColumnDialog::initSetupUi()
{
    //设置窗口属性
    setFixedSize(350, 340);
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

    //必须与日志管理表的顺序一样
    QStringList strList;

    strList << tr("唯一码") << tr("服务器IP") << tr("操作类型") << tr("子任务") << tr("主账号")
            << tr("操作用户") << tr("用户类型") << tr("用户点数") << tr("存入/提出点数")
            << tr("序号") << tr("页数") << tr("主页网址") << tr("回调网址") << tr("结果获取方式")
            << tr("网页响应信息") << tr("错误源文件") << tr("错误函数") << tr("错误行号")
            << tr("网页存储路径") << tr("错误信息") << tr("CURL错误信息") << tr("客户端IP")
            << tr("客户端端口") << tr("开始时间") << tr("结束时间") << tr("使用时间") << tr("执行结果");
    ui->listColumn->addItems(strList);
    ui->listColumn->setSpacing(1);

    //设置为选中
    int row = ui->listColumn->count();
    for (int i = 0; i < row; i++) {
        ui->listColumn->item(i)->setCheckState(Qt::Checked);
    }

    //button box
    connect(ui->btnBox, SIGNAL(accepted()), this, SLOT(slotAccepted()));
    connect(ui->btnBox, SIGNAL(rejected()), this, SLOT(slotRejected()));
}
