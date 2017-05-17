#pragma once
#include <vector>

namespace RakNet {
	class RakPeerInterface;
	struct RakNetGUID;
}

namespace RakNetServer{
	class NATPunchthroghFacilitator
	{
	public:
		static RakNetServer::NATPunchthroghFacilitator* GetInstance();
		void start();

	private:
		enum FeatureSupport 
		{
			SUPPORTED,
			UNSUPPORTED,
			QUERY
		};

		struct VoiceRoom
		{
			std::string						roomID;
			std::vector<RakNet::RakNetGUID> clientsIDList;
		};

		NATPunchthroghFacilitator();
		~NATPunchthroghFacilitator();

		void init();
		void removeClient(const RakNet::RakNetGUID clientID);

		static RakNetServer::NATPunchthroghFacilitator		*m_instance;
		RakNet::RakPeerInterface							*m_natPunchthroughFacilitator;
		std::vector<VoiceRoom>								m_voiceRoomList;
		bool												m_isWorking;
		const unsigned short								NAT_PUNCHGHROUGH_FACILITATOR_PORT;
	};
}
