#ifndef INPUTPASSWORDDIALOG_H
#define INPUTPASSWORDDIALOG_H

#include <QDialog>
#include <QtGui>

namespace Ui {
class InputPasswordDialog;
}

class InputPasswordDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit InputPasswordDialog(QWidget *parent = 0);
    ~InputPasswordDialog();

    //获取密码
    QString getPassword();
    
private:
    //初始化ui
    void initSetupUi();

    //variable
    Ui::InputPasswordDialog *ui;
};

#endif // INPUTPASSWORDDIALOG_H
