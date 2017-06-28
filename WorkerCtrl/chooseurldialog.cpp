#include "chooseurldialog.h"
#include "ui_chooseurldialog.h"

ChooseUrlDialog::ChooseUrlDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseUrlDialog)
{
    ui->setupUi(this);
    initSetupUi();
}

ChooseUrlDialog::~ChooseUrlDialog()
{
    delete ui;
}

int ChooseUrlDialog::GetUrlFlag()
{
    return m_nUrlFlag;
}

void ChooseUrlDialog::slotOk()
{
    this->accept();
}

void ChooseUrlDialog::slotCancel()
{
    this->reject();
}

void ChooseUrlDialog::slotChkClicked()
{
    QCheckBox* pChkArr[]=
    {
        ui->chkUrl0,
        ui->chkUrl1,
        ui->chkUrl2,
        ui->chkUrl3,
        ui->chkUrl4,
        ui->chkUrl5,
        ui->chkUrl6,
        ui->chkUrl7
    };
    m_uiMutex.lock();
    int nLen = sizeof(pChkArr) / sizeof(QCheckBox*);
    m_nUrlFlag = 0;
    for (int i = 0; i < nLen; ++i)
    {
        if (pChkArr[i]->isChecked())
        {
            m_nUrlFlag |= 1 << i;
        }
    }
    ui->labelCode->setText(QString::number(m_nUrlFlag));
    m_uiMutex.unlock();
}

void ChooseUrlDialog::initSetupUi()
{
    m_nUrlFlag = 0;

    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    setFixedSize(190, 250);

    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(slotCancel()));
    connect(ui->chkUrl0, SIGNAL(clicked()), this, SLOT(slotChkClicked()));
    connect(ui->chkUrl1, SIGNAL(clicked()), this, SLOT(slotChkClicked()));
    connect(ui->chkUrl2, SIGNAL(clicked()), this, SLOT(slotChkClicked()));
    connect(ui->chkUrl3, SIGNAL(clicked()), this, SLOT(slotChkClicked()));
    connect(ui->chkUrl4, SIGNAL(clicked()), this, SLOT(slotChkClicked()));
    connect(ui->chkUrl5, SIGNAL(clicked()), this, SLOT(slotChkClicked()));
    connect(ui->chkUrl6, SIGNAL(clicked()), this, SLOT(slotChkClicked()));
    connect(ui->chkUrl7, SIGNAL(clicked()), this, SLOT(slotChkClicked()));
}



