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

#include <Network/Server.h>

#include <Core/Utils.h>

#include <Network/NetworkModule.h>
#include <Network/Event.h>

namespace BFG {
namespace Network{

using namespace boost::asio::ip;
using namespace boost::system;

Server::Server(EventLoop* loop) :
mLoop(loop),
mLocalTime(new Clock::StopWatch(Clock::milliSecond))
{

	mLocalTime->start();

	mLoop->connect(ID::NE_LISTEN, this, &Server::controlEventHandler);
	mLoop->connect(ID::NE_DISCONNECT, this, &Server::controlEventHandler);
	mLoop->connect(ID::NE_SHUTDOWN, this, &Server::controlEventHandler);

	mLoop->connect(ID::NE_RECEIVED, this, &Server::dataPacketEventHandler);
}

Server::~Server()
{
	stop();
	mLoop->disconnect(ID::NE_LISTEN, this);
	mLoop->disconnect(ID::NE_DISCONNECT, this);
	mLoop->disconnect(ID::NE_SHUTDOWN, this);

	mLoop->disconnect(ID::NE_RECEIVED, this);
}

void Server::stop()
{
	dbglog << "Server::stop";
	mService.stop();
	mThread.join();

	if (!mNetworkModules.empty())
	{
		for(ModulesMap::iterator it = mNetworkModules.begin(); it != mNetworkModules.end(); ++it)
			delete it->second;

		mNetworkModules.clear();
	}
}

void Server::startAccepting()
{
	dbglog << "Server::startAccepting";
	
	PeerIdT peerId = generateNetworkHandle();
	NetworkModule* netModule = new NetworkModule(mLoop, mService, peerId, mLocalTime);
	mNetworkModules.insert(std::make_pair(peerId, netModule));

	dbglog << "Created Networkmodule(" << netModule << ")";
	mAcceptor->async_accept(*netModule->socket(), bind(&Server::acceptHandler, this, _1, peerId));
}

void Server::acceptHandler(const boost::system::error_code &ec, PeerIdT peerId)
{ 
	dbglog << "Client connected: "
	       << mNetworkModules[peerId]->socket()->remote_endpoint().address()
	       << ":" 
	       << mNetworkModules[peerId]->socket()->remote_endpoint().port();
	if (!ec) 
	{ 
		sendHandshake(peerId);
		startAccepting();
	}
	else
	{
		printErrorCode(ec, "acceptHandler");
	}
}

void Server::sendHandshake(PeerIdT peerId)
{
	dbglog << "Server::sendHandshake peer ID: " << peerId;
	Handshake hs;
	hs.mPeerId = peerId;
	hs.mTimestamp = mLocalTime->stop();
	hs.mChecksum = calculateHandshakeChecksum(hs);

	hs.serialize(mHandshakeBuffer);
	
	boost::asio::async_write
	(
		*(mNetworkModules[peerId]->socket()),
		boost::asio::buffer(mHandshakeBuffer.data(), Handshake::SerializationT::size()),
		boost::bind(&Server::writeHandshakeHandler, this, _1, _2, peerId)
	);
}

void Server::writeHandshakeHandler(const error_code &ec, std::size_t bytesTransferred, PeerIdT peerId)
{
	dbglog << "Server: peer ID was sent";
	Emitter e(mLoop);
	e.emit<ControlEvent>(ID::NE_CONNECTED, peerId);
	mNetworkModules[peerId]->startReading();
}

void Server::controlEventHandler(ControlEvent* e)
{
	switch(e->getId())
	{
	case ID::NE_LISTEN:
		onListen(boost::get<u16>(e->getData()));
		break;
	case ID::NE_DISCONNECT:
		onDisconnect(boost::get<PeerIdT>(e->getData()));
		break;
	case ID::NE_SHUTDOWN:
		stop();
		break;
	default:
		warnlog << "Server: Can't handle event with ID: "
		        << e->getId();
		break;
	}
}

void Server::dataPacketEventHandler(DataPacketEvent* e)
{
	switch(e->getId())
	{
	case ID::NE_RECEIVED:
	{
		DataPayload& payload = e->getData();

		switch(payload.mAppEventId)
		{
		case ID::NE_TIMESYNC:
		{
			dbglog << "Got time sync request from PeerId: " << e->mSender;
			u32 timestamp = mLocalTime->stop();
			CharArray512T ca512;
			memcpy(ca512.data(), &timestamp, sizeof(u32));
			Network::DataPayload payload(ID::NE_TIMESYNC, 0, 0, sizeof(u32), ca512);
			mNetworkModules[e->mSender]->queueTimeCriticalPacket(payload);
		}
		}

	}
	}
}

void Server::onListen(const u16 port)
{
	if (!mAcceptor)
	{
		mAcceptor.reset(new tcp::acceptor(mService, tcp::endpoint(tcp::v4(), port)));
		dbglog << "Server started to listen to port " << port;
		startAccepting();
		mThread = boost::thread(boost::bind(&boost::asio::io_service::run, &mService));
	}
	else
	{
		warnlog << "Server is allready listening!";
	}
}

void Server::onDisconnect(const PeerIdT& peerId)
{
	ModulesMap::iterator it = mNetworkModules.find(peerId);
	if (it != mNetworkModules.end())
	{
		delete mNetworkModules[peerId];
		mNetworkModules.erase(it);
		Emitter e(mLoop);
		e.emit<ControlEvent>(ID::NE_DISCONNECTED, peerId);
	}
}

u16 Server::calculateHandshakeChecksum(const Handshake& hs)
{
	boost::crc_16_type result;
	result.process_bytes(&(hs.mPeerId), sizeof(PeerIdT));
	return result.checksum();
}

void Server::printErrorCode(const error_code &ec, const std::string& method)
{
	warnlog << "[" << method << "] Error Code: " << ec.value() << ", message: " << ec.message();
}



} // namespace Network
} // namespace BFG
