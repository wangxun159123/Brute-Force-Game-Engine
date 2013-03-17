/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2013 Brute-Force Games GbR

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

#ifndef BFG_NETWORK_PACKET_H
#define BFG_NETWORK_PACKET_H

#include <cstring>
#include <boost/array.hpp>
#include <Core/ClockUtils.h>
#include <Network/Defs.h>
#include <Network/Event.h>
#include <Network/UnreliableHeader.h>
#include <Network/Rtt.h>

namespace BFG {
namespace Network {

class PayloadFactory
{
public:
	PayloadFactory(s32 timestampOffset,
	               boost::shared_ptr<Clock::StopWatch> localtime,
	               Rtt<s32, 10> rtt) :
	mTimestampOffset(timestampOffset),
	mLocalTime(localtime),
	mRtt(rtt)
	{}
	
	DataPayload create(const Segment& segment, const CharArray512T& data) const
	{
		u32 currentServerTimestamp = mTimestampOffset + mLocalTime->stop();
		
		DataPayload payload
		(
			segment.appEventId,
			segment.destinationId,
			segment.senderId,
			segment.dataSize,
			data,
			currentServerTimestamp,
			mRtt.mean() / 2
		);
		return payload;
	};
	
	s32 mTimestampOffset;
	boost::shared_ptr<Clock::StopWatch> mLocalTime;
	Rtt<s32, 10> mRtt;
};

class UdpHeaderFactory
{
public:
	typedef UnreliableHeader HeaderT;
	
	UnreliableHeader create() const
	{
		UnreliableHeader header;
		header.mSequenceNumber = 5;
		header.mTimestamp = 0.9f;
		return header;
	};
};

struct Udp
{
	typedef UnreliableHeader HeaderT;
	typedef UdpHeaderFactory HeaderFactoryT;
};

struct Todo{};

struct Tcp
{
	typedef NetworkEventHeader HeaderT;
	typedef Todo HeaderFactoryT;
};
	
template <typename ProtocolT>
class IPacket
{
	typedef typename ProtocolT::HeaderFactoryT HeaderFactoryT;
	
public:
	//! Constructing before write()
	IPacket(char* buffer, const HeaderFactoryT& factory) :
	mBuffer(buffer),
	mOffset(0)
	{
		makeHeader(factory);
	}
	
	void add(Segment& segment, const char* data)
	{
		memcpy(&mBuffer[mOffset], &segment, sizeof(Segment));
		mOffset += sizeof(Segment);
		memcpy(&mBuffer[mOffset], data, segment.dataSize);
		mOffset += segment.dataSize;
	}
	
	char* buffer() const
	{
		return mBuffer;
	}
	
	u16 size() const
	{
		return mOffset;
	}
	
	//void add(PayloadT&);
private:
	void makeHeader(const HeaderFactoryT& factory)
	{
		typedef typename ProtocolT::HeaderT HeaderT;
		typedef typename HeaderT::SerializationT SerializationT;
		const u16 headerSize = SerializationT::size();

		HeaderT header = factory.create();

		SerializationT output;
		header.serialize(output);
		
		memcpy(mBuffer, output.data(), headerSize);
		mOffset += headerSize;
	}
	
	char* mBuffer;
	u16 mOffset;
};

template <typename ProtocolT>
class OPacket
{
public:
	//! Constructing after read()
	OPacket(const char* buffer, std::size_t len) :
	mBuffer(buffer),
	mSize(len),
	mOffset(0)
	{
		parseHeader();
	}
	
	bool hasNextPayload() const
	{
		return mOffset + sizeof(Segment) <= mSize;
	}
	
	DataPayload nextPayload(PayloadFactory& payloadFactory)
	{
		Segment s;
		arrayToValue(s, mBuffer, mOffset);
		mOffset += sizeof(Segment);

		CharArray512T ca;
		memcpy(ca.data(), &mBuffer[mOffset], s.dataSize);
		mOffset += s.dataSize;

		DataPayload payload = payloadFactory.create(s, ca);
		return payload;
	}
	
	// TODO: Return Header
	void parseHeader()
	{
		typedef typename ProtocolT::HeaderT HeaderT;
		typedef typename HeaderT::SerializationT SerializationT;
		const u16 headerSize = SerializationT::size();

		//mOffset += headerSize;
	}
	
private:
	const char* mBuffer;
	const std::size_t mSize;
	
	std::size_t mOffset;
};

} // namespace Network
} // namespace BFG

#endif
