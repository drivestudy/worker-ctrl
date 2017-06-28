#ifndef TABSYNCUSERWIDGET_H
#define TABSYNCUSERWIDGET_H

#include <common.h>

class CSubmitTaskThread;

namespace Ui {
class TabSyncUserWidget;
}

class TabSyncUserWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit TabSyncUserWidget(QWidget *parent = 0);
    ~TabSyncUserWidget();
    
private slots:
    //��ť�źŲ�
    void slotBtnReset();
    void slotBtnSubmit();
    void slotBtnStop();

    //����������
    void slotServerChanged();

    //ʹ��ʱ�䶨ʱ��
    void slotTimeout();

    //����token
    void slotSendToken(const QString &token);

    //������
    void slotDealResult(const QString &result);

    //�߳̽����ź�
    void slotThreadFinished();

    //�б����
    void slotItemDoubleClicked(QTableWidgetItem *item);

private:
    //function
    //��ʼ��ui
    void initSetupUi();

    //��ʼ��layout
    void initLayout();

    //��ʼ������
    void initVariable();

    //������ʱ��ѭ����ȡ���
    void startTimer();

    //�رն�ʱ��
    void stopTimer();

    //variable
    Ui::TabSyncUserWidget *ui;
    QString m_strToken;
    QString m_strServerIp;
    ushort  m_iServerPort;
    QString m_strGetResultJson;
    QTimer  m_timer;//ʹ��ʱ�䶨ʱ��
    uint m_iTaskStartTime;//����ʼʱ��
    CSubmitTaskThread *m_pSubmitTaskThread;
    static const int GETRESULT_PERIOD = 3000;
};

#endif // TABSYNCUSERWIDGET_H
