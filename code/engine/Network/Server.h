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

#ifndef __SERVER_H__
#define __SERVER_H__

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <Base/EntryPoint.h>
#include <Core/CharArray.h>
#include <Core/ClockUtils.h>
#include <Core/Types.h>
#include <Network/Defs.h>
#include <Network/Event_fwd.h>

class EventLoop;

namespace BFG {
namespace Network{

using namespace boost::asio::ip;
using namespace boost::system;

class NetworkModule;

class NETWORK_API Server
{
public:
	Server(EventLoop* loop);
	~Server();
private:
	typedef std::map<PeerIdT, NetworkModule* > ModulesMap;

	void stop();
	void startAccepting();
	void sendHandshake(PeerIdT peerId);

	void acceptHandler(const boost::system::error_code &ec, PeerIdT peerId);
	void writeHandshakeHandler(const error_code &ec, std::size_t bytesTransferred, PeerIdT peerId);

	void controlEventHandler(ControlEvent* ne);

	void onListen(const u16 port);
	void onDisconnect(const PeerIdT& peerId);
	void printErrorCode(const error_code &ec, const std::string& method);
	
	u16 calculateHandshakeChecksum(const Handshake& hs);
	boost::asio::io_service mService;
	boost::shared_ptr<tcp::acceptor> mAcceptor;

	// TODO: This works only for 7 weeks of server runtime!
	// TODO: This will wreak havoc after 7 weeks.
	// TODO: Reset this watch somehow.
	boost::shared_ptr<Clock::StopWatch> mLocalTime;

	boost::thread mThread;

	EventLoop* mLoop;
	ModulesMap mNetworkModules;
	
	Handshake::SerializationT mHandshakeBuffer;
};

} // namespace Network
} // namespace BFG

#endif
