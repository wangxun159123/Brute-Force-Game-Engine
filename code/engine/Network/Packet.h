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
#include <Network/DataPayload.h>
#include <Network/Defs.h>
#include <Network/Event.h>
#include <Network/Segment.h>

namespace BFG {
namespace Network {

template <typename ProtocolT>
class IPacket
{
	typedef typename ProtocolT::HeaderFactoryT HeaderFactoryT;
	typedef typename ProtocolT::HeaderT        HeaderT;
	
public:
	//! Constructing before write()
	IPacket(char* buffer, const HeaderFactoryT& factory) :
	mBuffer(buffer),
	mOffset(headerSize()),
	mHeaderFactory(factory)
	{}
	
	bool add(Segment& segment, const char* data)
	{
		if (!maySwallowSegmentAndData(segment))
			return false;

		memcpy(&mBuffer[mOffset], &segment, sizeof(Segment));
		mOffset += sizeof(Segment);
		
		memcpy(&mBuffer[mOffset], data, segment.dataSize);
		mOffset += segment.dataSize;
		return true;
	}
	
	bool add(const DataPayload& payload)
	{
		Segment s;
		s.appEventId = payload.mAppEventId;
		s.destinationId = payload.mAppDestination;
		s.senderId = payload.mAppSender;
		s.dataSize = payload.mAppDataLen;
		return add(s, payload.mAppData.data());
	}

	//! Returns a raw byte pointer to the packet's content.
	//! The packet header get's added at this point.
	char* full() const
	{
		mHeaderFactory.create(mBuffer, mOffset);
		return mBuffer;
	}
	
	u16 size() const
	{
		return mOffset;
	}

	//! Makes the packet reusable. A new buffer must be provided, though.
	void clear(char* buffer)
	{
		mBuffer = buffer;
		mOffset = headerSize();
	}
	
	//! Checks if the packet contains at least one segment
	bool containsData() const
	{
		const std::size_t minimumAmount = headerSize() + sizeof(Segment);

		return this->size() >= minimumAmount;
	}
	
private:
	static u16 headerSize()
	{
		return ProtocolT::headerSize();
	}
	
	void makeHeader()
	{
		HeaderT header = mHeaderFactory.create(mBuffer, mOffset);
		
		// Integrate header into packet
		typename HeaderT::SerializationT output;
		header.serialize(output);
		memcpy(mBuffer, output.data(), headerSize());
	}
	
	bool maySwallowSegmentAndData(const Segment& segment) const
	{
		std::size_t requiredSize = segment.dataSize + sizeof(Segment);
		std::size_t sizeLeft = ProtocolT::MAX_PACKET_SIZE_BYTES - mOffset;

		bool hasEnoughSpace = requiredSize <= sizeLeft;
		return hasEnoughSpace;
	}

	char* mBuffer;
	u16   mOffset;
	
	const HeaderFactoryT& mHeaderFactory;
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
