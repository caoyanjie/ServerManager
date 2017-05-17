#pragma once
#include "DefineTypes.h"
#include <assert.h>

namespace RakNetServer {
	class ProcessJson
	{
	public:
		//{{{
		ProcessJson();
		~ProcessJson();

		std::string enecodeJson(std::string format, ...);
		DecJsnRst decodeJson(std::string jsonString);
		static std::string alterJsonValue(std::string sourceJsonStr, std::string jsonKey, std::string newValue);
		static std::string getJsonStringValue(std::string sourceJsonStr, std::string jsonKey);
		//}}}

	private:
		//{{{
		typedef unsigned			SizeType;
		typedef unsigned long long	uint64_t;
		typedef long long			int64_t;

		//enum JsonKey : int
		//{ //{{{
		//	Key_null,
		//	Key_action,
		//	Key_ndID,
		//	Key_ndOcpd,
		//	Key_ndEnable,
		//	Key_ndDelete,
		//	Key_transMat4,
		//	
		//	Key_author,
		//	Key_adsorb,
		//	Key_trackNm,
		//	Key_tpState
		//}; //}}}

		struct MyHandler
		{ //{{{
			//const char*				curtKey;
			std::string					curtKey;
			DecJsnRst					decJsnRst;
			std::vector<std::string>	allKeys;

			bool StartObject()
			{ //{{{
				return true;
			} //}}}
			bool Null()
			{ //{{{
				return true;
			} //}}}
			bool Key(const char* str, SizeType length, bool copy)
			{ //{{{
				curtKey = std::string(str);
				allKeys.push_back(curtKey);
				return true;
			} //}}}
			bool Int(int i)
			{ //{{{
				decJsnRst.append(curtKey.c_str(), i);
				return true;
			} //}}}
			bool Int64(uint64_t i)
			{ //{{{
				return true;
			} //}}}
			bool Bool(bool b)
			{ //{{{
				decJsnRst.append(curtKey.c_str(), b);
				return true;
			} //}}}
			bool Uint(unsigned u)
			{ //{{{
				decJsnRst.append(curtKey.c_str(), u);
				return true;
			} //}}}
			bool Int64(int64_t i)
			{ //{{{
				//switch (currentKey)
				//{
				//case Key_ndID:
				//	result.nodeID = i;
				//	break;
				//case Key_action:
				//case Key_transMat4:
				//default:
				//	assert(false);
				//}
				return true;
			} //}}}
			bool Uint64(uint64_t u)
			{ //{{{
				//switch (currentKey)
				//{
				//case Key_ndID:
				//	result.nodeID = u;
				//	break;
				//case Key_action:
				//case Key_transMat4:
				//default:
				//	assert(false);
				//}
				return true;
			} //}}}
			bool Double(double d)
			{ //{{{
				//switch (currentKey)
				//{
				//case Key_transMat4:
				//{
				//	/*result.transMat[mat4_idx++] = d;
				//	if (mat4_idx == 16)
				//		mat4_idx = 0;*/
				//	switch (mat4_idx)
				//	{
				//	case 0:
				//		result.transMat.m00 = d;
				//		break;
				//	case 1:
				//		result.transMat.m10 = d;
				//		break;
				//	case 2:
				//		result.transMat.m20 = d;
				//		break;
				//	case 3:
				//		result.transMat.m30 = d;
				//		break;
				//	case 4:
				//		result.transMat.m01 = d;
				//		break;
				//	case 5:
				//		result.transMat.m10 = d;
				//		break;
				//	case 6:
				//		result.transMat.m21 = d;
				//		break;
				//	case 7:
				//		result.transMat.m31 = d;
				//		break;
				//	case 8:
				//		result.transMat.m02 = d;
				//		break;
				//	case 9:
				//		result.transMat.m21 = d;
				//		break;
				//	case 10:
				//		result.transMat.m22 = d;
				//		break;
				//	case 11:
				//		result.transMat.m32 = d;
				//		break;
				//	case 12:
				//		result.transMat.m03 = d;
				//		break;
				//	case 13:
				//		result.transMat.m13 = d;
				//		break;
				//	case 14:
				//		result.transMat.m23 = d;
				//		break;
				//	case 15:
				//		result.transMat.m33 = d;
				//		mat4_idx = 0;
				//		break;
				//	}
				//	break;
				//}
				//default:
				//	assert(false);
				//}
				decJsnRst.append(curtKey.c_str(), d);
				return true;
			} //}}}
			bool String(const char* str, SizeType length, bool copy)
			{ //{{{
				decJsnRst.append(curtKey.c_str(), std::string(str));
				return true;
			} //}}}
			bool StartArray()
			{ //{{{
				decJsnRst.startArray();
				return true;
			} //}}}
			bool EndArray(SizeType elementCount)
			{ //{{{
				decJsnRst.endArray(curtKey);
				return true;
			} //}}}
			bool EndObject(SizeType memberCount)
			{ //{{{
				decJsnRst.setPrimary(allKeys);
				return true;
			} //}}}
		}; //}}}
		//}}}
	};
}
