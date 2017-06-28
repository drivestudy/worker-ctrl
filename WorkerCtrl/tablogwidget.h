#ifndef TABLOGWIDGET_H
#define TABLOGWIDGET_H

#include <QWidget>
#include <common.h>

namespace Ui {
class TabLogWidget;
}

class TabLogWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit TabLogWidget(QWidget *parent = 0);
    ~TabLogWidget();

private slots:
    //��ѯ��ť
    void slotBtnSearch();

    //ɾ����ť
    void slotBtnDelete();

    //������ť
    void slotBtnCount();

    //����
    void slotBtnReset();

    //��ղ�ѯ�����ť
    void slotBtnClearResult();

    //��ѯ���˫���ź�
    void slotItemDoubleClicked(QTableWidgetItem *item);

    //ˮƽ��ͷ���Ҽ��˵�����
    void slotCustomContextMenuRequested(const QPoint &pos);

    //ѡ����Action
    void slotSltColAction();
    
private:
    //func
    //��ʼ������
    void initVariable();

    //��ʼ��ui
    void initSetupUi();

    //��ʼ��layout
    void initLayout();

    //�����Ϸ��ж�
    bool isValidData();

    //���ݽ����ṩ������������Ӧ��sql���,��ѯ
    QString createSqlCmdSearch();

    //���ݽ����ṩ������������Ӧ��sql���,����
    QString createSqlCmdCount();

    //���ݽ����ṩ������������Ӧ��sql���,ɾ��
    QString createSqlCmdRemove();

    //ƴ����������
    QString createSqlCondition();

    //�������ݿ��ѯ�����ui
    void setSqlResultToTable(QSqlQuery &query);

    //�û�����ת��
    QString usertypeString(uint type);

    //���ñ���д�С
    void setTableColumnWidth();

    //���ư�ť�����������
    void setCtrlButtonEnabled(bool enabled);

    //��ղ�ѯ���
    void clearSearchResult();

    //��ȡxml�����ļ�
    void readXml(QList<int> &intList);

    //д��xml�����ļ�
    void writeXml(const QList<int> &intList);

    //��ӵ�ǰ��������������
    void addCurrentServerIp();

    //variable
    Ui::TabLogWidget *ui;
    QAction *m_pSltColAction;//ѡ����
    QMenu m_tableMenu;//���ͷ���Ҽ��˵�
};

#endif // TABLOGWIDGET_H
