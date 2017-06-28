#include "cpublic.h"

//ƥ��text�Ƿ����������ʽpattern�Ĺ���
bool CPublic::isRegMatch(const QString &text, const QString &pattern)
{
    QRegExp reg(pattern);
    return reg.exactMatch(text);
}

//����ת������ʱ����
QString CPublic::secToFriendlyFormat(int sec)
{
    int days = 0, houses = 0, minutes = 0, secs = 0;

    days    = sec / (24 * 3600);
    houses  = sec % (24 * 3600) / 3600;
    minutes = sec % 3600 / 60;
    secs    = sec % 60;

    QString strFormat;
    if (!days && !houses && !minutes) {
        strFormat = tr("%1��").arg(secs);
    } else if (!days && !houses) {
        strFormat = tr("%1��%2��").arg(minutes).arg(secs);
    } else if (!days) {
        strFormat = tr("%1ʱ%2��%3��").arg(houses).arg(minutes).arg(secs);
    } else {
        strFormat = tr("%1��%2ʱ%3��%4��").arg(days).arg(houses).arg(minutes).arg(secs);
    }
    return strFormat;
}

//���ֽ�ת�������׿��ĵ�λ
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

//�Ƿ���ip��ַ
bool CPublic::isIpAddr(const QString &text)
{
    return isRegMatch(text, "([1-9]|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])"
                      "(\\.(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])){3}");
}

//�Ƿ��ǺϷ�������
bool CPublic::isDigit(const QString &text)
{
    return isRegMatch(text, "\\d+");
}

//˯��
void CPublic::sleep(int secs)
{
    QElapsedTimer timer;
    timer.start();
    int tm = secs*1000;
    while (timer.elapsed() < tm) {
        qApp->processEvents();
    }
}

//���ÿ�������
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

