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

#include <Network/Client.h>

#include <Base/Logger.h>
#include <Network/NetworkModule.h>

namespace BFG {
namespace Network{

Client::Client(EventLoop* loop) :
mLoop(loop),
mLocalTime(new Clock::StopWatch(Clock::milliSecond)),
mRTT(Clock::milliSecond)
{
	mLocalTime->start();

	mResolver.reset(new tcp::resolver(mService));

	mLoop->connect(ID::NE_CONNECT, this, &Client::controlEventHandler);
	mLoop->connect(ID::NE_DISCONNECT, this, &Client::controlEventHandler);
	mLoop->connect(ID::NE_SHUTDOWN, this, &Client::controlEventHandler);

	mNetworkModule = new NetworkModule(mLoop, mService, 0, mLocalTime);
}

Client::~Client()
{
	stop();
	mLoop->disconnect(ID::NE_CONNECT, this);
	mLoop->disconnect(ID::NE_DISCONNECT, this);
	mLoop->disconnect(ID::NE_SHUTDOWN, this);

	if (mResolver)
		mResolver->cancel();

	mResolver.reset();
}

void Client::stop()
{
	dbglog << "Client::stop";
	mService.stop();
	mThread.join();

	delete mNetworkModule;
	mNetworkModule = NULL;
}

void Client::startConnecting(const std::string& ip, const std::string& port)
{
	dbglog << "NetworkModule::startConnecting";
	boost::asio::ip::tcp::resolver::query query(ip, port);
	mResolver->async_resolve(query, boost::bind(&Client::resolveHandler, this, _1, _2));
}

void Client::readHandshake()
{
	dbglog << "Client::readHandshake";
	boost::asio::async_read
	(
		*(mNetworkModule->socket()), 
		boost::asio::buffer(mHandshakeBuffer),
		boost::asio::transfer_exactly(Handshake::SerializationT::size()),
		bind(&Client::readHandshakeHandler, this, _1, _2)
	);
}

void Client::resolveHandler(const error_code &ec, tcp::resolver::iterator it)
{ 
	dbglog << "NetworkModule::resolveHandler";
	if (!ec) 
	{
		mRTT.start();
		mNetworkModule->socket()->async_connect(*it, bind(&Client::connectHandler, this, _1)); 
	}
	else
		printErrorCode(ec, "resolveHandler");
}

void Client::connectHandler(const error_code &ec)
{
	dbglog << "Client::connectHandler";
	if (!ec) 
	{
		readHandshake();
	}
	else
	{
		printErrorCode(ec, "connectHandler");
	}
}

void Client::readHandshakeHandler(const error_code &ec, size_t bytesTransferred)
{
	dbglog << "Client::readHandshakeHandler (" << bytesTransferred << ")";
	if (!ec) 
	{
		Handshake hs;
		hs.deserialize(mHandshakeBuffer);
		
		u16 hsChecksum = hs.mChecksum;
		u16 ownHsChecksum = calculateHandshakeChecksum(hs);
		
		if (ownHsChecksum != hsChecksum)
		{
			warnlog << std::hex << std::uppercase 
				<< "readHandshakeHandler: Got bad PeerId (Own CRC: "
				<< ownHsChecksum
				<< " Rcvd CRC: "
				<< hsChecksum
				<< "). Disconnecting Peer.";

			// Peer sends crap? Bye bye!
			mNetworkModule->socket().reset();
			return;
		}
		else
		{
			dbglog << "Received peer ID: " << hs.mPeerId;
			mPeerId = hs.mPeerId;

			u32 offset = calculateServerTimestampOffset(hs.mTimestamp);

			mNetworkModule->setTimestampOffset(offset);

			mNetworkModule->startReading();

			Emitter e(mLoop);
			e.emit<ControlEvent>(ID::NE_CONNECTED, mPeerId);
		}
	}
}

void Client::controlEventHandler(ControlEvent* nce)
{
	switch(nce->getId())
	{
	case ID::NE_CONNECT:
		onConnect(boost::get<EndpointT>(nce->getData()));
		break;
	case ID::NE_DISCONNECT:
	case ID::NE_SHUTDOWN:
		onDisconnect(0);
		break;
	default:
		warnlog << "Client: Can't handle event with ID: "
		        << nce->getId();
		break;
	}

}

void Client::onConnect(const EndpointT& endpoint)
{
	startConnecting(endpoint.get<0>().data(), endpoint.get<1>().data());
	mThread = boost::thread(boost::bind(&boost::asio::io_service::run, &mService));
}

void Client::onDisconnect(const PeerIdT& peerId)
{
	stop();
	Emitter e(mLoop);
	e.emit<ControlEvent>(ID::NE_DISCONNECTED, peerId);
}

u16 Client::calculateHandshakeChecksum(const Handshake& hs)
{
	boost::crc_16_type result;
	result.process_bytes(&(hs.mPeerId), sizeof(PeerIdT));
	return result.checksum();
}

u32 Client::calculateServerTimestampOffset(u32 serverTimestamp)
{
	// https://en.wikipedia.org/wiki/Cristian%27s_algorithm
	// offset = tS + dP/2 - tC
	u32 dP = mRTT.stop();
	u32 tC = mLocalTime->stop();
	u32 offset = serverTimestamp + dP / 2 - tC;

	dbglog << "Calculated server Timestamp Offset: " << offset 
		<< " with RTT of " << dP;
	dbglog << "LocalTime was: " << tC;

	return offset;
}

void Client::printErrorCode(const error_code &ec, const std::string& method)
{
	warnlog << "[" << method << "] Error Code: " << ec.value() << ", message: " << ec.message();
}

} // namespace Network
} // namespace BFG
