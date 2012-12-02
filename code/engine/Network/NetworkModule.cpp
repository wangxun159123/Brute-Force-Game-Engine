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

#include <Network/NetworkModule.h>

#include <Network/Enums.hh>
#include <Network/Event.h>

namespace BFG {
namespace Network{

using namespace boost::asio::ip;
using namespace boost::system;

NetworkModule::NetworkModule(EventLoop* loop_, boost::asio::io_service& service, PeerIdT peerId, boost::shared_ptr<Clock::StopWatch> localTime) :
BFG::Emitter(loop_),
mPeerId(peerId),
mOutPacketPosition(0),
mLocalTime(localTime)
{
	mSocket.reset(new tcp::socket(service));
	mTimer.reset(new boost::asio::deadline_timer(service));

	setFlushTimer(FLUSH_WAIT_TIME);

	loop()->connect(ID::NE_SEND, this, &NetworkModule::dataPacketEventHandler);
	if (peerId)
		loop()->connect(ID::NE_SEND, this, &NetworkModule::dataPacketEventHandler, peerId);
}

NetworkModule::~NetworkModule()
{
	dbglog << "NetworkModule::~NetworkModule";
	loop()->disconnect(ID::NE_SEND, this);

	mTimer->cancel();

	mSocket.reset();
	mTimer.reset();
}

void NetworkModule::startReading()
{
	dbglog << "NetworkModule::startReading";
	read();
}

void NetworkModule::setFlushTimer(const long& waitTime_ms)
{
	if (waitTime_ms == 0)
		return;

	mTimer->expires_from_now(boost::posix_time::milliseconds(waitTime_ms));
	mTimer->async_wait(boost::bind(&NetworkModule::timerHandler, this, _1));
}

void NetworkModule::write(const char* data, size_t size)
{
	// TODO: we probably need to lock a mutex here and unlock it in the writeHandler
	dbglog << "NetworkModule::write: " << size << " Bytes";
	memcpy(mWriteBuffer.c_array(), data, size);
	boost::asio::async_write
	(
		*mSocket,
		boost::asio::buffer(mWriteBuffer, size),
		boost::bind(&NetworkModule::writeHandler, this, _1, _2)
	);
}

void NetworkModule::read()
{
	dbglog << "NetworkModule::read";
	boost::asio::async_read
	(
		*mSocket, 
		boost::asio::buffer(mReadHeaderBuffer),
		boost::asio::transfer_exactly(NetworkEventHeader::SerializationT::size()),
		bind(&NetworkModule::readHeaderHandler, this, _1, _2)
	);
}

// async Handler
void NetworkModule::timerHandler(const error_code &ec)
{
	if (!ec)
	{
		flush();
		setFlushTimer(FLUSH_WAIT_TIME);
	}
	else
	{
		printErrorCode(ec, "timerHandler");
	}
}

void NetworkModule::readHeaderHandler(const error_code &ec, std::size_t bytesTransferred)
{
	dbglog << "NetworkModule::readHeaderHandler (" << bytesTransferred << ")";
	if (!ec) 
	{
		NetworkEventHeader neh;
		neh.deserialize(mReadHeaderBuffer);
		
		if (neh.mPacketSize == 0)
		{
			warnlog << "readHeaderHandler: Got empty Header! Disconnecting Client.";

			// Peer sends crap? Bye bye!
			mSocket.reset();
			return;
		}

		u16 headerChecksum = neh.mHeaderChecksum;
		neh.mHeaderChecksum = 0;

		u16 ownHeaderChecksum = calculateHeaderChecksum(neh);

		if (ownHeaderChecksum != headerChecksum)
		{
			warnlog << std::hex << std::uppercase 
			        << "readHeaderHandler: Got bad Header (Own CRC: "
			        << ownHeaderChecksum
			        << " Rcvd CRC: "
			        << headerChecksum
			        << "). Disconnecting Peer.";

			// Peer sends crap? Bye bye!
			mSocket.reset();
			return;
		}

		dbglog << "PacketSize: " << neh.mPacketSize;
		boost::asio::async_read
		(
			*mSocket, 
			boost::asio::buffer(mReadBuffer),
			boost::asio::transfer_exactly(neh.mPacketSize),
			bind(&NetworkModule::readDataHandler, this, _1, _2, neh.mPacketChecksum)
		);
	}
	else
	{
		emit<ControlEvent>(ID::NE_DISCONNECT, mPeerId);
		printErrorCode(ec, "readHeaderHandler");
	}
}

void NetworkModule::readDataHandler(const error_code &ec, std::size_t bytesTransferred, u32 packetChecksum)
{
	dbglog << "NetworkModule::readDataHandler (" << bytesTransferred << ")";
	if (!ec) 
	{
		u32 ownPacketChecksum = calculatePacketChecksum(mReadBuffer, bytesTransferred);

		if (ownPacketChecksum != packetChecksum)
		{
			warnlog << std::hex << std::uppercase 
			        << "readDataHandler: Got bad Packet (Own CRC: "
			        << ownPacketChecksum
			        << " Rcvd CRC: "
			        << packetChecksum
			        << "). Disconnecting Peer.";

			// Peer sends crap? Bye bye!
			mSocket.reset();
			return;
		}

		onReceive(mReadBuffer.c_array(), bytesTransferred);
		read();
	}
	else
	{
		emit<ControlEvent>(ID::NE_DISCONNECT, mPeerId);
		printErrorCode(ec, "readDataHandler");
	}
}

void NetworkModule::writeHandler(const error_code &ec, std::size_t bytesTransferred)
{
	dbglog << "NetworkModule::writeHandler: " << bytesTransferred << " Bytes written";
	if (ec)
	{
		printErrorCode(ec, "writeHandler");
	}
} 

void NetworkModule::dataPacketEventHandler(DataPacketEvent* e)
{
	switch(e->getId())
	{
	case ID::NE_SEND:
		onSend(e->getData());
		break;
	default:
		warnlog << "NetworkModule: Can't handle event with ID: "
		        << e->getId();
		break;
	}
}

void NetworkModule::onSend(DataPayload& payload)
{
	dbglog << "onSend: " << payload.mAppDataLen + sizeof(Segment)
	       << "(" << payload.mAppDataLen << "|" << sizeof(Segment) << ")";

	dbglog << "NetworkModule:Current Time: " << mLocalTime->stop();

	Segment s;
	s.appEventId = payload.mAppEventId;
	s.destinationId = payload.mAppDestination;
	s.senderId = payload.mAppSender;
	s.dataSize = payload.mAppDataLen;

	size_t requiredSize = s.dataSize + sizeof(Segment);
	size_t sizeLeft = mBackPacket.size() - mOutPacketPosition;
	if (requiredSize < sizeLeft)
	{
		boost::mutex::scoped_lock scoped_lock(mPacketMutex);
		memcpy(&mBackPacket[mOutPacketPosition], &s, sizeof(Segment));
		mOutPacketPosition += sizeof(Segment);
		memcpy(&mBackPacket[mOutPacketPosition], payload.mAppData.c_array(), s.dataSize);
		mOutPacketPosition += s.dataSize;
	}
	else
	{
		flush();
		onSend(payload);
	}
}

void NetworkModule::onReceive(const char* data, size_t size)
{
	dbglog << "NetworkModule::onReceive";
	size_t packetPosition = 0;

	do 
	{
		Segment s;
		arrayToValue(s, data, packetPosition);
		packetPosition += sizeof(Segment);

		CharArray512T ca;
		memcpy(ca.data(), &data[packetPosition], s.dataSize);
		packetPosition += s.dataSize;

		u32 currentServerTimestamp = mTimestampOffset + mLocalTime->stop();

		DataPayload payload(s.appEventId, s.destinationId, s.senderId, s.dataSize, ca, currentServerTimestamp);

		try 
		{
			dbglog << "NetworkModule::onReceive: Emitting NE_RECEIVED to: " << s.destinationId << " from: " << mPeerId;
			emit<DataPacketEvent>(ID::NE_RECEIVED, payload, s.destinationId, mPeerId);
		}
		catch (std::exception& ex)
		{
			warnlog << ex.what();
		}
	}
	while ((packetPosition + sizeof(Segment)) < size);
}

void NetworkModule::flush()
{
	boost::mutex::scoped_lock scoped_lock(mPacketMutex);

	// Nothing to write?
	if (mOutPacketPosition == 0)
		return;

	std::swap(mFrontPacket, mBackPacket);

	// Create Header with Checksums
	u32 packetChecksum = calculatePacketChecksum(mFrontPacket, mOutPacketPosition);
	NetworkEventHeader neh = {0.0f, packetChecksum, 0, mOutPacketPosition};
	neh.mHeaderChecksum = calculateHeaderChecksum(neh);

	neh.serialize(mWriteHeaderBuffer);
	
	dbglog << "Flushing: " << mOutPacketPosition;
	write(mWriteHeaderBuffer.data(), NetworkEventHeader::SerializationT::size());
	write(mFrontPacket.data(), mOutPacketPosition);

	// Cleanup
	memset(mFrontPacket.c_array(), 0, mFrontPacket.size());
	mOutPacketPosition = 0;
}

u16 NetworkModule::calculateHeaderChecksum(const NetworkEventHeader& neh)
{
	boost::crc_16_type result;
	result.process_bytes(&(neh.mPacketSize), sizeof(neh.mPacketSize));
	result.process_bytes(&(neh.mTimestamp), sizeof(neh.mTimestamp));
	result.process_bytes(&(neh.mPacketChecksum), sizeof(neh.mPacketChecksum));
	return result.checksum();
}

void NetworkModule::printErrorCode(const error_code &ec, const std::string& method)
{
	warnlog << "[" << method << "] Error Code: " << ec.value() << ", message: " << ec.message();
}

void NetworkModule::setTimestampOffset(const s32 offset)
{
	mTimestampOffset = offset;
	dbglog << "NetworkModule:setTimestampOffset: " << offset;
}




} // namespace Network
} // namespace BFG
