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

#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <Base/EntryPoint.h>
#include <Core/ClockUtils.h>
#include <Network/Defs.h>
#include <Network/Event_fwd.h>

class EventLoop;

namespace BFG {
namespace Network{

using namespace boost::asio::ip;
using namespace boost::system;

class NetworkModule;

class NETWORK_API Client
{
public:
	Client(EventLoop* loop);
	~Client();
private:
	void stop();
	void startConnecting(const std::string& ip, const std::string& port);

	void readHandshake();

	void resolveHandler(const error_code &ec, tcp::resolver::iterator it);
	void connectHandler(const error_code &ec);
	void readHandshakeHandler(const error_code &ec, size_t bytesTransferred);

	void controlEventHandler(ControlEvent* ne);
	void dataPacketEventHandler(DataPacketEvent* e);

	void onConnect(const EndpointT& endpoint);
	void onDisconnect(const PeerIdT& peerId);
	
	u16 calculateHandshakeChecksum(const Handshake& hs);
	s32 calculateServerTimestampOffset(u32 serverTimestamp);

	void printErrorCode(const error_code &ec, const std::string& method);

	void sendTimesyncRequest();
	void setTimeSyncTimer(const long& waitTime_ms);
	void timerHandler(const error_code &ec);

	boost::asio::io_service mService;
	boost::shared_ptr<tcp::resolver> mResolver;

	boost::thread mThread;
	
	EventLoop* mLoop;
	NetworkModule* mNetworkModule;
	PeerIdT mPeerId;
	
	// TODO: This works only for 7 weeks of server runtime!
	// TODO: This will wreak havoc after 7 weeks.
	// TODO: Reset this watch somehow.
	boost::shared_ptr<Clock::StopWatch> mLocalTime;
	Clock::StopWatch mRTT;

	Handshake::SerializationT mHandshakeBuffer;

	boost::shared_ptr<boost::asio::deadline_timer> mTimeSyncTimer;

};

} // namespace Network
} // namespace BFG

#endif
