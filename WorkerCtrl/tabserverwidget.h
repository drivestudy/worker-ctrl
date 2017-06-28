#ifndef TABSERVERWIDGET_H
#define TABSERVERWIDGET_H

#include <QWidget>
#include <common.h>

namespace Ui {
class TabServerWidget;
}

class CUpdateThread;
class MonitorThread;
typedef QList<MonitorThread *> MonitorThreadList;

class TabServerWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit TabServerWidget(QWidget *parent = 0);
    ~TabServerWidget();

protected:
    void resizeEvent(QResizeEvent *event);

signals:
    //����������
    void signalServerChanged();

    //����̵߳�״̬�л��ź�
    void signalChangeState(bool online, const QString &ip, const QString &version);
    
private slots:
    //tabServer
    void slotCurrentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void slotCellDoubleClicked(int row, int column);

    //�Ҽ��˵�
    void slotCustomContextMenuRequested(const QPoint &pos);

    //�˵��¼�:ȫѡ
    void slotActionSelectAll();

    //�˵��¼�:��ѡ
    void slotActionUnSelect();

    //���
    void slotBtnSrvAdd();

    //�޸�
    void slotBtnSrvModify();

    //ɾ��
    void slotBtnSrvDelete();

    //���
    void slotBtnSrvClearList();

    //��ʼ���
    void slotBtnSrvStartMonitor();

    //ֹͣ���
    void slotBtnSrvStopMonitor();

    //����ļ�
    void slotBtnScanFile();

    //����
    void slotBtnUpdate();

    //����
    void slotBtnReset();

    //����߳�
    //��ʼ����ź�
    void slotStartMonitor(const QString &ip);

    //ֹͣ����ź�
    void slotStopMonitor(const QString &ip);

    //���·�������Ϣ
    void slotUpdateServerInfo(bool online, const ServerInfo &srvInfo);

    //�汾�����߳�
    //��������
    void slotUpdateProgress(const QString &ip, const QString &info);

    //�߳̽���
    void slotUpdateFinished();

private:
    //func
    //��ʼ������
    void initVariable();

    //��ʼ��ui
    void initSetupUi();

    //��ʼ��layout
    void initLayout();

    //�������б�༭
    bool appendServer(const QString &ip, ushort srvPort, ushort ctrlPort,
                      int monitorPeriod, const QString &ctrlPwd);
    bool removeServer(const QString &ip);
    bool modifyServer(const QString &ip, ushort srvPort, ushort ctrlPort,
                      int monitorPeriod, const QString &ctrlPwd);
    int findItem(const QString &ip);

    //�����Ϸ��ж�
    bool isValidData();

    //������д��xml
    void writeXml();

    //��xml��ȡ��������Ϣ
    void readXml();

    //�����б��п��
    void setTableColumnWidth();

    //ֹͣ���м���߳�
    void stopAllMonitor();

    //��ȡ�б�ѡ�еķ�����ip
    QStringList selectedIpList();

    //���Ƴ��������İ�ť
    void enabledUpdateWidget(bool enabled);

    //variable
    Ui::TabServerWidget *ui;

    //��ǰ��ʾʵʱ״̬�ķ�����ip
    QString m_strCurRtSrvIP;

    //�˵�
    QMenu m_srvMenu;
    QAction *m_pActionSelectAll;
    QAction *m_pActionUnSelect;
    CUpdateThread *m_pUpdateThread;
};

#endif // TABSERVERWIDGET_H
