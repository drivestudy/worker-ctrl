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

    //设置选择结果
    void setSelectColumn(QList<int> intList);

    //获取选择结果
    QList<int> getSelectColumn();

    //计算被选中的个数
    int countSelectColumn();
    
private slots:
    //button box
    void slotAccepted();
    void slotRejected();

private:
    //func
    //初始化ui
    void initSetupUi();

    //variable
    Ui::SelectColumnDialog *ui;
};

#endif // SELECTCOLUMNDIALOG_H
