#ifndef CJSON_H
#define CJSON_H

#include <iostream>
#include <map>
#include <list>
#include <string>
#include <json/value.h>
#include <json/writer.h>
#include <json/reader.h>

using namespace std;

typedef map<string, string> StringMap;
typedef list<string> StringList;
typedef vector<string> StringVector;

class CJson
{
public:
    CJson();
    virtual ~CJson();

    static bool         IsValidJson(const string &data);//简单的判断
    static int          Count(const char *str, const char ch);//计算ch到个数
    static void         ParseJsonData(const string &jsondata, StringList &lst);
    static bool         ParseStringToJsonValue(const string &data, Json::Value &value);
//    static void         GetJsonKeys(const Json::Value &val, StringVector &vec);
    static void         JsonToMap(const string &jsonStr, StringMap &jsonMap);
    static void         JsonToMap(const Json::Value &jval, StringMap &kvmap);
    static string       JsonValueToStringEx(const Json::Value &value);//把所有json数据转化成string
    static string       JsonValueToString(const Json::Value &value);
    static int          JsonValueToInt(const Json::Value &value);
    static string       JsonTest();
    static string		JsonWriteToString(const Json::Value &jv);
};
#endif //CJSON_H
