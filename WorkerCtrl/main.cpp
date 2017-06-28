#include <QtGui/QApplication>
#include <QtCore/QTextCodec>
#include <QtCore/QTranslator>
#include "workerctrlwidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //���ñ���
    QTextCodec::setCodecForTr(QTextCodec::codecForLocale());
    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());

    //����������Ϊ����
    QTranslator translator;
    translator.load(a.applicationDirPath() + "/qt_zh_CN.qm");
    a.installTranslator(&translator);

    WorkerCtrlWidget w;
    w.show();
    
    return a.exec();
}
