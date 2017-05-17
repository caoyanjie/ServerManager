#pragma once
#include "RakNet/RakPeerInterface.h"
#include "RakNet/RakNetTypes.h"
#include "RakNet/MessageIdentifiers.h"

#include <set>
#include <vector>
#include <map>
#include <utility>
#include <string>
#include <stdio.h>

namespace RakNetServer
{
	enum MessageType
	{ //{{{
		REQUEST_CREATEROOM = ID_USER_PACKET_ENUM + 1,
		REQUEST_JOIN_ROOM,
		REQUEST_GET_ROOMS,
		REQUEST_GET_USER_INFO,
		STATE_CREATED_ROOM_SUCCESS,
		STATE_CREATED_ROOM_FAIL,
		STATE_JOINED_ROOM_SUCCESS,
		STATE_JOINED_ROOM_FAIL,
		STATE_ROOMS,
		STATE_USER_INFO,
		MSG_CLIENT_DATA,
		MSG_CLIENT_VIEWPORT,
		SWITCH_CLIENT_VIEWPORT,
		RECEIVED_IMAGE_COUNT,
		CHANGE_MONITOR_QUILITY,
		CHANGE_USERNAME
	}; //}}}

	//std::map<std::string, std::string> JsonKeyhehehe;

	// 行为类型
	enum ActionType
	{ //{{{
		null = 0,
		NodeOccupied,

		SendNewFile = 18,
		SendFolder,

		ClientViewMonitor,
		AddClient,
		RemoveClient,
		RenameClient,
		clientMonitorState,

		MaxState

		//createNode,
		//deleteNode,
		//nodeWorldtransform,
		//nodeTransform,
		//cloneNode,
		//nodeEnable,
		//nodeVisible,
		//setNodeName,
		//setNodeID,
		//setNodeParent,
		//addNodeChild,
		//removeNodeChild,
		//itrTrackEvent,

		//setSurMtrColor,
		//setSurMtrTexrNm,
	}; //}}}

	// 矩阵
	struct mat4
	{ //{{{
		union {
			struct {
				float m00, m10, m20, m30;
				float m01, m11, m21, m31;
				float m02, m12, m22, m32;
				float m03, m13, m23, m33;
			};
			float mat[16];
		};
	}; //}}}

	//struct DecodeJsonResult
	//{ //{{{
	//	int			action;
	//	int			nodeID;
	//	bool		nodeOccupied;
	//	int			nodeEnable;
	//	bool		nodeDelete;
	//	mat4		transMat;
	//	std::string trackerNm;
	//	int			trackPlayerState;

	//	std::string author;
	//	int			adsorb;
	//}; //}}}

	struct DecJsnRst
	{ //{{{
		struct AnyType
		{ //{{{
			int				int_value;
			unsigned		unsigned_value;
			bool			bool_value;
			float			float_value;
			std::string 	str_value;

			enum VariableType
			{ //{{{
				Int,
				UnsignedInt,
				Bool,
				Float,
				String,
				Array
			}variableType; //}}}

			bool operator==(const AnyType & u_value) const
			{ //{{{
				if (u_value.variableType == this->variableType)
				{
					switch (variableType)
					{
					case Int:
						return u_value.int_value == this->int_value ? true : false;
					case UnsignedInt:
						return u_value.unsigned_value == this->unsigned_value ? true : false;
					case Bool:
						return u_value.bool_value == this->bool_value ? true : false;
					case Float:
						return u_value.float_value == this->float_value ? true : false;
					case String:
						return u_value.str_value == this->str_value ? true : false;
					case Array:
						return false;
					default:
						assert(false && "Unknown Variable Type");
					}
				}
				return false;
			} //}}}
		}; //}}}

		bool hasKey(const char* key) const
		{ //{{{
			if (jsonValue.find(key) == jsonValue.end())
				return false;
			else
				return true;
		} //}}}

		const AnyType & operator[](const char* key) const
		{ //{{{
			auto value = jsonValue.find(key);
			if (value != jsonValue.end())
				return value->second;
			
			printError(key);
			assert(false && "Json 中没有解析到该 Key");
		} //}}}

		const std::vector<AnyType> & getArray(const char* key) const
		{ //{{{
			auto array = jsonArray.find(key);
			if (array != jsonArray.end())
				return array->second;

			printError(key);
			assert(false && "Json 中没有解析到该 Key 的数组");
		} //}}}

		const std::map<std::string, AnyType> & getPrimary() const
		{ //{{{
			return primary;
		} //}}}

		void printError(const char* errorKey) const
		{ //{{{
			printf("\n\nJson 中没有解析到 Key: %s\n", errorKey);
			printf("Key 列表:\n");
			for (auto i = jsonValue.begin(); i != jsonValue.end(); ++i)
				printf("%s\t", i->first.c_str());
			for (auto i = jsonArray.begin(); i != jsonArray.end(); ++i)
				printf("%s\t", i->first.c_str());
			printf("\n");
		} //}}}

		void append(std::string key, int value)
		{ //{{{
			AnyType u_value;
			u_value.variableType = AnyType::Int;
			u_value.int_value = value;
			if (!isArray)
				jsonValue[key] = u_value;
			else
				array.push_back(u_value);
		} //}}}

		void append(std::string key, unsigned value)
		{ //{{{
			AnyType u_value;
			u_value.variableType = AnyType::UnsignedInt;
			u_value.unsigned_value = value;
			if (!isArray)
				jsonValue[key] = u_value;
			else
				array.push_back(u_value);
		} //}}}

		void append(std::string key, bool value)
		{ //{{{
			AnyType u_value;
			u_value.variableType = AnyType::Bool;
			u_value.bool_value = value;
			if (!isArray)
				jsonValue[key] = u_value;
			else
				array.push_back(u_value);
		} //}}}

		void append(std::string key, float value)
		{ //{{{
			AnyType u_value;
			u_value.variableType = AnyType::Float;
			u_value.float_value = value;
			if (!isArray)
				jsonValue[key] = u_value;
			else
				array.push_back(u_value);
		} //}}}
		
		void append(std::string key, double value)
		{ //{{{
			AnyType u_value;
			u_value.variableType = AnyType::Float;
			u_value.float_value = value;
			if (!isArray)
				jsonValue[key] = u_value;
			else
				array.push_back(u_value);
		} //}}}
		
		void append(std::string key, std::string value)
		{ //{{{
			AnyType u_value;
			//u_value.str_value = value.c_str();
			u_value.variableType = AnyType::String;
			u_value.str_value = value;
			if (!isArray)
				jsonValue[key] = u_value;
			else
				array.push_back(u_value);
		} //}}}

		void startArray()
		{ //{{{
			isArray = true;
			array.clear();
		} //}}}

		void endArray(std::string key)
		{ //{{{
			jsonArray[key] = array;
			isArray = false;
		} //}}}

		void setPrimary(std::vector<std::string> allKeys)
		{ //{{{
			auto array = jsonArray["primary"];
			for (auto keyIndex = array.begin(); keyIndex != array.end(); ++keyIndex)
			{
				switch (keyIndex->variableType)
				{
				case AnyType::Int:
					if (keyIndex->int_value >= 0)
					{
						const std::string key = allKeys.at(keyIndex->int_value);
						auto item = jsonValue.find(key);
						if (item != jsonValue.end())
						{
							primary[key] = item->second;
						}
						else
						{
							auto arrayItem = jsonArray.find(key);
							if (arrayItem != jsonArray.end())
							{
								AnyType temp;
								temp.variableType = AnyType::Array;
								primary[key] = temp;
							}
						}
					}
					break;
				case AnyType::UnsignedInt:
					if (keyIndex->unsigned_value >= 0)
					{
						const std::string key = allKeys.at(keyIndex->unsigned_value);
						auto item = jsonValue.find(key);
						if (item != jsonValue.end())
						{
							primary[key] = item->second;
						}
						else
						{
							auto arrayItem = jsonArray.find(key);
							if (arrayItem != jsonArray.end())
							{
								AnyType temp;
								temp.variableType = AnyType::Array;
								primary[key] = temp;
							}
						}
					}
					break;
				default:
					assert(false && "Unknown type of AnyType");
				}
			}
		} //}}}

		bool isPrimaryEqual(DecJsnRst &decJsnRst) const
		{ //{{{
			if (decJsnRst.getPrimary().size() != primary.size())
				return false;

			auto primary = decJsnRst.getPrimary();
			for (auto iter = primary.begin(); iter != primary.end(); ++iter)
			{
				if (!(iter->second == this->primary.at(iter->first)))
					return false;
			}

			return true;
		} //}}}
		
		private:
		std::map<std::string, AnyType>					jsonValue;
		std::map<std::string, std::vector<AnyType> >	jsonArray;
		std::map<std::string, AnyType>					primary;

		std::vector<DecJsnRst::AnyType>					array;
		bool isArray = false;
	}; //}}}

	// 客户端信息
	struct ClientInfo
	{ //{{{
		struct ExitMessage
		{
			std::string		jsnStr;
			DecJsnRst		jsnRst;		// 用于快速比较的结构
		};
		void appendSceneState(std::string jsonStr, DecJsnRst decJsnRst)
		{
			ExitMessage sMsg;
			sMsg.jsnStr = jsonStr;
			sMsg.jsnRst = decJsnRst;

			// std::lock_guard<std::mutex> lock(sceneStateLock);
			exitMessages.push_back(sMsg);
		}

		std::string									ipWithPort;					// 局域网服务器变为广域网服务器后这个变量几乎没用了，因为广域网ip连不上局域网ip
		RakNet::RakNetGUID							guid;
		std::string									username;
		int											index;
		std::set<int>								pickupList;
		std::vector<ExitMessage>					exitMessages;				// 房间中场景状态（刚进入房间中同步场景用）
		std::vector<std::pair<std::string, int> >	trackerNameAndStateList;
	}; //}}}

	// 房间
	struct Room
	{ //{{{
		struct SceneState
		{ //{{{
			std::string		jsnStr;
			DecJsnRst		jsnRst;		// 用于快速比较的结构
		}; //}}}

		void appendSceneState(std::string jsonStr, DecJsnRst decJsnRst)
		{ //{{{
			SceneState sState;
			sState.jsnStr = jsonStr;
			sState.jsnRst = decJsnRst;

			// std::lock_guard<std::mutex> lock(sceneStateLock);
			sceneState.push_back(sState);
		} //}}}

		RakNet::RakNetGUID				id;						// 房间ID（创建房间人的GUID
		std::string						name;
		std::string						passwd;
		ClientInfo						roomHost;				// 房间主
		bool							hasRole;
		std::string						voiceServer;			// 此房间所用的语聊服务器
		std::vector<ClientInfo>			clientList;				// 用户列表
		std::vector<SceneState>			sceneState;				// 房间中场景状态（刚进入房间中同步场景用）
		// std::mutex					sceneStateLock;
	}; //}}}

	// test
	struct ClientIndex
	{ //{{{
		int roomIndex;
		int clientIndex;
	}; //}}}

	//数据包信息
	struct PacketInfo
	{ //{{{
		RakNet::RakNetGUID	clientID;
		std::string			host;
		std::string			hostWithPort;
		std::string			jsonStr;
	}; //}}}

	// track 动画状态
	enum TrackplayerState :int
	{ //{{{
		TP_create,
		TP_remove,
		TP_play,
		TP_pause,
		TP_end,
		TP_reset,
		TP_delayPlay,

		TP_Max
	}; //}}}

	struct PacketNeedSend
	{ //{{{
		std::vector<RakNet::RakNetGUID> clients;
		std::string						jsonStr;
	}; //}}}

	enum BroadcastType
	{
		OnlyMe,
		ExceptMe,
		Everyone,
		RoomHost,
	};
}
