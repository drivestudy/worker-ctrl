#ifndef CHOOSEURLDIALOG_H
#define CHOOSEURLDIALOG_H

#include <QtCore>
#include <QtGui>

namespace Ui {
class ChooseUrlDialog;
}

class ChooseUrlDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ChooseUrlDialog(QWidget *parent = 0);
    ~ChooseUrlDialog();
    int GetUrlFlag();

private slots:
    void slotOk();
    void slotCancel();
    void slotChkClicked();

private:
    void initSetupUi();
    
private:
    Ui::ChooseUrlDialog *ui;
    int m_nUrlFlag;
    QMutex m_uiMutex;
};

#endif // CHOOSEURLDIALOG_H
