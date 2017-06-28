#ifndef SELECTCOLUMNDIALOG_H
#define SELECTCOLUMNDIALOG_H

#include <QDialog>
#include <common.h>

namespace Ui {
class SelectColumnDialog;
}

class SelectColumnDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SelectColumnDialog(QWidget *parent = 0);
    ~SelectColumnDialog();

    //����ѡ����
    void setSelectColumn(QList<int> intList);

    //��ȡѡ����
    QList<int> getSelectColumn();

    //���㱻ѡ�еĸ���
    int countSelectColumn();
    
private slots:
    //button box
    void slotAccepted();
    void slotRejected();

private:
    //func
    //��ʼ��ui
    void initSetupUi();

    //variable
    Ui::SelectColumnDialog *ui;
};

#endif // SELECTCOLUMNDIALOG_H
