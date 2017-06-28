#include "inputpassworddialog.h"
#include "ui_inputpassworddialog.h"

InputPasswordDialog::InputPasswordDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InputPasswordDialog)
{
    ui->setupUi(this);

    initSetupUi();
}

InputPasswordDialog::~InputPasswordDialog()
{
    delete ui;
}

//获取密码
QString InputPasswordDialog::getPassword()
{
    return ui->linePwd->text();
}

//初始化ui
void InputPasswordDialog::initSetupUi()
{
    //设置窗口属性
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    setFixedSize(380, 50);
    setWindowTitle(tr("请输入密码"));

    //设置layout
    QHBoxLayout *pLayout = new QHBoxLayout(this);
    pLayout->addWidget(ui->labelPwd);
    pLayout->addWidget(ui->linePwd);
    pLayout->addWidget(ui->btnOk);
    pLayout->addWidget(ui->btnCancel);
    setLayout(pLayout);

    //slot
    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->linePwd, SIGNAL(returnPressed()), this, SLOT(accept()));
}
