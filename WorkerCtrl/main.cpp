#include <QtGui/QApplication>
#include <QtCore/QTextCodec>
#include <QtCore/QTranslator>
#include "workerctrlwidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //设置编码
    QTextCodec::setCodecForTr(QTextCodec::codecForLocale());
    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());

    //设计软件语言为中文
    QTranslator translator;
    translator.load(a.applicationDirPath() + "/qt_zh_CN.qm");
    a.installTranslator(&translator);

    WorkerCtrlWidget w;
    w.show();
    
    return a.exec();
}
