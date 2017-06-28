#ifndef WORKERCTRLWIDGET_H
#define WORKERCTRLWIDGET_H

#include <QWidget>
#include <common.h>

//custom
class TabServerWidget;
class TabLogWidget;
class TabSettingWidget;
class TabRankListWidget;
class TabSyncUserWidget;

namespace Ui {
class WorkerCtrlWidget;
}

class WorkerCtrlWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit WorkerCtrlWidget(QWidget *parent = 0);
    ~WorkerCtrlWidget();

    //��ʼ��ui
    void initSetupUi();

    //��ʼ��layout
    void initLayout();

    //��ʼ������
    void initVariable();

    //��ʼ��ϵͳͼ��
    void initSystrayIcon();

protected:
    void closeEvent(QCloseEvent *event);
    
private slots:
    //ϵͳ����ͼ��
    void slotSysTrayIconActivated(QSystemTrayIcon::ActivationReason reason);

    //�˳��˵�
    void slotActionQuit();

    //�л�tabҳ���ź�
    void slotSwitchTabWidget(int page);

    //������ҳ�洫�ݹ����� ����̵߳�״̬�л��ź�
    void slotChangeState(bool online, const QString &ip, const QString &version);

private:
    //variable
    Ui::WorkerCtrlWidget *ui;
    //tabҳ
    TabServerWidget *m_pTabServerWidget;
    TabLogWidget *m_pTabLogWidget;
    TabSettingWidget *m_pTabSettingWidget;
    TabRankListWidget *m_pTabRankListWidget;
    TabSyncUserWidget *m_pTabSyncUserWidget;

    //ϵͳ����ͼ��
    QAction *m_pActionQuit;
    QMenu *m_pSystrayMenu;
    QSystemTrayIcon *m_pSysTrayIcon;
};

#endif // WORKERCTRLWIDGET_H
