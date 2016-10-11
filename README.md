# protobuf2json
protobuf object  convert to jsoncpp object 

as3 and javascript tested


Json::Value value;
PB2Json::ToJsonObj(resp,value);

Json::FastWriter fast_writer;
std::string jsonStr = fast_writer.write(value);

proto::auth;
PB2Json::ToPb(auth,jsonStr);

