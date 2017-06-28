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
    //排行
    void slotBtnRank();
    //重置
    void slotBtnReset();
    //清空榜单
    void slotBtnClearBoard();
    //清空排行榜
    void slotBtnClearRankList();
    //排行信息下拉框
    void slotCurrentIndexChanged(int index);
    //双击复制单元格
    void slotItemDoubleClicked(QTableWidgetItem *item);
    
private:
    //func
    //初始化变量
    void initVariable();

    //初始化ui
    void initSetupUi();

    //初始化layout
    void initLayout();

    //设置表格列大小
    void setTableColumnWidth();

    //生成sql查询命令
    QString createSqlCmd(int index);

    //设置查询结果到ui
    void setSqlResultToUi(int rankIndex, int totalRow, QSqlQuery &query);

    //设置按钮有效
    void setCtrlButtonEnabled(bool enabled);

    //variable
    Ui::TabRankListWidget *ui;
    static const int m_iRankIndex = 4;//操作类型的index
};

#endif // TABRANKLISTWIDGET_H
