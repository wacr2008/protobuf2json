#include <stdlib.h>
#include <stdio.h>
#include "pb2as3json.h"
 

/*
20160925 wuding 
修正array的bug，原来版本多了一层嵌套，修正枚举变量强制转换编绎的问题

20161010 wuding 
加入as3支持,狗屎as3使用的camelName，所以加了宏AS3_CPP_NAME_CONVERT来处理这个

20161011 wuding 
考虑到js中读很多参数时没有做检查，特加入设置默认值功能
加入default支持,服务器有一些未设置的default值的，js中依赖从json中读，因此转json的时候，将默认值也加上
常用类型以及数组，加入未设置参数默认填充，int默认0,enum默认为0,string默认""，数组默认为[]
*/


//是否进行camelcase_name和lowercase_name的转换
#define AS3_CPP_NAME_CONVERT  1

//是否开启自动填充值功能，如果对端没有设置值的话，也填上默认值 ,默认int为０，string为"" enum为第一个值 数组为 []
#define AS3_CPP_FILLVALUE_NOTSET  1  


using namespace google::protobuf;

namespace PB2AS3Json
{ 
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
                value[name_str] = (Json::Int64)pReflection->GetInt64(message, pFieldDescriptor);
                break;
            }
            case FieldDescriptor::CPPTYPE_UINT64:
            {
                value[name_str] = (Json::UInt64)pReflection->GetUInt64(message, pFieldDescriptor);
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
					tmp_value.append(((Json::Int64)pReflection->GetRepeatedInt64(message, pFieldDescriptor, FieldNum)));
                    break;
                }
                case FieldDescriptor::CPPTYPE_UINT64:
                {
					tmp_value.append(((Json::UInt64)pReflection->GetRepeatedUInt64(message, pFieldDescriptor, FieldNum)));
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

    int ToPbRepeated(const Json::Value &value, const FieldDescriptor *pFieldDescriptor, Message &message, map<string, string>& key_map)
    {
        if (!check_type(value[0].type(), pFieldDescriptor->cpp_type()))
        {
            return 1;
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
                    pReflection->AddInt64(&message, pFieldDescriptor, value[i].asInt64());
                    break;
                }
                case FieldDescriptor::CPPTYPE_UINT64:
                {
                    pReflection->AddUInt64(&message, pFieldDescriptor, value[i].asUInt64());
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
                        ret = ToPb(*pmessage, value[i]);
                    }
                    else
                    {
                        ret = ToPbMap(*pmessage, value[i], key_map);
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

    int ToPbSingle(const Json::Value &value, const FieldDescriptor *pFieldDescriptor, Message &message, map<string, string>& key_map)
    {
        if (!check_type(value.type(), pFieldDescriptor->cpp_type()))
        {
            return 1;
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
                pReflection->SetInt64(&message, pFieldDescriptor, value.asInt64());
                break;
            }
            case FieldDescriptor::CPPTYPE_UINT64:
            {
                pReflection->SetUInt64(&message, pFieldDescriptor, value.asUInt64());
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
                    ret = ToPb(*pmessage, value);
                }
                else
                {
                    ret = ToPbMap(*pmessage, value, key_map);
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
            return ToPb(message,value);
        }  
        return 1;
    }

    int ToPb(Message& message, const Json::Value& value)
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
                ret += ToPbRepeated(field, pFieldDescriptor, message, key_map);
                continue;
            }

            ret += ToPbSingle(field, pFieldDescriptor, message, key_map);
        }

        return ret;
    }
    
    int ToPb2(Message& message, const Json::Value& value)
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
                ret += ToPbRepeated(value[name], pFieldDescriptor, message, key_map);
                continue;
            }

            ret += ToPbSingle(value[name], pFieldDescriptor, message, key_map);
        }

        return ret;
    }

    int ToPbMap(Message& message, const Json::Value& value, map<string, string>& key_map)
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
                ret += ToPbRepeated(field, pFieldDescriptor, message, key_map);
                continue;
            }

            ret += ToPbSingle(field, pFieldDescriptor, message, key_map);
        }

        return ret;
    }
}



 