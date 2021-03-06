#include <stdlib.h>
#include <stdio.h>
#include "pb2as3json.h"
#include "log.h" 
#include <windows.h>
#undef GetMessage
/*
2016112210 wuding
狗屎as3的camelName，竟然对数字做了特殊处理，好吧，只能按它来了了，重新编c++的protobuf

20161025 wuding
check_type增加对js传过来的null类型，认为是合法的pb message

20161021 wuding
增加宏 AS3_CPP_64Bit_JSON_CONVERT
js,as3自己封装了一个64位的bigint，格式比较特别，虽然目前64bit好像没使用，但是防止以后用到，就支持一下as3,js了，这样js,as3就不用修改任何代码了，但是默认还是开启类型检查来检查js传过来的string,int不匹配问题的。暂不考虑兼容js中的string,int默认转换

20161014 wuding
js中有直接使用json中的子object，并没有做is null的检查，因此也加MessageObject的默认值吧Json::nullValue

20161011 wuding
考虑到js中读很多参数时没有做检查，特加入设置默认值功能
加入default支持,服务器有一些未设置的default值的，js中依赖从json中读，因此转json的时候，将默认值也加上
常用类型以及数组，加入未设置参数默认填充，int默认0,enum默认为0,string默认""，数组默认为[]

20161010 wuding
加入as3支持,狗屎as3使用的camelName，所以加了宏AS3_CPP_NAME_CONVERT来处理这个

20160925 wuding
修正array的bug，原来版本多了一层嵌套，修正枚举变量强制转换编绎的问题
*/  

//#define LOG_V 

//是否进行camelcase_name和lowercase_name的转换
#define AS3_CPP_NAME_CONVERT  1
//是否启用js,as3的64bit封装 格式为 json对象　{"high":33,"low":11} 
#define AS3_CPP_64Bit_JSON_CONVERT  1
//是否开启自动填充值功能，如果对端没有设置值的话，也填上默认值 ,默认int为０，string为"" enum为第一个值 数组为 []
#define AS3_CPP_FILLVALUE_NOTSET  1  


using namespace google::protobuf;
 
class Json64BitValue
{
public:
	Json64BitValue(const Json::Value &value)
	{ 
		m_Value64bit = 0;

#ifdef AS3_CPP_64Bit_JSON_CONVERT
		if(value.type() == Json::objectValue && !value["high"].isNull() && !value["low"].isNull())
		{
			UINT32 hi_value  = value["high"].asUInt();
			UINT32 low_value = value["low"].asUInt(); 
			 
			m_Value64bit = pack(hi_value, low_value);

			hi_value = high();
			low_value = low();

			m_bJson64bit = true;
		}
		else
		{
			m_Value64bit = value.asInt64();
			m_bJson64bit = false;
		}
#else
		m_Value64bit = value.asInt64();
		m_bJson64bit = false;
#endif
	}

	Json64BitValue(UINT64 value)
	{
		m_Value64bit = value;
	} 

	Json64BitValue(INT64 value)
	{
		m_Value64bit = (UINT64)value;
	}

	Json::Value ToInt64Json()
	{
#ifdef AS3_CPP_64Bit_JSON_CONVERT
		Json::Value json_value_64bit; 
		json_value_64bit["high"] = (UINT32)high();
		json_value_64bit["low"]  = (UINT32)low();
		return json_value_64bit;
#else
		return (Json::Int64)m_Value64bit;
#endif
	}

	Json::Value ToUInt64Json()
	{
#ifdef AS3_CPP_64Bit_JSON_CONVERT
		Json::Value json_value_64bit;
		json_value_64bit["high"] = (UINT32)high();
		json_value_64bit["low"]  = (UINT32)low();
		return json_value_64bit;
#else
		return (Json::UInt64)m_Value64bit;
#endif
	}
	

	inline UINT64 pack(UINT32 h, UINT32 l)
	{   
		return  ((UINT64)h) << 32 | ((UINT64)l & 0xffffffff);
	}

	inline UINT32 high() const
	{
		return (UINT32)((m_Value64bit & 0xFFFFFFFF00000000LL) >> 32);
	}

	inline UINT32 low() const
	{
		return (UINT32)(m_Value64bit & 0xFFFFFFFFLL); 
	}

	operator INT64() const
	{
		return (INT64)m_Value64bit;
	}

	operator UINT64() const
	{
		return m_Value64bit;
	}

	bool IsJson64Bit()
	{
#ifdef AS3_CPP_64Bit_JSON_CONVERT
		return m_bJson64bit;
#else
		return false;
#endif
	}

private:
	bool m_bJson64bit;
	UINT64 m_Value64bit;
};
 
namespace PB2AS3Json
{ 
	static bool g_enable_debug = false;
	
	
	/*
	char easytolower(char in)
	{
		if (in <= 'Z' && in >= 'A')
			return in - ('Z' - 'z');
		return in;
	}

	char easytoupper(char in)
	{
		if (in <= 'z' && in >= 'a')
			return in + ('Z' - 'z');
		return in;
	}

	//将驼峰式命名的字符串转换为下划线小写方式 helloWorld -> hellow_world
	static std::string underscoreName(const std::string &name) 
	{
		std::string result;
		if ( !name.empty() && name.length() > 0)
		{
			// 将第一个字符处理成小写
			result.append(tolower(name.substr(0, 1)));
			
			// 循环处理其余字符
			for (unsigned int i = 1; i < name.length(); i++) 
			{
				std::string s = name.substr(i, 1); 

				char char_first = s.at(0);
				char char_upper = easytoupper(char_first);

				// 在大写字母前添加下划线
				if (char_first == char_upper && !isdigit(char_first))
				{
					result.append("_");
				}
				// 其他字符直接转成小写
				result.append(tolower(s));
			}
		}

		return result;
	}

	//将下划线小写方式命名的字符串转换为驼峰式  hellow_world -> helloWorld 
	static std::string camelName(const std::string &name)
	{
		if (name.empty())
		{
			return "";
		}
		else if (name.find("_") == std::string::npos)
		{
			// 不含下划线，仅将首字母小写
			return (name.substr(0, 1)) + name.substr(1);
		}

		std::string result;
		// 用下划线将原始字符串分割
		std::vector<std::string> camels;
		splitString(name, "_", camels);

		for (unsigned int i = 0; i < camels.size(); i++)
		{
			std::string camel = camels[i];
			// 跳过原始字符串中开头、结尾的下换线或双重下划线
			if (camel.empty())
			{
				continue;
			}

			// 处理真正的驼峰片段
			if (result.length() == 0)
			{
				// 第一个驼峰片段，全部字母都小写
				result.append(tolower(camel));
			}
			else
			{
				// 其他的驼峰片段，首字母大写
				result.append(toupper(camel.substr(0, 1)));
				result.append(tolower(camel.substr(1)));
			}
		}

		return result;
	}

	void test()
	{
		std::string name = "hello_name";
		std::string camel_name = "helloName";

		std::string a = camelName(name);
		std::string b = underscoreName(camel_name);

		std::string c = camelName(b);
		std::string d = underscoreName(a);
	}
	*/

	void enableDebug(bool bDebug)
	{
		g_enable_debug = bDebug;
	}

	int ToJson(const Message& message, const FieldDescriptor* pFieldDescriptor, Json::Value& value, string& name_str, map<string, string>& key_map)
    {
        const Reflection* pReflection = message.GetReflection();
        int ret = 0; 

        switch (pFieldDescriptor->cpp_type())
        {
            case FieldDescriptor::CPPTYPE_INT32:
            {
                value[name_str] = pReflection->GetInt32(message, pFieldDescriptor);
                break;
            }
            case FieldDescriptor::CPPTYPE_UINT32:
            {
                value[name_str] = pReflection->GetUInt32(message, pFieldDescriptor);
                break;
            }
            case FieldDescriptor::CPPTYPE_INT64:
            {
				Json64BitValue v(pReflection->GetInt64(message, pFieldDescriptor));
				value[name_str] = v.ToInt64Json();
                break;
            }
            case FieldDescriptor::CPPTYPE_UINT64:
            {
				Json64BitValue v(pReflection->GetUInt64(message, pFieldDescriptor));
				value[name_str] = v.ToUInt64Json(); 
                break;
            }
            case FieldDescriptor::CPPTYPE_STRING:
            {
                value[name_str] = pReflection->GetString(message, pFieldDescriptor);
                break;
            }
            case FieldDescriptor::CPPTYPE_BOOL:
            {
                value[name_str] = pReflection->GetBool(message, pFieldDescriptor);
                break;
            }
            case FieldDescriptor::CPPTYPE_ENUM:
            {
                const EnumValueDescriptor* pEnumValueDes = NULL;
                pEnumValueDes = pReflection->GetEnum(message, pFieldDescriptor);
                if (NULL != pEnumValueDes)
                {
					value[name_str] =  pEnumValueDes->number();
                }
                else
                {
                    ret = 1;
                }
                break;
            }
            case FieldDescriptor::CPPTYPE_DOUBLE:
            {
                value[name_str] = pReflection->GetDouble(message, pFieldDescriptor);
                break;
            }
            case FieldDescriptor::CPPTYPE_FLOAT:
            {
                value[name_str] = pReflection->GetFloat(message, pFieldDescriptor);
                break;
            }
            case FieldDescriptor::CPPTYPE_MESSAGE:
            { 
                const Message& tmp = pReflection->GetMessage(message, pFieldDescriptor);
                Json::Value tmp_value;

                if (key_map.size() == 0)
                {
					ret = ToJsonObj(tmp, tmp_value);
                }
                else
                {
					ret = ToJsonObjMap(tmp, tmp_value, key_map);
                }

				

                value[name_str] = tmp_value;
                break;
            }
            default:
            {
                ret = 1;
                break;
            }
        }

        return ret;
    }

    int ToJsonArray(const Message& message, const FieldDescriptor* pFieldDescriptor, Json::Value& value, string& name_str, map<string, string>& key_map)
    {
        const Reflection* pReflection = message.GetReflection();
		//Json::Value tmp_value = Json::arrayValue;
		Json::Value tmp_value =Json::Value(Json::arrayValue);
		
        int ret = 0;
        for (int FieldNum = 0; FieldNum < pReflection->FieldSize(message, pFieldDescriptor); FieldNum++)
        {
            switch (pFieldDescriptor->cpp_type())
            {
                case FieldDescriptor::CPPTYPE_INT32:
                {
					tmp_value.append((pReflection->GetRepeatedInt32(message, pFieldDescriptor, FieldNum)));
                    break;
                }
                case FieldDescriptor::CPPTYPE_UINT32:
                {
					tmp_value.append((pReflection->GetRepeatedUInt32(message, pFieldDescriptor, FieldNum)));
                    break;
                }
                case FieldDescriptor::CPPTYPE_INT64:
                {
					Json64BitValue v(pReflection->GetRepeatedInt64(message, pFieldDescriptor, FieldNum));
					
					tmp_value.append(v.ToInt64Json());
                    break;
                }
                case FieldDescriptor::CPPTYPE_UINT64:
                {
					Json64BitValue v(pReflection->GetRepeatedUInt64(message, pFieldDescriptor, FieldNum));
					tmp_value.append(v.ToUInt64Json());
					break;
                }
                case FieldDescriptor::CPPTYPE_STRING:
                {
					tmp_value.append((pReflection->GetRepeatedString(message, pFieldDescriptor, FieldNum)));
                    break;
                }
                case FieldDescriptor::CPPTYPE_BOOL:
                {
					tmp_value.append((pReflection->GetRepeatedBool(message, pFieldDescriptor, FieldNum)));
                    break;
                }
                case FieldDescriptor::CPPTYPE_ENUM:
                {
                    const EnumValueDescriptor* pEnumValueDes = NULL;
                    pEnumValueDes = (EnumValueDescriptor*)pReflection->GetRepeatedEnum(message, pFieldDescriptor, FieldNum);
                    if (NULL == pEnumValueDes)
                    {
                        return 1;
                    }
					tmp_value.append((pEnumValueDes->number()));
                    break;
                }
                case FieldDescriptor::CPPTYPE_DOUBLE:
                {
					tmp_value.append((pReflection->GetRepeatedDouble(message, pFieldDescriptor, FieldNum)));
                    break;
                }
                case FieldDescriptor::CPPTYPE_FLOAT:
                {
					tmp_value.append((pReflection->GetRepeatedFloat(message, pFieldDescriptor, FieldNum)));
                    break;
                }
                case FieldDescriptor::CPPTYPE_MESSAGE:
                {
                    const Message& tmp = pReflection->GetRepeatedMessage(message, pFieldDescriptor, FieldNum);
                    
					if (0 != tmp.ByteSize())
					{
						Json::Value t_value;
						if (key_map.size() == 0)
						{
							ret = ToJsonObj(tmp, t_value);
						}
						else
						{
							ret = ToJsonObjMap(tmp, t_value, key_map);
						}
						 
						tmp_value.append(t_value);
					}
					
                    break;
                }
                default:
                {
                    return 1;
                }
            }
        }

        value[name_str] = tmp_value;
        
        return ret;
    }

    int ToJsonObj(const Message& message, std::string &json_string)
    {
        Json::Value value;

        int ret =ToJsonObj(message,value);

        Json::FastWriter fast_writer;
            
        json_string = fast_writer.write(value);

        return ret;
    }

    int ToJsonObj(const Message& message, Json::Value& value)
    {
        const Reflection* pReflection = message.GetReflection();
        const FieldDescriptor* pFieldDescriptor = NULL;
        bool bRepeated = false;

        //std::vector<const FieldDescriptor*> fields;
        //pReflection->ListFields(message, &fields);
		
		const google::protobuf::Descriptor *desc = message.GetDescriptor();
		if (!desc)
		{
			return 0;
		}

		size_t count = desc->field_count();

        int ret = 0;

        //for (size_t i = 0; i < fields.size(); i++)
		//{
		//pFieldDescriptor = fields[i];

		for (size_t i = 0; i < count; ++i)
		{
			pFieldDescriptor = desc->field(i);
			if (!pFieldDescriptor)
			{
				return 0;
			}
        
            bRepeated = pFieldDescriptor->is_repeated();
#ifdef AS3_CPP_NAME_CONVERT
			string name_str = pFieldDescriptor->camelcase_name(); 
#else
			string name_str = pFieldDescriptor->name();
#endif
            map<string, string> key_map;

            if (bRepeated)
            {
				if (pReflection->FieldSize(message, pFieldDescriptor) > 0)
					ret += ToJsonArray(message, pFieldDescriptor, value, name_str, key_map);
                
#ifdef AS3_CPP_FILLVALUE_NOTSET
				else
				{  
					value[name_str] = Json::Value(Json::arrayValue);
					ret += 0;
				} 
#endif

				continue;
            }
			else  if (pReflection->HasField(message, pFieldDescriptor) || pFieldDescriptor->has_default_value())
			{ 
				ret += ToJson(message, pFieldDescriptor, value, name_str, key_map);
			}
			else if (pFieldDescriptor->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE)
			{
				ret += ToJson(message, pFieldDescriptor, value, name_str, key_map);
			}  
			else if (pFieldDescriptor->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE)
			{ 
				//对象就直接一个空过去吧，免得js报错
				value[name_str] = Json::Value(Json::nullValue);  
			}
        }

        return ret;
    }

    int ToJsonObjMap(const Message& message, Json::Value& value, map<string, string>& key_map)
    {
        const Reflection* pReflection = message.GetReflection();
        const FieldDescriptor* pFieldDescriptor = NULL;
        bool bRepeated = false;

        std::vector<const FieldDescriptor*> fields;
        pReflection->ListFields(message, &fields);
        int ret = 0;

        for (size_t i = 0; i < fields.size(); i++)
        {
            pFieldDescriptor = fields[i];
            bRepeated = pFieldDescriptor->is_repeated();
            string name_str = pFieldDescriptor->containing_type()->name() + "." + pFieldDescriptor->name();

            map<string, string>::iterator it = key_map.find(name_str);
            if (it != key_map.end())
            {
                name_str = it->second;
            }
            else
            {
                name_str = pFieldDescriptor->name();
            }

            if (bRepeated)
            {
                ret += ToJsonArray(message, pFieldDescriptor, value, name_str, key_map);
                continue;
            }

            ret += ToJson(message, pFieldDescriptor, value, name_str, key_map);
        }

        return ret;
    }

    bool check_type(Json::ValueType value_type, FieldDescriptor::CppType type)
    {
		if (value_type == Json::nullValue)
		{
			if (FieldDescriptor::CPPTYPE_MESSAGE == type)
			{
				return true;
			}

			return false;
		}

        if (value_type == Json::objectValue)
        {
            if (FieldDescriptor::CPPTYPE_MESSAGE == type)
            {
                return true;
            }

            return false;
        }


        if ((value_type == Json::intValue) || (value_type == Json::uintValue))
        {
            if (type <= FieldDescriptor::CPPTYPE_UINT64)
            {
                return true;
            }

            if (type == FieldDescriptor::CPPTYPE_ENUM)
            {
                return true;
            }

            return false;
        }

        if (value_type == Json::stringValue)
        {
            if (type == FieldDescriptor::CPPTYPE_STRING)
            {
                return true;
            }

            return false;
        }

        if (value_type == Json::realValue)
        {
            if ((FieldDescriptor::CPPTYPE_DOUBLE == type) || (FieldDescriptor::CPPTYPE_FLOAT == type))
            {
                return true;
            }

            return false;
        }

        if (value_type == Json::booleanValue)
        {
            if (FieldDescriptor::CPPTYPE_BOOL == type)
            {
                return true;
            }

            return false;
        }

        return false;
    }

	std::string getPBCPPTypeName(google::protobuf::FieldDescriptor::CppType t)
	{ 
		switch (t)
		{
			case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
				return "CPPTYPE_INT32"; 
			case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
				return "CPPTYPE_INT64";
			case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
				return "CPPTYPE_UINT32";
			case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
				return "CPPTYPE_UINT64";
			case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
				return "CPPTYPE_DOUBLE";
			case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
				return "CPPTYPE_FLOAT";
			case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
				return "CPPTYPE_BOOL";
			case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
				return "CPPTYPE_ENUM";
			case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
				return "CPPTYPE_STRING";
			case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
				return "CPPTYPE_MESSAGE";
			default:
				return "MAX_CPPTYPE";
		} 
	}

	std::string getJsonValueTypeName(Json::ValueType t)
	{
		switch (t)
		{
			case Json::nullValue: ///< 'null' value
				return "nullValue";
			case Json::intValue:      ///< signed integer value
				return "intValue";
			case Json::uintValue:     ///< unsigned integer value
				return "uintValue";
			case Json::realValue:     ///< double value
				return "realValue";
			case Json::stringValue:   ///< UTF-8 string value
				return "stringValue";
			case Json::booleanValue:  ///< bool value
				return "booleanValue";
			case Json::arrayValue:    ///< array value (ordered list)
				return "arrayValue";
			case Json::objectValue:    ///< object value (collection of name/value pairs).
				return "objectValue"; 
			default:
				return "Unhown";
		} 
	} 

	int ToPbRepeated(const Json::Value &value, const FieldDescriptor *pFieldDescriptor, Message &message, map<string, string>& key_map, const std::string &json_string)
    {   
		if (!check_type(value[0].type(), pFieldDescriptor->cpp_type()))
		{
			bool breportError = true;
			//64位特殊处理一下，因为js不支持64位。收js的包特殊处理，回给js的不处理了 
			switch (pFieldDescriptor->cpp_type())
			{
			case FieldDescriptor::CPPTYPE_INT64:
			case FieldDescriptor::CPPTYPE_UINT64:
			{
				Json64BitValue v(value[0]); 

				if (v.IsJson64Bit())
				{
					breportError = false;
				}
			}
			break;
			default:
				break;
			}

			if (breportError)
			{
				std::string sErrorFormat;
				sErrorFormat += "[";
				sErrorFormat += pFieldDescriptor->name();
				sErrorFormat += "] type not match ";
				sErrorFormat += getJsonValueTypeName(value[0].type());
				sErrorFormat += "_" + getPBCPPTypeName(pFieldDescriptor->cpp_type());
				sErrorFormat += "|| " + json_string;

				if (g_enable_debug)
					::MessageBoxA(NULL, sErrorFormat.c_str(), "完蛋了", 0);

				LOG_V << sErrorFormat;
				return 1;
			}

		} 

        const Reflection *pReflection = message.GetReflection();
        EnumDescriptor      *pEnumDes = NULL;
        EnumValueDescriptor *pEnumValueDes = NULL;
        int ret = 0;
        for(unsigned int i = 0; i < value.size(); i++)
        {
            switch(pFieldDescriptor->cpp_type())
            {
                case FieldDescriptor::CPPTYPE_INT32:
                {
                    pReflection->AddInt32(&message, pFieldDescriptor, value[i].asInt());
                    break;
                }
                case FieldDescriptor::CPPTYPE_UINT32:
                {
                    pReflection->AddUInt32(&message, pFieldDescriptor, value[i].asUInt());
                    break;
                }
                case FieldDescriptor::CPPTYPE_INT64:
				{
					Json64BitValue v(value[i]);
					pReflection->AddInt64(&message, pFieldDescriptor,  v);
					break;
                }
                case FieldDescriptor::CPPTYPE_UINT64:
                {
					Json64BitValue v(value[i]);
					pReflection->AddUInt64(&message, pFieldDescriptor, v);
                    break;
                }
                case FieldDescriptor::CPPTYPE_STRING:
                {
                    pReflection->AddString(&message, pFieldDescriptor, value[i].asString());
                    break;
                }
                case FieldDescriptor::CPPTYPE_BOOL:
                {
                    pReflection->AddBool(&message, pFieldDescriptor, value[i].asBool());
                    break;
                }
                case FieldDescriptor::CPPTYPE_DOUBLE:
                {
                    pReflection->AddDouble(&message, pFieldDescriptor, value[i].asDouble());
                    break;
                }
                case FieldDescriptor::CPPTYPE_FLOAT:
                {
                    pReflection->AddFloat(&message, pFieldDescriptor, value[i].asFloat());
                    break;
                }
                case FieldDescriptor::CPPTYPE_ENUM:
                {
                    if ((pEnumDes = (EnumDescriptor *)pFieldDescriptor->enum_type()) == NULL)
                    {
                        return 1;
                    }

                    if ((pEnumValueDes = (EnumValueDescriptor *)pEnumDes->FindValueByNumber(value[i].asInt())) == NULL)
                    {
                        return 1;
                    }

                    pReflection->AddEnum(&message, pFieldDescriptor, pEnumValueDes);
                    break;
                }
                case FieldDescriptor::CPPTYPE_MESSAGE:
                {
                    Message *pmessage = pReflection->AddMessage(&message, pFieldDescriptor);
                    if (key_map.size() == 0)
                    {
						ret = ToPb(*pmessage, value[i], json_string);
                    }
                    else
                    {
						ret = ToPbMap(*pmessage, value[i], key_map, json_string);
                    }
                    break;
                }
                default:
                {
                    ret = 1;
                    break;
                }
            }
        }

        return ret;
    }

	int ToPbSingle(const Json::Value &value, const FieldDescriptor *pFieldDescriptor, Message &message, map<string, string>& key_map, const std::string &json_string)
    { 
		UINT64 value_64bit_default=0; 

        if (!check_type(value.type(), pFieldDescriptor->cpp_type()))
		{
			bool breportError = true;
			//64位特殊处理一下，因为js不支持64位。收js的包特殊处理，回给js的不处理了 
			switch (pFieldDescriptor->cpp_type())
			{
				case FieldDescriptor::CPPTYPE_INT64: 
				case FieldDescriptor::CPPTYPE_UINT64:
				{
					Json64BitValue v(value); 

					if (v.IsJson64Bit())
					{ 
						breportError = false;
					}
				}
				break;
				default:
					break;
			}

			if (breportError)
			{ 
				std::string sErrorFormat;
				sErrorFormat += "[";
				sErrorFormat += pFieldDescriptor->name();
				sErrorFormat += "] type not match ";
				sErrorFormat += getJsonValueTypeName(value.type());
				sErrorFormat += "_" + getPBCPPTypeName(pFieldDescriptor->cpp_type());
				sErrorFormat += "|| " + json_string;

				if (g_enable_debug)
					::MessageBoxA(NULL, sErrorFormat.c_str(), "完蛋了", 0);

				LOG_V << sErrorFormat;
				return 1;
			}

        }

        const Reflection *pReflection = message.GetReflection();
        EnumDescriptor      *pEnumDes = NULL;
        EnumValueDescriptor *pEnumValueDes = NULL;
        int ret = 0;
        switch(pFieldDescriptor->cpp_type())
        {
            case FieldDescriptor::CPPTYPE_INT32:
            {
                pReflection->SetInt32(&message, pFieldDescriptor, value.asInt());
                break;
            }
            case FieldDescriptor::CPPTYPE_UINT32:
            {
                pReflection->SetUInt32(&message, pFieldDescriptor, value.asUInt());
                break;
            }
            case FieldDescriptor::CPPTYPE_INT64:
            {
				pReflection->SetInt64(&message, pFieldDescriptor, (INT64)value_64bit_default);
				break;
            }
            case FieldDescriptor::CPPTYPE_UINT64:
            {
				pReflection->SetUInt64(&message, pFieldDescriptor, value_64bit_default);
                break;
            }
            case FieldDescriptor::CPPTYPE_STRING:
            {
                pReflection->SetString(&message, pFieldDescriptor, value.asString());
                break;
            }
            case FieldDescriptor::CPPTYPE_BOOL:
            {
                pReflection->SetBool(&message, pFieldDescriptor, value.asBool());
                break;
            }
            case FieldDescriptor::CPPTYPE_DOUBLE:
            {
                pReflection->SetDouble(&message, pFieldDescriptor, value.asDouble());
                break;
            }
            case FieldDescriptor::CPPTYPE_FLOAT:
            {
                pReflection->SetFloat(&message, pFieldDescriptor, value.asFloat());
                break;
            }
            case FieldDescriptor::CPPTYPE_ENUM:
            {
                if ((pEnumDes = (EnumDescriptor *)pFieldDescriptor->enum_type()) == NULL)
                {
                    return 1;
                }

                if ((pEnumValueDes = (EnumValueDescriptor *)pEnumDes->FindValueByNumber(value.asInt())) == NULL)
                {
                    return 1;
                }

                pReflection->SetEnum(&message, pFieldDescriptor, pEnumValueDes);
                break;
            }
            case FieldDescriptor::CPPTYPE_MESSAGE:
            {
                Message *pmessage = pReflection->MutableMessage(&message, pFieldDescriptor);
                if (key_map.size() == 0)
                {
					ret = ToPb(*pmessage, value, json_string);
                }
                else
                {
					ret = ToPbMap(*pmessage, value, key_map, json_string);
                }

                break;
            }
            default:
            {
                ret = 1;
                break;
            }
        }

        return ret;
    }

    int ToPb(Message& message, const std::string &json_string)
    {
        Json::Reader reader;
        Json::Value value;

        if (reader.parse(json_string, value))
        {    
			return ToPb(message, value, json_string);
        }  
        return 1;
    }

	int ToPb(Message& message, const Json::Value& value, const std::string &json_string)
    {
        int ret = 0;
        const Descriptor* pDescriptor = message.GetDescriptor();
        const FieldDescriptor* pFieldDescriptor = NULL;
        bool bRepeated = false;
        map<string, string> key_map;

        for(int i = 0; i < pDescriptor->field_count(); i++)
        {
            string name_str;
            pFieldDescriptor = (FieldDescriptor *)pDescriptor->field(i);
#ifdef AS3_CPP_NAME_CONVERT
			name_str = pFieldDescriptor->camelcase_name();
#else
			name_str = pFieldDescriptor->name();
#endif
			//std::string nn= pFieldDescriptor->lowercase_name();
            if (!value.isMember(name_str))
            {
                continue;
            }

            const Json::Value &field = value[name_str.c_str()];

            bRepeated = pFieldDescriptor->is_repeated();
            if ((bRepeated && !field.isArray()) || (!bRepeated && field.isArray()))
            {
                ret += 1;
                continue;
            }

            if (bRepeated)
            {
				ret += ToPbRepeated(field, pFieldDescriptor, message, key_map, json_string);
                continue;
            }

			ret += ToPbSingle(field, pFieldDescriptor, message, key_map, json_string);
        }

        return ret;
    }
    
	int ToPb2(Message& message, const Json::Value& value, const std::string &json_string)
    {
        int ret = 0;
        const Descriptor* pDescriptor = message.GetDescriptor();
        const FieldDescriptor* pFieldDescriptor = NULL;
        bool bRepeated = false;
        map<string, string> key_map;

        Json::Value::Members members(value.getMemberNames());
        for (Json::Value::Members::iterator it = members.begin(); it != members.end(); ++it)
        {
            const std::string &name = *it;
            if (value[name].isNull())
            {
                continue;
            }

            pFieldDescriptor = pDescriptor->FindFieldByName(name);
            if (NULL == pFieldDescriptor)
            {
                ret += 1;
                continue;
            }

            bRepeated = pFieldDescriptor->is_repeated();
            if ((bRepeated && !value[name].isArray()) || (!bRepeated && value[name].isArray()))
            {
                ret += 1;
                continue;
            }

            if (bRepeated)
            {
				ret += ToPbRepeated(value[name], pFieldDescriptor, message, key_map, json_string);
                continue;
            }

			ret += ToPbSingle(value[name], pFieldDescriptor, message, key_map, json_string);
        }

        return ret;
    }

	int ToPbMap(Message& message, const Json::Value& value, map<string, string>& key_map, const std::string &json_string)
    {
        int ret = 0;
        const Descriptor* pDescriptor = message.GetDescriptor();
        const FieldDescriptor* pFieldDescriptor = NULL;
        bool bRepeated = false;

        for(int i = 0; i < pDescriptor->field_count(); i++)
        {
            pFieldDescriptor = (FieldDescriptor *)pDescriptor->field(i);
            string name_str = pFieldDescriptor->containing_type()->name() + "." + pFieldDescriptor->name();
            
            map<string, string>::iterator it = key_map.find(name_str);
            if (it != key_map.end())
            {
                name_str = it->second;
            }
            else
            {
                name_str = pFieldDescriptor->name();
            }

            if (!value.isMember(name_str))
            {
                continue;
            }

            const Json::Value &field = value[name_str.c_str()];

            bRepeated = pFieldDescriptor->is_repeated();
            if ((bRepeated && !field.isArray()) || (!bRepeated && field.isArray()))
            {
                ret += 1;
                continue;
            }

            if (bRepeated)
            {
				ret += ToPbRepeated(field, pFieldDescriptor, message, key_map, json_string);
                continue;
            }

			ret += ToPbSingle(field, pFieldDescriptor, message, key_map, json_string);
        }

        return ret;
    }
}



 