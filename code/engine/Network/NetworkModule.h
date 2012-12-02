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

#ifndef __BFG_NETWORKMODULE_H__
#define __BFG_NETWORKMODULE_H__

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/crc.hpp>
#include <boost/shared_ptr.hpp>

#include <Core/ClockUtils.h>
#include <Core/Types.h>
#include <EventSystem/Emitter.h>
#include <Network/Defs.h>
#include <Network/Event_fwd.h>


class EventLoop;


namespace BFG {
namespace Network{

using namespace boost::asio::ip;
using namespace boost::system;

class NetworkModule : public Emitter
{
public:
	NetworkModule(EventLoop* loop, boost::asio::io_service& service, PeerIdT peerId, boost::shared_ptr<Clock::StopWatch> localTime);
	~NetworkModule();

	boost::shared_ptr<tcp::socket> socket(){return mSocket;}

	void startReading();

	void setTimestampOffset(const s32 offset);
private:
	void setFlushTimer(const long& waitTime_ms);

	void write(const char* data, size_t size);
	void read();

	// async Handler
	void timerHandler(const error_code &ec);
	void readHeaderHandler(const error_code &ec, std::size_t bytesTransferred);
	void readDataHandler(const error_code &ec, std::size_t bytesTransferred, u32 packetChecksum);
	void writeHandler(const error_code &ec, std::size_t bytesTransferred);

	void dataPacketEventHandler(DataPacketEvent* ne);
	void onSend(DataPayload& payload);
	void onReceive(const char* data, size_t size);
	void flush();

	u16 calculateHeaderChecksum(const NetworkEventHeader& neh);

	template <typename PacketT>
	u32 calculatePacketChecksum(const PacketT& packet, size_t packetSize)
	{
		boost::crc_32_type result;
		result.process_bytes(packet.data(), packetSize);
		return result.checksum();
	}

	void startTimeSync();


	void printErrorCode(const error_code &ec, const std::string& method);

	boost::array<char, PACKET_MTU> mBackPacket;
	boost::array<char, PACKET_MTU> mFrontPacket;
	boost::array<char, PACKET_MTU> mWriteBuffer;
	boost::array<char, PACKET_MTU> mReadBuffer;

	NetworkEventHeader::SerializationT mWriteHeaderBuffer;
	NetworkEventHeader::SerializationT mReadHeaderBuffer;

	boost::shared_ptr<tcp::socket>                 mSocket;
	boost::shared_ptr<boost::asio::deadline_timer> mTimer;
	boost::mutex                                   mPacketMutex;

	boost::shared_ptr<Clock::StopWatch> mLocalTime;
	s32 mTimestampOffset;

	PeerIdT mPeerId;

	size_t mOutPacketPosition;

};

} // namespace Network
} // namespace BFG

#endif
