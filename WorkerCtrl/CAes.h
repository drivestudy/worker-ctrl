#ifndef CAES_H
#define CAES_H

#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

/*encryption key length*/
#define LEVEL_128_KEYLEN	16
#define LEVEL_192_KEYLEN	24
#define LEVEL_256_KEYLEN	32

/*encrypt level*/
#define LEVEL_128			128
#define LEVEL_192			192
#define LEVEL_256			256

//默认密钥
#define DEFAULT_AESKEY      (U8 *)("ABCDabcd1234!@#$")

#define AESKEY_1            (U8 *)("N60wk418f7H9t02C")
#define AESKEY_2            (U8 *)("SbWj5JfuvyMlbhuE70XFphpc")
#define AESKEY_3            (U8 *)("P2C0n17RRbp4bNK5s461bv6are9sym10")

#ifndef U8
typedef unsigned char U8;
#endif

#ifndef U32
typedef unsigned long int U32;
#endif

typedef struct
{
    U32 erk[64];     /* encryption round keys */
    U32 drk[64];     /* decryption round keys */
    int nr;             /* number of rounds */
} aes_context;

class CAes
{
public:
    CAes();
	//设置Aes密钥与加密级别
	bool	SetAesKey(const U8 *key, int level = LEVEL_128);

	//加解密原始数据
	void	Encrypt(const string &src, string &dst);
    void	Decrypt(const string &src, string &dst);

	//加密成十六进制 解密
    void	EncryptToHex(const string &src, string &dst);
    bool	DecryptFromHex(const string &src, string &dst);

	//加解密文件
	bool	EncryptFile(const char *path);
    bool	DecryptFile(const char *path);

	//文件加密成十六进制 解密 适合小文件的加解密
	bool	EncryptFileToHex(const char *path);
	bool	DecryptFileFromHex(const char *path);

    //默认密钥加解密
    static void EncryptToHexDefault(const string &src, string &dst);
    static bool DecryptFromHexDefault(const string &src, string &dst);

    //不同级别的加解密,密钥由程序确定
    static void EncryptToHexLevel(int level, const string &src, string &dst);
    static bool DecryptFromHexLevel(int level, const string &src, string &dst);

	//十六进制的转换
	static string ToHex(const string &str);
	static string FromHex(const string &hex);

	//判断
	static bool IsHexData(const string &hex);
	static bool IsAesData(const string &aes);

protected:
    void	encrypt_data(U8 *src, U8 *dst, int len, int level);
    void	decrypt_data(U8 *src, U8 *dst, int len, int level);
    int		aes_set_key(aes_context *ctx, U8 *key, int nbits);
    void	aes_encrypt(aes_context *ctx, U8 input[16], U8 output[16]);
    void	aes_decrypt(aes_context *ctx, U8 input[16], U8 output[16]);

private:
	aes_context m_ctx;
	int			m_iLevel;
};
#endif //CAES_H
