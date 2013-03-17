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

#include <boost/typeof/typeof.hpp>
#include <Network/Enums.hh>
#include <Network/Event.h>
#include <Network/Packet.h>

namespace BFG {
namespace Network{

using namespace boost::asio::ip;
using namespace boost::system;

NetworkModule::NetworkModule(EventLoop* loop_,
                             boost::asio::io_service& service,
                             PeerIdT peerId,
                             boost::shared_ptr<Clock::StopWatch> localTime) :
BFG::Emitter(loop_),
mPeerId(peerId),
mLocalTime(localTime),
mRoundTripTimer(Clock::milliSecond),
mOutPacketPosition(0),
mPool(PACKET_MTU*2)
{
	// Check case of accidental integer overflow for when mOutPacketPosition
	// might become smaller than one of the packet buffers.
	assert(PACKET_MTU <= std::numeric_limits<BOOST_TYPEOF(mOutPacketPosition)>::max());
	
	mSocket.reset(new SocketT(service));
	mTimer.reset(new boost::asio::deadline_timer(service));
}

NetworkModule::~NetworkModule()
{
	mTimer.reset();

	dbglog << "NetworkModule::~NetworkModule (" << this << ")";
	loop()->disconnect(ID::NE_SEND, this);

	mBackPacket.empty();
	mFrontPacket.empty();
	mWriteBuffer.empty();
	mReadBuffer.empty();
	mReadHeaderBuffer.empty();
	mWriteHeaderBuffer.empty();

	mOutPacketPosition = 0;

	mSocket.reset();
}

void NetworkModule::startReading()
{
	dbglog << "NetworkModule::startReading";
	setFlushTimer(FLUSH_WAIT_TIME);

	loop()->connect(ID::NE_SEND, this, &NetworkModule::dataPacketEventHandler);
	if (mPeerId)
		loop()->connect(ID::NE_SEND, this, &NetworkModule::dataPacketEventHandler, mPeerId);

	setTcpDelay(false);
	
	read();
}

void NetworkModule::sendTimesyncRequest()
{
	dbglog << "Sending timesync request";
	Network::DataPayload payload(ID::NE_TIMESYNC_REQUEST, 0, 0, 0, CharArray512T());
	queueTimeCriticalPacket(payload);
	mRoundTripTimer.start();
}

void NetworkModule::setFlushTimer(const long& waitTime_ms)
{
	if (waitTime_ms == 0)
		return;

	mTimer->expires_from_now(boost::posix_time::milliseconds(waitTime_ms));
	mTimer->async_wait(boost::bind(&NetworkModule::timerHandler, shared_from_this(), _1));
}

void NetworkModule::setTcpDelay(bool on)
{
	boost::asio::ip::tcp::no_delay oldOption;
	mSocket->get_option(oldOption);
	boost::asio::ip::tcp::no_delay newOption(!on);
	mSocket->set_option(newOption);
	
	dbglog << "Set TCP_NODELAY from " << oldOption.value()
	       << " to " << newOption.value();
}

void NetworkModule::write(const char* headerData, size_t headerSize, const char* packetData, size_t packetSize)
{
	dbglog << "NetworkModule::write: " << headerSize << " Bytes";

	size_t totalSize = headerSize+packetSize;
	assert(totalSize < mPool.get_requested_size());

	char* buffer = static_cast<char*>(mPool.malloc());
	memcpy(buffer, headerData, headerSize);
	memcpy(buffer+headerSize, packetData, packetSize);

	boost::asio::async_write
	(
		*mSocket,
		boost::asio::buffer(buffer, totalSize),
		boost::bind(&NetworkModule::writeHandler, shared_from_this(), _1, _2, buffer)
	);
}

void NetworkModule::read()
{
	dbglog << "NetworkModule::read (" << mPeerId << ")";
	boost::asio::async_read
	(
		*mSocket, 
		boost::asio::buffer(mReadHeaderBuffer),
		boost::asio::transfer_exactly(NetworkEventHeader::SerializationT::size()),
		bind(&NetworkModule::readHeaderHandler, shared_from_this(), _1, _2)
	);
	dbglog << "NetworkModule::read Done";
}

// async Handler
void NetworkModule::timerHandler(const error_code &ec)
{
	if (!ec)
	{
		flush();
		setFlushTimer(FLUSH_WAIT_TIME);
	}
	else if (ec.value() == boost::asio::error::operation_aborted)
	{
		dbglog << "NetworkModule: mTimer was cancelled!";
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
			bind(&NetworkModule::readDataHandler, shared_from_this(), _1, _2, neh.mPacketChecksum)
		);
	}
	else if (ec.value() == boost::asio::error::connection_reset)
	{
		dbglog << "NetworkModule: connection was closed!";
		emit<ControlEvent>(ID::NE_DISCONNECT, mPeerId);
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
	else if (ec.value() == boost::asio::error::connection_reset)
	{
		dbglog << "NetworkModule: connection was closed!";
		emit<ControlEvent>(ID::NE_DISCONNECT, mPeerId);
	}
	else
	{
		emit<ControlEvent>(ID::NE_DISCONNECT, mPeerId);
		printErrorCode(ec, "readDataHandler");
	}
}

void NetworkModule::writeHandler(const error_code &ec, std::size_t bytesTransferred, char* buffer)
{
	dbglog << "NetworkModule::writeHandler: " << bytesTransferred << " Bytes written";
	if (ec)
	{
		printErrorCode(ec, "writeHandler");
	}
	mPool.free(static_cast<void*>(buffer));
} 

void NetworkModule::queueTimeCriticalPacket(DataPayload& payload)
{
	dbglog << "NetworkModule::queueTimeCriticalPacket -> onSend";
	onSend(payload);
	dbglog << "NetworkModule::queueTimeCriticalPacket -> Flush";
	flush();
	dbglog << "NetworkModule::queueTimeCriticalPacket -> Done";
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
	       << " (" << sizeof(Segment) << " + " << payload.mAppDataLen << ")";

	dbglog << "NetworkModule:Current Time: " << mLocalTime->stop();

	Segment s;
	s.appEventId = payload.mAppEventId;
	s.destinationId = payload.mAppDestination;
	s.senderId = payload.mAppSender;
	s.dataSize = payload.mAppDataLen;

	size_t requiredSize = s.dataSize + sizeof(Segment);
	size_t sizeLeft = mBackPacket.size() - mOutPacketPosition;

	if (requiredSize <= sizeLeft)
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

	PayloadFactory payloadFactory(mTimestampOffset, mLocalTime, mRtt);
	OPacket<Tcp> packet(data, size);
	
	while (packet.hasNextPayload())
	{
		DataPayload payload = packet.nextPayload(payloadFactory);

		if (payload.mAppEventId == ID::NE_TIMESYNC_REQUEST)
		{
			onTimeSyncRequest();
			return;
		}
		else if (payload.mAppEventId == ID::NE_TIMESYNC_RESPONSE)
		{
			TimestampT serverTimestamp;
			memcpy(&serverTimestamp, payload.mAppData.data(), payload.mAppDataLen);
			onTimeSyncResponse(serverTimestamp);
		}
		
		try 
		{
			dbglog << "NetworkModule::onReceive: Emitting NE_RECEIVED to: " << payload.mAppDestination << " from: " << mPeerId;
			emit<DataPacketEvent>(ID::NE_RECEIVED, payload, payload.mAppDestination, mPeerId);
		}
		catch (std::exception& ex)
		{
			warnlog << ex.what();
		}
	}
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
	
	dbglog << "NetworkModule::flush -> Flushing: " << mOutPacketPosition;

	write
	(
		mWriteHeaderBuffer.data(),
		NetworkEventHeader::SerializationT::size(),
		mFrontPacket.data(),
		mOutPacketPosition
	);

	// Cleanup
	memset(mFrontPacket.c_array(), 0, mFrontPacket.size());
	mOutPacketPosition = 0;
}

void NetworkModule::onTimeSyncRequest()
{
	dbglog << "Got time sync request from PeerId: " << mPeerId;

	TimestampT timestamp = mLocalTime->stop();

	CharArray512T ca512;
	memcpy(ca512.data(), &timestamp, sizeof(TimestampT));
	Network::DataPayload payload(ID::NE_TIMESYNC_RESPONSE, 0, 0, sizeof(TimestampT), ca512);

	queueTimeCriticalPacket(payload);
}

void NetworkModule::onTimeSyncResponse(TimestampT serverTimestamp)
{
	dbglog << "Got time sync response from PeerId: " << mPeerId;

	s32 offset;
	s32 rtt;
	calculateServerTimestampOffset(serverTimestamp, offset, rtt);

	setTimestampOffset(offset, rtt);
}

void NetworkModule::setTimestampOffset(const s32 offset, const s32 rtt)
{
	dbglog << "NetworkModule:setTimestampOffset: "
		<< offset << "(" << offset - mTimestampOffset << ")" << ", "
		<< rtt << "(" << rtt - mRtt.last() << ")";

	mTimestampOffset = offset;
	mRtt.add(rtt);

	dbglog << "New avg rtt: " << mRtt.mean();
}

u16 NetworkModule::calculateHeaderChecksum(const NetworkEventHeader& neh)
{
	boost::crc_16_type result;
	result.process_bytes(&(neh.mPacketSize), sizeof(neh.mPacketSize));
	result.process_bytes(&(neh.mTimestamp), sizeof(neh.mTimestamp));
	result.process_bytes(&(neh.mPacketChecksum), sizeof(neh.mPacketChecksum));
	return result.checksum();
}


void NetworkModule::calculateServerTimestampOffset(u32 serverTimestamp, s32& offset, s32& rtt)
{
	// https://en.wikipedia.org/wiki/Cristian%27s_algorithm
	// offset = tS + dP/2 - tC
	u32 dP = mRoundTripTimer.stop();
	u32 tC = mLocalTime->stop();
	s32 serverOffset = serverTimestamp + dP / 2 - tC;

	dbglog << "Calculated server Timestamp Offset: " << serverOffset 
		<< " with RTT of " << dP;
	dbglog << "LocalTime was: " << tC;
	
	offset = serverOffset;
	rtt = dP;
}

void NetworkModule::printErrorCode(const error_code &ec, const std::string& method)
{
	warnlog << "This (" << this << ") " << "[" << method << "] Error Code: " << ec.value() << ", message: " << ec.message();
}


} // namespace Network
} // namespace BFG
