
#ifndef __AS3_PROTOBUF_TO_JSON_H_
#define __AS3_PROTOBUF_TO_JSON_H_
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <json/json.h>
#include <map>
/*
#if __BYTE_ORDER == __BIG_ENDIAN
#ifndef ntohll
#define ntohll(x)   (x)
#endif
#ifndef htonll
#define htonll(x)   (x)
#endif
#else
#if __BYTE_ORDER == __LITTLE_ENDIAN
#ifndef ntohll
#define ntohll(x)   __bswap_64 (x)
#endif
#ifndef htonll
#define htonll(x)   __bswap_64 (x)
#endif
#endif
#endif

#if !defined(__GNUC__) || (__GNUC__ == 2 && __GNUC_MINOR__ < 96)
#ifndef likely
#define likely(x)       if((x))
#endif
#ifndef unlikely
#define unlikely(x)     if((x))
#endif
#else
#ifndef likely
#define likely(x)       if(__builtin_expect((x) != 0, 1))
#endif
#ifndef unlikely
#define unlikely(x)     if(__builtin_expect((x) != 0, 0))
#endif
#endif
*/ 
namespace PB2AS3Json
{
	//void test();
	void enableDebug(bool bDebug);

	int ToJsonObj(const google::protobuf::Message& message, std::string &json_string);
	int ToJsonObj(const google::protobuf::Message& message, Json::Value& value);
	int ToJson(const google::protobuf::Message& message, const google::protobuf::FieldDescriptor* pFieldDescriptor, Json::Value& value, std::map<std::string, std::string>& key_map);
	int ToJsonArray(const google::protobuf::Message& message, const google::protobuf::FieldDescriptor* pFieldDescriptor, Json::Value& value);
	int ToJsonObjMap(const google::protobuf::Message& message, Json::Value& value, std::map<std::string, std::string>& key_map);
	
	
	int ToPbSingle(const Json::Value &value, const google::protobuf::FieldDescriptor *pFieldDescriptor, google::protobuf::Message &message, std::map<std::string, std::string>& key_map, const std::string &json_string);
	int ToPbRepeated(const Json::Value &value, const google::protobuf::FieldDescriptor *pFieldDescriptor, google::protobuf::Message &message, std::map<std::string, std::string>& key_map, const std::string &json_string);
	int ToPb(google::protobuf::Message& message, const std::string &json_string);
	int ToPb(google::protobuf::Message& message, const Json::Value& value, const std::string &json_string);
	int ToPb2(google::protobuf::Message& message, const Json::Value& value, const std::string &json_string);
	int ToPbMap(google::protobuf::Message& message, const Json::Value& value, std::map<std::string, std::string>& key_map, const std::string &json_string);
}
#endif

