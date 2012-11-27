/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2011 Brute-Force Games GbR

The BFG-Engine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

The BFG-Engine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the BFG-Engine. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef BFG_NETWORKDEFS_H
#define BFG_NETWORKDEFS_H

#include <cstring>
#include <boost/array.hpp>

#include <Core/Types.h>

#define BFG_SERVER 0
#define BFG_CLIENT 1

#ifdef _WIN32
	#ifndef NETWORK_EXPORTS
		#define NETWORK_API __declspec(dllimport)
	#else
		#define NETWORK_API __declspec(dllexport)
	#endif //NETWORK_EXPORTS
#else // UNIX
    #define NETWORK_API
#endif// UNIX

namespace BFG {
namespace Network{

typedef GameHandle PeerIdT;

struct Handshake
{
	typedef boost::array<char, 16> SerializationT;
	
	void serialize(SerializationT& output) const
	{
		char* p = output.data();
		memcpy(p, &mPeerId, sizeof(PeerIdT));
		p += sizeof(PeerIdT);
		memcpy(p, &mTimestamp, sizeof(u32));
		p += sizeof(u32);
		memcpy(p, &mProtocolVersion, sizeof(u16));
		p += sizeof(u16);
		memcpy(p, &mChecksum, sizeof(u16));
	}

	void deserialize(const SerializationT& input)
	{
		const char* p = input.data();
		memcpy(&mPeerId, p, sizeof(PeerIdT));
		p += sizeof(PeerIdT);
		memcpy(&mTimestamp, p, sizeof(u32));
		p += sizeof(u32);
		memcpy(&mProtocolVersion, p, sizeof(u16));
		p += sizeof(u16);
		memcpy(&mChecksum, p, sizeof(u16));
	}
	
	// TODO: replace PeerIdT with simpler type
	PeerIdT mPeerId;

	u32 mTimestamp;
	u16 mProtocolVersion;
	u16 mChecksum;
};

const u32 PACKET_MTU(2000); // max size a packet can expand to before it will be flushed (Q3: rate)
const u32 FLUSH_WAIT_TIME(20); // ms before automatic flush (Q3: 1000/cl_update_rate), "natural flush time" depends on bandwidth

struct NetworkEventHeader
{
	typedef boost::array<char, 12> SerializationT;

	void serialize(SerializationT& output) const
	{
		char* p = output.data();
		memcpy(p, &mTimestamp, sizeof(mTimestamp));
		p += sizeof(mTimestamp);
		memcpy(p, &mPacketChecksum, sizeof(mPacketChecksum));
		p += sizeof(mPacketChecksum);
		memcpy(p, &mHeaderChecksum, sizeof(mHeaderChecksum));
		p += sizeof(mHeaderChecksum);
		memcpy(p, &mPacketSize, sizeof(mPacketSize));
	}

	void deserialize(const SerializationT& input)
	{
		const char* p = input.data();
		memcpy(&mTimestamp, p, sizeof(mTimestamp));
		p += sizeof(mTimestamp);
		memcpy(&mPacketChecksum, p, sizeof(mPacketChecksum));
		p += sizeof(mPacketChecksum);
		memcpy(&mHeaderChecksum, p, sizeof(mHeaderChecksum));
		p += sizeof(mHeaderChecksum);
		memcpy(&mPacketSize, p, sizeof(mPacketSize));
	}

	f32 mTimestamp;
	u32 mPacketChecksum;
	u16 mHeaderChecksum;
	u16 mPacketSize;
};

// TODO: replace GameHandle with simpler type
struct Segment
{
	// set biggest datatype to the top to optimize padding
	GameHandle destinationId;
	GameHandle senderId;
	u32 appEventId;
	u32 dataSize;
};

}// namespace BFG
}// namespace Network

#endif
