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

#ifndef BFG_NETWORK_UDP_H
#define BFG_NETWORK_UDP_H

#include <Network/UnreliableHeader.h>

namespace BFG {
namespace Network {

class UdpHeaderFactory
{
	typedef UnreliableHeader HeaderT;

public:
	//! Creates an UnreliableHeader in a provided buffer.
	//! \param[in] buffer The buffer to write the header data in
	static UnreliableHeader create(boost::asio::const_buffer, std::size_t)
	{
		// Make header
		UnreliableHeader header;
		header.mSequenceNumber = 5;
		header.mTimestamp = 0.9f;
		return header;
	}
};

struct Udp
{
	static const std::size_t MAX_PACKET_SIZE_BYTES = 1400;

	typedef UnreliableHeader HeaderT;
	typedef UdpHeaderFactory HeaderFactoryT;
	
	static std::size_t headerSize()
	{
		return HeaderT::SerializationT::size();
	}
};

} // namespace Network
} // namespace BFG

#endif
