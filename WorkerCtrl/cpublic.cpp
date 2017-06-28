#include "cpublic.h"

//匹配text是否符合正则表达式pattern的规则
bool CPublic::isRegMatch(const QString &text, const QString &pattern)
{
    QRegExp reg(pattern);
    return reg.exactMatch(text);
}

//将秒转换成天时分秒
QString CPublic::secToFriendlyFormat(int sec)
{
    int days = 0, houses = 0, minutes = 0, secs = 0;

    days    = sec / (24 * 3600);
    houses  = sec % (24 * 3600) / 3600;
    minutes = sec % 3600 / 60;
    secs    = sec % 60;

    QString strFormat;
    if (!days && !houses && !minutes) {
        strFormat = tr("%1秒").arg(secs);
    } else if (!days && !houses) {
        strFormat = tr("%1分%2秒").arg(minutes).arg(secs);
    } else if (!days) {
        strFormat = tr("%1时%2分%3秒").arg(houses).arg(minutes).arg(secs);
    } else {
        strFormat = tr("%1天%2时%3分%4秒").arg(days).arg(houses).arg(minutes).arg(secs);
    }
    return strFormat;
}

//将字节转换成容易看的单位
QString CPublic::byteToFriendlyFormat(uint size)
{
    static const uint kb = 1024;
    static const uint mb = 1024 * 1024;
    static const uint gb = 1024 * 1024 * 1024;
    QString strFormat;

    if (size >= 0 && size < kb) {
        strFormat = tr("%1B").arg(size);
    } else if (size >= kb && size < mb) {
        strFormat = tr("%1KB").arg(1.0*size/kb, 0, 'f', 2);
    } else if (size >= mb && size < gb) {
        strFormat = tr("%1MB").arg(1.0*size/mb, 0, 'f', 2);
    } else if (size >= gb) {
        strFormat = tr("%1GB").arg(1.0*size/gb, 0, 'f', 2);
    }
    return strFormat;
}

//是否是ip地址
bool CPublic::isIpAddr(const QString &text)
{
    return isRegMatch(text, "([1-9]|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])"
                      "(\\.(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])){3}");
}

//是否是合法的数字
bool CPublic::isDigit(const QString &text)
{
    return isRegMatch(text, "\\d+");
}

//睡眠
void CPublic::sleep(int secs)
{
    QElapsedTimer timer;
    timer.start();
    int tm = secs*1000;
    while (timer.elapsed() < tm) {
        qApp->processEvents();
    }
}

//设置开机启动
void CPublic::setPowerBoot(bool enabled)
{
    QSettings reg("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);

    if (enabled) {
        reg.setValue("workerctrl", QDir::toNativeSeparators(qApp->applicationFilePath()));
    }
    else {
        reg.remove("workerctrl");
    }
}

