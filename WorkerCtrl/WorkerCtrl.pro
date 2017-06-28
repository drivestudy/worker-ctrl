#-------------------------------------------------
#
# Project created by QtCreator 2013-10-31T15:47:02
#
#-------------------------------------------------

QT       += core gui network sql xml

TARGET = WorkerCtrl
TEMPLATE = app


SOURCES += main.cpp\
    tabserverwidget.cpp \
    tablogwidget.cpp \
    tabsettingwidget.cpp \
    workerctrlwidget.cpp \
    cdataware.cpp \
    cpublic.cpp \
    selectcolumndialog.cpp \
    json/json_writer.cpp \
    json/json_valueiterator.inl \
    json/json_value.cpp \
    json/json_reader.cpp \
    json/json_internalmap.inl \
    json/json_internalarray.inl \
    monitorthread.cpp \
    caes.cpp \
    cjson.cpp \
    tabranklistwidget.cpp \
    inputpassworddialog.cpp \
    ccheckcookieserverthread.cpp \
    nofocusdelegate.cpp \
    ctcpsocket.cpp \
    tabsyncuserwidget.cpp \
    cmd5.cpp \
    csubmittaskthread.cpp \
    cupdatethread.cpp \
    chooseurldialog.cpp

HEADERS  += \
    tabserverwidget.h \
    tablogwidget.h \
    tabsettingwidget.h \
    common.h \
    workerctrlwidget.h \
    cdataware.h \
    cpublic.h \
    selectcolumndialog.h \
    json/writer.h \
    json/value.h \
    json/reader.h \
    json/json_batchallocator.h \
    json/json.h \
    json/forwards.h \
    json/features.h \
    json/config.h \
    json/autolink.h \
    monitorthread.h \
    caes.h \
    cjson.h \
    tabranklistwidget.h \
    inputpassworddialog.h \
    ccheckcookieserverthread.h \
    nofocusdelegate.h \
    ctcpsocket.h \
    tabsyncuserwidget.h \
    cmd5.h \
    csubmittaskthread.h \
    cupdatethread.h \
    chooseurldialog.h

FORMS    += \
    tabserverwidget.ui \
    tablogwidget.ui \
    tabsettingwidget.ui \
    workerctrlwidget.ui \
    selectcolumndialog.ui \
    tabranklistwidget.ui \
    inputpassworddialog.ui \
    tabsyncuserwidget.ui \
    chooseurldialog.ui

RESOURCES += \
    images/images.qrc

RC_FILE = images/logo.rc

DESTDIR = ../workerctrl_bin

OTHER_FILES += \
    json/sconscript \
    images/logo.rc
