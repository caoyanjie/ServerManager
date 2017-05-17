#include "PubHeader.h"

#include "ProcessJson.h"

#include "RapidJson/reader.h"
#include "RapidJson/stringbuffer.h"

#include <stdarg.h>
#include <sstream>

RakNetServer::ProcessJson::ProcessJson()
{ //{{{
} //}}}

RakNetServer::ProcessJson::~ProcessJson()
{ //{{{
} //}}}

// 把字符串编码成 Json 字符串
std::string RakNetServer::ProcessJson::enecodeJson(std::string format, ...)
{ //{{{
	va_list arg_ptr;
	va_start(arg_ptr, format);
	std::string json = "{";
	for (int start = 0, index = 0; index = format.find("%", start), index != (int)std::string::npos; start = index + 1)
	{
		unsigned index_next = format.find("%", index + 1);
		if (index_next == std::string::npos)
		{
			index_next = format.length();
		}
		std::string flag = format.substr(index + 1, index_next - index - 1);
		if (flag == std::string("actn"))
		{
			int value = va_arg(arg_ptr, int);
			std::ostringstream action;
			action << "\"action\":" << value;
			json.append(action.str());
		}
		else if (flag == std::string("ndID"))
		{
			int value = va_arg(arg_ptr, int);
			std::ostringstream nodeID;
			nodeID << ",\"ndID\":" << value;
			json.append(nodeID.str());
		}
		else if (flag == std::string("ocup"))
		{
			int value = va_arg(arg_ptr, int);
			std::ostringstream nodeRemove;
			if (value)
				nodeRemove << ",\"ndOcup\":" << "true";
			else
				nodeRemove << ",\"ndOcup\":" << "false";
			json.append(nodeRemove.str());
		}
		else if (flag == std::string("enbl"))
		{
			int value = va_arg(arg_ptr, int);
			std::ostringstream nodeEnable;
			nodeEnable << ",\"ndEnbl\":" << value;
			json.append(nodeEnable.str());
		}
//		else if (flag == "trsf")
//		{
//			Mat4Data value = va_arg(arg_ptr, Mat4Data);
//			String mat;
//			mat = String::format(",\"transMat4\":[%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f]", value.m00, value.m10, value.m20, value.m30, value.m01, value.m11, value.m21, value.m31, value.m02, value.m12, value.m22, value.m32, value.m03, value.m13, value.m23, value.m33);
//			json.append(mat.get());
//		}
		else if (flag == std::string("delt"))
		{
			int value = va_arg(arg_ptr, int);
			std::ostringstream nodeRemove;
			if (value)
				nodeRemove << ",\"ndDelt\":" << "true";
			else
				nodeRemove << ",\"ndDelt\":" << "false";
			json.append(nodeRemove.str());
		}
		else if (flag == std::string("trackerName"))
		{
			const char* value = va_arg(arg_ptr, const char*);
			std::ostringstream track;
			track << ",\"trackerName\":" << "\"" << value << "\"";
			json.append(track.str());
		}
		else if (flag == std::string("trackPlayerState"))
		{
			int value = va_arg(arg_ptr, int);
			std::ostringstream track;
			track << ",\"trackPlayerState\":" << value;
			json.append(track.str());
		}
		else if (flag == std::string("filename"))
		{
			const char* value = va_arg(arg_ptr, const char*);
			std::ostringstream filename;
			filename << ",\"filename\":\"" << value << "\"";
			json.append(filename.str());
		}
		else
		{
			printf("assert: %s\n", flag.c_str());
			assert(false);
		}
	}
	json.append("}");
	va_end(arg_ptr);

	return json;
} //}}}

// 解码 Json 字符串
RakNetServer::DecJsnRst RakNetServer::ProcessJson::decodeJson(std::string jsonString)
{ //{{{
	MyHandler hander;
	rapidjson::Reader reader;
	rapidjson::StringStream strStream(jsonString.c_str());
	reader.Parse(strStream, hander);
	//return hander.result;
	return hander.decJsnRst;
} //}}}

std::string RakNetServer::ProcessJson::alterJsonValue(std::string sourceJsonStr, std::string jsonKey, std::string newValue)
{ //{{{
	auto startIndex = sourceJsonStr.find(jsonKey.c_str());
	auto endIndex = sourceJsonStr.find(",", startIndex + jsonKey.size() + 1);
	if (endIndex == std::string::npos)
	{
		endIndex = sourceJsonStr.find("}", startIndex + jsonKey.size() + 1);
	}
	assert(startIndex != std::string::npos && endIndex != std::string::npos && "find json key failed");

	startIndex += jsonKey.size() + 2;
	sourceJsonStr.replace(startIndex, endIndex - startIndex, newValue);

	return sourceJsonStr;
} //}}}

std::string RakNetServer::ProcessJson::getJsonStringValue(std::string sourceJsonStr, std::string jsonKey)
{
	std::string result = "";

	auto startIndex = sourceJsonStr.find(jsonKey.c_str());
	if (startIndex == std::string::npos)
	{
		return result;
	}

	auto endIndex = sourceJsonStr.find(",", startIndex + jsonKey.size() + 1);
	if (endIndex == std::string::npos)
	{
		endIndex = sourceJsonStr.find("}", startIndex + jsonKey.size() + 1);
	}
	assert(startIndex != std::string::npos && endIndex != std::string::npos && "find json key failed");

	startIndex += jsonKey.size() + 2;
	result = sourceJsonStr.substr(startIndex, endIndex - startIndex);
	if (result.find_first_of("\"") == 0 && result.find_last_of("\"") == result.size()-1)
	{
		result = result.substr(1, result.size() - 2);
	}

	return result;
}

