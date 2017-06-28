#ifndef CPUBLIC_H
#define CPUBLIC_H

#include <QObject>
#include <common.h>

class CPublic : public QObject
{
    Q_OBJECT

public:
    //ƥ��text�Ƿ����������ʽpattern�Ĺ���
    static bool isRegMatch(const QString &text, const QString &pattern);

    //����ת������ʱ����
    static QString secToFriendlyFormat(int sec);

    //���ֽ�ת�������׿��ĵ�λ
    static QString byteToFriendlyFormat(uint size);

    //�Ƿ���ip��ַ
    static bool isIpAddr(const QString &text);

    //�Ƿ��ǺϷ�������
    static bool isDigit(const QString &text);

    //˯��
    static void sleep(int secs);

    //���ÿ�������
    static void setPowerBoot(bool enabled);
};

#endif // CPUBLIC_H
