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

#include <Base/CLogger.h>
#include <Base/Pause.h>
#include <Core/CharArray.h>
#include <Core/Location.h>
#include <Core/ShowException.h>
#include <Core/Types.h>
#include <Core/Utils.h>
#include <EventSystem/Event.h>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/tuple/tuple.hpp>


#define TESTID 54321
#define A_NETWORK_SEND 7373
#define A_NETWORK_RECEIVED 1337

using namespace BFG;


typedef Event
<
	int,
	v3,
	GameHandle,
	GameHandle
> VectorEvent;

typedef boost::tuple
<
	int,
	GameHandle,
	GameHandle,
	size_t,
	CharArray512T
> NetworkPayloadType;

typedef Event
<
	int,
	NetworkPayloadType,
	GameHandle,
	GameHandle
> NetworkEvent;

namespace boost {
namespace serialization {

	template<class Archive>
	void serialize(Archive & ar, BFG::v3& v, const unsigned int version)
	{
		ar & v.x;
		ar & v.y;
		ar & v.z;
	}

} // namespace serialization
} // namespace boost

void onTest(const v3& vec)
{
	std::cout << "Got 'TESTID': " << vec << std::endl;
}

void vectorEventHandler(VectorEvent* ve)
{
	switch(ve->getId())
	{
	case TESTID:
		onTest(ve->getData());
		break;
	default:
		throw std::runtime_error("ID wrong");
		break;
	}
}

void eventHandler(NetworkEvent* ne)
{
	switch(ne->getId())
	{
	case A_NETWORK_RECEIVED:
		{
			NetworkPayloadType payload = ne->getData();

			std::stringstream ss;
			ss.str(payload.get<4>().c_array());
			boost::archive::text_iarchive ia(ss);

			switch(payload.get<0>())
			{
			case TESTID:
				{
					v3 vec;
					ia >> vec;
					onTest(vec);

//					VectorEvent ve(payload.get<0>(), vec, payload.get<1>(), payload.get<2>());
//					vectorEventHandler(&ve);
				}
				break;
			default:
				break;
			}
		}

		break;
	default:
		throw std::runtime_error("ID wrong");
		break;
	}
}

