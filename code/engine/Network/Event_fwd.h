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

#ifndef BFG_NETWORKEVENT_FWD_H
#define BFG_NETWORKEVENT_FWD_H

#include <boost/tuple/tuple.hpp>
#include <boost/variant.hpp>

#include <EventSystem/Event_fwd.h>

#include <Core/CharArray.h>
#include <Core/Types.h>

#include <Network/Enums.hh>

namespace BFG {
namespace Network {

typedef boost::tuple
<
	u32,
	GameHandle,
	GameHandle,
	size_t,
	CharArray512T
> NetworkPayloadType;

typedef Event
<
	ID::NetworkAction,
	NetworkPayloadType,
	GameHandle,
	GameHandle
> NetworkPacketEvent;

typedef boost::tuple
<
	CharArray128T,	// ip
	CharArray128T	// port
> NetworkEndpointT;

typedef boost::variant
<
	u16,
	NetworkEndpointT,
	GameHandle,
	PeerIdT
> NetworkControlPayloadT;

typedef Event
<
	ID::NetworkAction,
	NetworkControlPayloadT,
	GameHandle,
	GameHandle
> NetworkControlEvent;

std::string NETWORK_API debug(const NetworkPacketEvent& e);

} // namespace Network
} // namespace BFG

#endif
