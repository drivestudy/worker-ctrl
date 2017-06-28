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

//����ѡ����
void SelectColumnDialog::setSelectColumn(QList<int> intList)
{
    int minLen = (intList.size() <= ui->listColumn->count()) ? intList.size() : ui->listColumn->count();
    for (int i = 0; i < minLen; i++) {
        ui->listColumn->item(i)->setCheckState((Qt::CheckState)intList.at(i));
    }
}

//��ȡѡ����
QList<int> SelectColumnDialog::getSelectColumn()
{
    QList<int> intList;
    int rows = ui->listColumn->count();

    for (int i = 0; i < rows; i++) {
        intList.append((int)ui->listColumn->item(i)->checkState());
    }
    return intList;
}

//���㱻ѡ�еĸ���
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
        QMessageBox::warning(this, tr("����"), tr("����ѡ��һ�У�"));
        return;
    }
    this->accept();
}

void SelectColumnDialog::slotRejected()
{
    this->reject();
}

/*****slot*****/

//��ʼ��ui
void SelectColumnDialog::initSetupUi()
{
    //���ô�������
    setFixedSize(350, 340);
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

    //��������־������˳��һ��
    QStringList strList;

    strList << tr("Ψһ��") << tr("������IP") << tr("��������") << tr("������") << tr("���˺�")
            << tr("�����û�") << tr("�û�����") << tr("�û�����") << tr("����/�������")
            << tr("���") << tr("ҳ��") << tr("��ҳ��ַ") << tr("�ص���ַ") << tr("�����ȡ��ʽ")
            << tr("��ҳ��Ӧ��Ϣ") << tr("����Դ�ļ�") << tr("������") << tr("�����к�")
            << tr("��ҳ�洢·��") << tr("������Ϣ") << tr("CURL������Ϣ") << tr("�ͻ���IP")
            << tr("�ͻ��˶˿�") << tr("��ʼʱ��") << tr("����ʱ��") << tr("ʹ��ʱ��") << tr("ִ�н��");
    ui->listColumn->addItems(strList);
    ui->listColumn->setSpacing(1);

    //����Ϊѡ��
    int row = ui->listColumn->count();
    for (int i = 0; i < row; i++) {
        ui->listColumn->item(i)->setCheckState(Qt::Checked);
    }

    //button box
    connect(ui->btnBox, SIGNAL(accepted()), this, SLOT(slotAccepted()));
    connect(ui->btnBox, SIGNAL(rejected()), this, SLOT(slotRejected()));
}
