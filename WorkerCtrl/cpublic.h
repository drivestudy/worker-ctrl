#ifndef CPUBLIC_H
#define CPUBLIC_H

#include <QObject>
#include <common.h>

class CPublic : public QObject
{
    Q_OBJECT

public:
    //匹配text是否符合正则表达式pattern的规则
    static bool isRegMatch(const QString &text, const QString &pattern);

    //将秒转换成天时分秒
    static QString secToFriendlyFormat(int sec);

    //将字节转换成容易看的单位
    static QString byteToFriendlyFormat(uint size);

    //是否是ip地址
    static bool isIpAddr(const QString &text);

    //是否是合法的数字
    static bool isDigit(const QString &text);

    //睡眠
    static void sleep(int secs);

    //设置开机启动
    static void setPowerBoot(bool enabled);
};

#endif // CPUBLIC_H
