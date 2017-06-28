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
    //按钮信号槽
    void slotBtnReset();
    void slotBtnSubmit();
    void slotBtnStop();

    //服务器增减
    void slotServerChanged();

    //使用时间定时器
    void slotTimeout();

    //发送token
    void slotSendToken(const QString &token);

    //处理结果
    void slotDealResult(const QString &result);

    //线程结束信号
    void slotThreadFinished();

    //列表项复制
    void slotItemDoubleClicked(QTableWidgetItem *item);

private:
    //function
    //初始化ui
    void initSetupUi();

    //初始化layout
    void initLayout();

    //初始化变量
    void initVariable();

    //开启定时器循环获取结果
    void startTimer();

    //关闭定时器
    void stopTimer();

    //variable
    Ui::TabSyncUserWidget *ui;
    QString m_strToken;
    QString m_strServerIp;
    ushort  m_iServerPort;
    QString m_strGetResultJson;
    QTimer  m_timer;//使用时间定时器
    uint m_iTaskStartTime;//任务开始时间
    CSubmitTaskThread *m_pSubmitTaskThread;
    static const int GETRESULT_PERIOD = 3000;
};

#endif // TABSYNCUSERWIDGET_H
