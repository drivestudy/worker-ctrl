#ifndef TABRANKLISTWIDGET_H
#define TABRANKLISTWIDGET_H

#include <QWidget>
#include <common.h>

namespace Ui {
class TabRankListWidget;
}

class TabRankListWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit TabRankListWidget(QWidget *parent = 0);
    ~TabRankListWidget();

private slots:
    //����
    void slotBtnRank();
    //����
    void slotBtnReset();
    //��հ�
    void slotBtnClearBoard();
    //������а�
    void slotBtnClearRankList();
    //������Ϣ������
    void slotCurrentIndexChanged(int index);
    //˫�����Ƶ�Ԫ��
    void slotItemDoubleClicked(QTableWidgetItem *item);
    
private:
    //func
    //��ʼ������
    void initVariable();

    //��ʼ��ui
    void initSetupUi();

    //��ʼ��layout
    void initLayout();

    //���ñ���д�С
    void setTableColumnWidth();

    //����sql��ѯ����
    QString createSqlCmd(int index);

    //���ò�ѯ�����ui
    void setSqlResultToUi(int rankIndex, int totalRow, QSqlQuery &query);

    //���ð�ť��Ч
    void setCtrlButtonEnabled(bool enabled);

    //variable
    Ui::TabRankListWidget *ui;
    static const int m_iRankIndex = 4;//�������͵�index
};

#endif // TABRANKLISTWIDGET_H
