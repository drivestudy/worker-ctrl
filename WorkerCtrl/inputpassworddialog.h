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

    //��ȡ����
    QString getPassword();
    
private:
    //��ʼ��ui
    void initSetupUi();

    //variable
    Ui::InputPasswordDialog *ui;
};

#endif // INPUTPASSWORDDIALOG_H
