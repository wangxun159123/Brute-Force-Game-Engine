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

#ifndef BFG_NETWORK_EVENT_H
#define BFG_NETWORK_EVENT_H

#include <EventSystem/Event.h>
#include <Network/Event_fwd.h>

namespace BFG {
namespace Network {

struct DataPayload
{
	DataPayload() :
	mAppEventId(0),
	mAppDestination(0),
	mAppSender(0),
	mAppDataLen(0)
	{}
	
	DataPayload(u32 appEventId, GameHandle appDestination,
	            GameHandle appSender, size_t appDataLen,
	            CharArray512T appData) :
	mAppEventId(appEventId),
	mAppDestination(appDestination),
	mAppSender(appSender),
	mAppDataLen(appDataLen),
	mAppData(appData)
	{}
	     
	//! Id of the actual event on application-level
	u32 mAppEventId;

	//! GameHandle of the actual receiver on application-level
	GameHandle mAppDestination;

	//! GameHandle of the actual sender on application-level
	GameHandle mAppSender;
	
	//! Actual length (not capacity) of the to be send/received data.
	size_t mAppDataLen;
	
	//! Actual data to be transmitted on application-level (serialized)
	CharArray512T mAppData;
};

} // namespace Network
} // namespace BFG

#endif
