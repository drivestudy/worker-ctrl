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
    //查询按钮
    void slotBtnSearch();

    //删除按钮
    void slotBtnDelete();

    //计数按钮
    void slotBtnCount();

    //重置
    void slotBtnReset();

    //清空查询结果按钮
    void slotBtnClearResult();

    //查询结果双击信号
    void slotItemDoubleClicked(QTableWidgetItem *item);

    //水平表头的右键菜单触发
    void slotCustomContextMenuRequested(const QPoint &pos);

    //选择列Action
    void slotSltColAction();
    
private:
    //func
    //初始化变量
    void initVariable();

    //初始化ui
    void initSetupUi();

    //初始化layout
    void initLayout();

    //交互合法判断
    bool isValidData();

    //根据界面提供的条件生成响应的sql语句,查询
    QString createSqlCmdSearch();

    //根据界面提供的条件生成响应的sql语句,计数
    QString createSqlCmdCount();

    //根据界面提供的条件生成响应的sql语句,删除
    QString createSqlCmdRemove();

    //拼接条件部分
    QString createSqlCondition();

    //设置数据库查询结果到ui
    void setSqlResultToTable(QSqlQuery &query);

    //用户类型转换
    QString usertypeString(uint type);

    //设置表格列大小
    void setTableColumnWidth();

    //控制按钮的启用与禁用
    void setCtrlButtonEnabled(bool enabled);

    //清空查询结果
    void clearSearchResult();

    //读取xml配置文件
    void readXml(QList<int> &intList);

    //写入xml配置文件
    void writeXml(const QList<int> &intList);

    //添加当前服务器到下拉框
    void addCurrentServerIp();

    //variable
    Ui::TabLogWidget *ui;
    QAction *m_pSltColAction;//选择列
    QMenu m_tableMenu;//表格头的右键菜单
};

#endif // TABLOGWIDGET_H
