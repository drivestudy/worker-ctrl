#include <CJson.h>
#include <CPublic.h>

CJson::CJson()
{
}

CJson::~CJson()
{
}

bool CJson::IsValidJson(const string &data)
{
    if (data.empty())
    {
        return false;
    }
    const char *ptr = data.c_str();
    return Count(ptr, '{') == Count(ptr, '}') &&
           Count(ptr, '[') == Count(ptr, ']') &&
           Count(ptr, '"')%2 == 0;
}

int CJson::Count(const char *str, const char ch)
{
    int num = 0;
    char *p = (char *)str;
    while (*p)
    {
        if (ch ==*p)
        {
            ++num;
        }
        ++p;
    }
    return num;
}

void CJson::ParseJsonData(const string &jsondata, StringList &lst)
{
	Json::Reader		jsonReader;
	Json::FastWriter	jsonWriter;
	Json::Value			jsonValue;
	Json::Value 		tmpJson;
	string				tmpStr;
	StringVector		keyVec;

	if (!ParseStringToJsonValue(jsondata, jsonValue))
    {
        return;
    }

//    printf("%s\n\n", jsonValue.toStyledString().c_str());
	keyVec = jsonValue.getMemberNames();
	for (StringVector::iterator it = keyVec.begin(); it != keyVec.end(); ++it)
	{
		tmpStr.clear();
		tmpJson.clear();

		tmpJson[*it] = jsonValue[*it];
		tmpStr = jsonWriter.write(tmpJson);
		//将该站点的value和key再包装成一个对象，转换成string型，添加到lst
		lst.push_back(tmpStr);
	}
}

bool CJson::ParseStringToJsonValue(const string &data, Json::Value &value)
{
    if (!IsValidJson(data))
    {
        return false;
    }
    Json::Reader jsonReader;
    value.clear();
    return jsonReader.parse(data, value);
}

//void CJson::GetJsonKeys(const Json::Value &val, StringVector &vec)
//{
//    vec = val.getMemberNames();
//}

void CJson::JsonToMap(const string &jsonStr, StringMap &jsonMap)
{
    Json::Value jval;

	if (ParseStringToJsonValue(jsonStr, jval))
    {
		StringVector keyVec = jval.getMemberNames();
		jsonMap.clear();
		for (StringVector::iterator it = keyVec.begin(); it != keyVec.end(); ++it)
		{
			jsonMap[*it] = JsonValueToStringEx(jval[*it]);
		}
    }
}

void CJson::JsonToMap(const Json::Value &jval, StringMap &kvmap)
{
    kvmap.clear();
    if (!jval.isObject()) return;

	StringVector keyVec = jval.getMemberNames();
	for (StringVector::iterator it = keyVec.begin(); it != keyVec.end(); ++it)
	{
		kvmap[*it] = JsonValueToStringEx(jval[*it]);
	}
}

string CJson::JsonValueToStringEx(const Json::Value &value)
{
    char buf[256];
	string str;

	switch (value.type())
	{
	case Json::intValue:
		sprintf(buf, "%d", value.asInt());
		str = buf;
		break;
	case Json::uintValue:
		sprintf(buf, "%u", value.asUInt());
		str = buf;
		break;
	case Json::realValue:
		sprintf(buf, "%f", value.asDouble());
		str = buf;
		break;
	case Json::stringValue:
		str = value.asString();
		break;
    case Json::arrayValue:
        str = JsonWriteToString(value);
        break;
    case Json::objectValue:
        str = JsonWriteToString(value);
        break;
	default:
		break;
	}

	return str;
}

string CJson::JsonValueToString(const Json::Value &value)
{
    if (!value.isString())
    {
        return string();
    }
    return value.asString();
}

int CJson::JsonValueToInt(const Json::Value &value)
{
    if (!value.isInt())
    {
        return 0;
    }
    return value.asInt();
}

string CJson::JsonTest()
{
    Json::Value jval;
    Json::Value jarr;
    Json::Value jopt;
    Json::Value jsql;
    Json::Value jck;
    Json::FastWriter jwrite;

    jval["cmd"] = 2;
    jval["pwd"] = "123456";

    //opt.sql
    jsql["ip"] = "192.168.1.1";
    jsql["port"] = 3306;
    jsql["user"] = "root";
    jsql["pwd"] = "123456";
    jsql["dbname"] = "webhelper";

    //opt.cookiesrv
    jck.append("192.168.1.1:26128");
    jck.append("192.168.1.2:26128");
    jck.append("192.168.1.3:26128");
    jck.append("192.168.1.4:26128");

    //opt
    jopt["sql"] = jsql;
    jopt["cookiesrv"] = jck;

    jval["opt"] = jopt;

    //解析
//    cout << jval["opt"]["sql"]["ip"].asString() << endl;
//    cout << jval["opt"]["cookiesrv"][0].asString() << endl;
    Json::Value jtmp = jval["opt"]["cookiesrv"];
    if (jtmp.isNull()) {
        cout << "null" << endl;
    } else if (jtmp.isArray()) {
        cout << "arr" << endl;
        for (uint i = 0; i < jtmp.size(); i++) {
            cout << jtmp[i].asString() << endl;
        }
    }

    return "";
//    return jval.toStyledString();


//    Json::Value jsonValue;
//    Json::Value jsonArr;
//    Json::FastWriter jsonWriter;
//
////    jsonValue["arr"].append("aa");
////    jsonValue["arr"].append("bb");
////    jsonValue["arr"].append("cc");
//    jsonValue["checkout"] = 0;
//    return jsonWriter.write(jsonValue);

//    return jsonValue.toStyledString();
}

string CJson::JsonWriteToString(const Json::Value &jv)
{
	if (jv.isNull())
	{
		return "";
	} else {
	    Json::FastWriter jwrite;
	    return jwrite.write(jv);
	}

//	Json::FastWriter jw;
//	string strJson = jw.write(jv);
//	char ch;
//	ch = *(strJson.end()-1);
//	if ('\n' == ch)
//	{
//		strJson.erase(strJson.end()-1);
//	}
//	return strJson;
}
