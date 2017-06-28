// Md5.h: interface for the CMd5 class.
//
//////////////////////////////////////////////////////////////////////

#ifndef CMD5_H
#define CMD5_H

#include <iostream>
#include <string>
#include <cstring>

using namespace std;

typedef struct
{
	unsigned int count[2];
	unsigned int state[4];
	unsigned char buffer[64];
}MD5_CTX;

#define F(x,y,z) ((x & y) | (~x & z))
#define G(x,y,z) ((x & z) | (y & ~z))
#define H(x,y,z) (x^y^z)
#define I(x,y,z) (y ^ (x | ~z))
#define ROTATE_LEFT(x,n) ((x << n) | (x >> (32-n)))
#define FF(a,b,c,d,x,s,ac) \
{ \
	a += F(b,c,d) + x + ac; \
	a = ROTATE_LEFT(a,s); \
	a += b; \
}
#define GG(a,b,c,d,x,s,ac) \
{ \
	a += G(b,c,d) + x + ac; \
	a = ROTATE_LEFT(a,s); \
	a += b; \
}
#define HH(a,b,c,d,x,s,ac) \
{ \
	a += H(b,c,d) + x + ac; \
	a = ROTATE_LEFT(a,s); \
	a += b; \
}
#define II(a,b,c,d,x,s,ac) \
{ \
	a += I(b,c,d) + x + ac; \
	a = ROTATE_LEFT(a,s); \
	a += b; \
}

class CMd5
{
public:
	CMd5();
	virtual ~CMd5();
	string Md5(const string &src);//不能使用静态函数，保证线程安全
	static bool IsMd5Data(const string &md5);
	static string GetMd5(const string &src);

private:
	void MD5Init(MD5_CTX *context);
	void MD5Update(MD5_CTX *context,unsigned char *input,unsigned int inputlen);
	void MD5Final(MD5_CTX *context,unsigned char digest[16]);
	void MD5Transform(unsigned int state[4],unsigned char block[64]);
	void MD5Encode(unsigned char *output,unsigned int *input,unsigned int len);
	void MD5Decode(unsigned int *output,unsigned char *input,unsigned int len);
	string ToHex(const string &str);
	string FromHex(const string &hex);

private:
	MD5_CTX m_md5Ctx;
};
#endif //CMD5_H
