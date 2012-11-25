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

#ifndef NETWORKED_H_
#define NETWORKED_H_

#include <boost/foreach.hpp>

#include <Core/CharArray.h>
#include <Model/Property/Concept.h>
#include <Network/Event_fwd.h>
#include <Physics/Event_fwd.h>

using namespace BFG;

namespace BFG
{

class Networked : public Property::Concept
{
public:
	Networked(GameObject& owner, PluginId pid) :
	Property::Concept(owner, "Networked", pid),
	mSynchronizationMode(ID::SYNC_MODE_NETWORK_NONE)
	{
		require("Physical");

		mPhysicsActions.push_back(ID::PE_POSITION);
		mPhysicsActions.push_back(ID::PE_ORIENTATION);
		BOOST_FOREACH(ID::PhysicsAction action, mPhysicsActions)
		{
			loop()->connect(action, this, &Networked::onPhysicsEvent, ownerHandle());
		}

		mNetworkActions.push_back(ID::NE_RECEIVED);
		BOOST_FOREACH(ID::NetworkAction action, mNetworkActions)
		{
			loop()->connect(action, this, &Networked::onNetworkEvent, ownerHandle());
		}

		requestEvent(ID::GOE_SYNCHRONIZATION_MODE);
	}

	~Networked()
	{
		BOOST_FOREACH(ID::PhysicsAction action, mPhysicsActions)
		{
			loop()->disconnect(action, this);
		}
		BOOST_FOREACH(ID::NetworkAction action, mNetworkActions)
		{
			loop()->disconnect(action, this);
		}
	}

	void onSynchronizationMode(ID::SynchronizationMode mode)
	{
		mSynchronizationMode = mode;

		dbglog << "Networked: setting synchronization mode to " << mode;
	}

	void internalOnEvent(EventIdT action,
		                 Property::Value payload,
		                 GameHandle module,
		                 GameHandle sender)
	{
		switch(action)
		{
		case ID::GOE_SYNCHRONIZATION_MODE:
		{
			onSynchronizationMode(static_cast<ID::SynchronizationMode>(static_cast<s32>(payload)));
		}
		}
	}

	void onNetworkEvent(Network::NetworkPacketEvent* e)
	{
		// Don't receive if not either READ or RW
		if (!(mSynchronizationMode == ID::SYNC_MODE_NETWORK_READ || mSynchronizationMode == ID::SYNC_MODE_NETWORK_RW))
			return;

		switch(e->getId())
		{
		case ID::NE_RECEIVED:
		{
			const BFG::Network::NetworkPayloadType& payload = e->getData();

			switch(boost::get<0>(payload))
			{
			case ID::PE_UPDATE_POSITION:
			{
				assert(ownerHandle() == boost::get<1>(payload));

				std::string vec(boost::get<4>(payload).data(), boost::get<3>(payload));
				v3 v;
				stringToVector3(vec, v);
				dbglog << "Networked:onNetworkEvent: Vector: " << v;
				emit<Physics::Event>(ID::PE_UPDATE_POSITION, v, ownerHandle());
				break;
			}
			case ID::PE_UPDATE_ORIENTATION:
			{
				assert(ownerHandle() == boost::get<1>(payload));

				std::string ori(boost::get<4>(payload).data(), boost::get<3>(payload));
				qv4 o;
				stringToQuaternion4(ori, o);
				dbglog << "Networked:onNetworkEvent: Quaternion: " << o;
				emit<Physics::Event>(ID::PE_UPDATE_ORIENTATION, o, ownerHandle());
				break;
			}
			}
		}
		}
	}

	void onPhysicsEvent(Physics::Event* e)
	{
		switch(e->getId())
		{
			// 		case ID::PE_FULL_SYNC:
			// 			onFullSync(boost::get<Physics::FullSyncData>(e->getData()));
			// 			break;

		case ID::PE_POSITION:
			onPosition(boost::get<v3>(e->getData()));
			break;

		case ID::PE_ORIENTATION:
			onOrientation(boost::get<qv4>(e->getData()));
			break;

		default:
			warnlog << "Networked: Can't handle event with ID: "
				<< e->getId();
			break;
		}
	}

	void onOrientation(const qv4& newOrientation)
	{
		// Don't send if not either WRITE or RW
		if (!(mSynchronizationMode == ID::SYNC_MODE_NETWORK_WRITE || mSynchronizationMode == ID::SYNC_MODE_NETWORK_RW))
			return;

		dbglog << "Networked:onOrientation: " << newOrientation;
			
		std::stringstream ss;
		ss << newOrientation;

		CharArray512T ca512 = stringToArray<512>(ss.str());
			
		BFG::Network::NetworkPayloadType payload = 
			boost::make_tuple
			(
				ID::PE_UPDATE_ORIENTATION, 
				ownerHandle(),
				ownerHandle(),
				ss.str().length(),
				ca512
			);

		emit<BFG::Network::NetworkPacketEvent>(BFG::ID::NE_SEND, payload);

	}

	void onPosition(const v3& newPosition)
	{
		// Don't send if not either WRITE or RW
		if (!(mSynchronizationMode == ID::SYNC_MODE_NETWORK_WRITE || mSynchronizationMode == ID::SYNC_MODE_NETWORK_RW))
			return;

		dbglog << "Networked:onPosition: " << newPosition;
			
		std::stringstream ss;
		ss << newPosition;

		CharArray512T ca512 = stringToArray<512>(ss.str());
			
		BFG::Network::NetworkPayloadType payload = 
			boost::make_tuple
			(
				ID::PE_UPDATE_POSITION, 
				ownerHandle(),
				ownerHandle(),
				ss.str().length(),
				ca512
			);

		emit<BFG::Network::NetworkPacketEvent>(BFG::ID::NE_SEND, payload);
	}

private:
	std::vector<ID::PhysicsAction> mPhysicsActions;
	std::vector<ID::NetworkAction> mNetworkActions;

	ID::SynchronizationMode mSynchronizationMode;
};

} // namespace BFG

#endif