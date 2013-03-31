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
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/pool/pool.hpp>

#include <Core/ClockUtils.h>
#include <Core/Types.h>
#include <EventSystem/Emitter.h>
#include <Network/Defs.h>
#include <Network/Event_fwd.h>
#include <Network/Packet.h>
#include <Network/Rtt.h>
#include <Network/Tcp.h>

class EventLoop;

namespace BFG {
namespace Network {

using namespace boost::asio::ip;
using namespace boost::system;

//! A NetworkModule is used to communicate over the network. The data can be send using the NE_SEND event. 
//! Received data is send via the NE_RECEIVED event.
class NetworkModule : public Emitter, public boost::enable_shared_from_this<NetworkModule>
{
public:
	typedef tcp::socket SocketT;
	
	//! \brief Constructor
	//! \param[in] loop Eventloop the NetworkModule is connected to 
	//! \param[in] service Asio service for the network connection
	//! \param[in] peerId ID of the module for identification over the network
	//! \param[in] localTime The local time of this module
	NetworkModule(EventLoop* loop, boost::asio::io_service& service, PeerIdT peerId, boost::shared_ptr<Clock::StopWatch> localTime);
	~NetworkModule();

	//! \brief Returns the socket of the connection
	//! \return tcp::socket of the connection
	boost::shared_ptr<SocketT> socket(){return mSocket;}

	//! \brief The connection is ready to receive data
	void startReading();

	//! \brief Start synchronizing the local clock with the server
	void sendTimesyncRequest();
	
private:
	typedef u32 TimestampT;

	//! \brief Reset the time for the next automatic flush
	//! \param[in] waitTime_ms Time in milliseconds to the next flush
	void setFlushTimer(const long& waitTime_ms);

	//! \brief Enable or disable Nagle's Algorithm
	//! Enabling this could cause a delay in latency
	//! \param[in] on Enable delay or not
	void setTcpDelay(bool on);
	
	//! \brief Perform an asynchronous write of data to the connected network module
	//! \param[in] packet data to write over the net
	//! \param[in] size Size of the data set
	void write(boost::asio::const_buffer packet, std::size_t size);

	//! \brief Start asynchronous reading from the connected network module
	void read();

	//! \brief Handler for the flush timer
	//! \param[in] ec Error code of boost asio
	void flushTimerHandler(const error_code &ec);

	//! \brief Handler for the reading of the data header
	//! \param[in] ec Error code of boost asio
	//! \param[in] bytesTransferred size of the data received
	void readHeaderHandler(const error_code &ec, std::size_t bytesTransferred);

	//! \brief Handler for the reading of the data
	//! \param[in] ec Error code of boost asio
	//! \param[in] bytesTransferred size of the data received
	//! \param[in] pacetChecksum Checksum of the data packet
	void readDataHandler(const error_code &ec, std::size_t bytesTransferred, u32 packetChecksum);

	//! \brief Handler for the writing of data
	//! \param[in] ec Error code of boost::asio
	//! \param[in] bytesTransferred size of the data written
	void writeHandler(const error_code &ec, std::size_t bytesTransferred, boost::asio::const_buffer buffer);

	//! \brief Send time criticle data to the connected network module
	//! Use this function to send packets as fast as possible.
	//! (e.g. for time synchronization). The payload will be sent almost
	//! immediately together with any other packets which were yet waiting
	//! for delivery. Therefore, delays caused by the event system or flush
	//! wait times are avoided.
	//! \param[in] payload Data to send
	void queueTimeCriticalPacket(DataPayload& payload);
	
	//! \brief Handler for DataPacketEvents
	//! \param[in] e The DataPacketEvent to distribute
	void dataPacketEventHandler(DataPacketEvent* e);

	//! \brief Sending data to the connected network module
	//! With this method the data is queued and flushes automatically every few milliseconds 
	//! or if the queue is full
	//! \param[in] payload Data to send
	void onSend(DataPayload& payload);

	//! \brief Received data from the net is packed as a corresponding event 
	//! \param[in] data data array received from the network
	//! \param[in] size size of the data received
	void onReceive(OPacket<Tcp>& oPacket);

	//! \brief Flushes the data queue to send its data to the connected network module
	void flush();

	//! \brief Handles a request for time synchronization and sends the local timestamp back
	void onTimeSyncRequest();

	//! \brief Handles a response for time synchronization
	//! \param[in] serverTimestamp Timestamp of the server
	void onTimeSyncResponse(TimestampT serverTimestamp);
	
	//! \brief Sets the offset between the local time and server time
	//! \param[in] offset The time offset between client and server
	//! \param[in] rtt Round-Trip-Time of the data packet
	void setTimestampOffset(const s32 offset, const s32 rtt);
	
	//! \brief Calculates the offset between server and client time
	//! \param[in] serverTimestamp Timestamp of the server
	//! \param[out] offset Calculated offset
	//! \param[out] rtt Calculated round trip time
	void calculateServerTimestampOffset(u32 serverTimestamp, s32& offset, s32& rtt);
	
	//! \brief Logs an error_code
	//! \param[in] ec Error code to log
	//! \param[in] method Name of the method that received the error
	void printErrorCode(const error_code &ec, const std::string& method);

	boost::pool<> mPool;

	Tcp::HeaderFactoryT mHeaderFactory;
	IPacket<Tcp> mSendPacket;

	boost::array<char, PACKET_MTU> mWriteBuffer;
	boost::array<char, PACKET_MTU> mReadBuffer;

	NetworkEventHeader::SerializationT mWriteHeaderBuffer;
	NetworkEventHeader::SerializationT mReadHeaderBuffer;

	boost::shared_ptr<SocketT>                     mSocket;
	boost::shared_ptr<boost::asio::deadline_timer> mTimer;
	boost::mutex                                   mFlushMutex;

	s32 mTimestampOffset;
	Rtt<s32, 10> mRtt;
	Clock::StopWatch mRoundTripTimer;

	PeerIdT mPeerId;
	boost::shared_ptr<Clock::StopWatch> mLocalTime;
};

} // namespace Network
} // namespace BFG

#endif
