原始代码来源于 https://github.com/shewang/PB2Json
在此代码基础上修改已适配项目需要


# protobuf2json
protobuf object  convert to jsoncpp object 

as3 and javascript tested


Json::Value value;
PB2Json::ToJsonObj(resp,value);

Json::FastWriter fast_writer;
std::string jsonStr = fast_writer.write(value);

proto::auth;
PB2Json::ToPb(auth,jsonStr);



