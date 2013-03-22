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

#ifndef BFG_NETWORKMODULE_H
#define BFG_NETWORKMODULE_H

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/pool/pool.hpp>

#include <Core/Types.h>
#include <EventSystem/Emitter.h>
#include <Network/Event_fwd.h>

namespace BFG {
namespace Network {

class UdpModule : public Emitter, public boost::enable_shared_from_this<UdpModule>
{
public:
	UdpModule(EventLoop* loop, boost::asio::io_service& service, u16 port);
	~UdpModule();
	
private:
	void read();
	void readHandler(const boost::system::error_code &ec, std::size_t bytesTransferred, char* buffer);
	
	void write(boost::asio::const_buffer buffer, std::size_t bytesTransferred);
	void writeHandler(const boost::system::error_code& ec, std::size_t bytesTransferred, boost::asio::const_buffer buffer);
	
	boost::asio::ip::udp::socket mSocket;
	boost::asio::ip::udp::endpoint mRemoteEndpoint;
	boost::pool<> mPool;
};

} // namespace Network
} // namespace BFG

#endif
