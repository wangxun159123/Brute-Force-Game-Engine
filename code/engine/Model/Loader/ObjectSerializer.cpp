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

#include <Model/Loader/ObjectSerializer.h>

#include <Base/CLogger.h>

namespace BFG {
namespace Loader {

void debugOut(const BFG::Loader::ObjectParameter& op)
{
	infolog << "Object: \"" << op.mName << "\"";
	infolog << "--------------------------";
	infolog << "Type: " << op.mType;
	infolog << "Ang Vel: " << op.mAngularVelocity;
	infolog << "Lin Vel: " << op.mLinearVelocity;
	infolog << "Loc: " << op.mLocation;
	infolog << "Connection Ok: " << op.mConnection.good();
	infolog << "... other GO: " << op.mConnection.mConnectedExternToGameObject;
	infolog << "... other Module: " << op.mConnection.mConnectedExternToModule;
	infolog << "##########################";
}

void DummyObjectSerializer::read(ObjectParameter::MapT& objects)
{
	// First Test Object
	ObjectParameter op;
	op.mName = "First Test Object";
	op.mType = "Type1";
	op.mAngularVelocity = v3::ZERO;
	op.mLinearVelocity = v3::UNIT_X;
	
	const std::string connection = "1@Other Object:Other Module:1";
	parseConnection(connection, op.mConnection);

	objects[op.mName] = op;
	
	// Second Test Object
	ObjectParameter op2;
	op2.mName = "Second Test Object";
	op2.mType = "Type2";
	op2.mAngularVelocity = v3::NEGATIVE_UNIT_Y;
	op2.mLinearVelocity = v3::NEGATIVE_UNIT_Z;
	objects[op2.mName] = op2;
}

void DummyObjectSerializer::write(const ObjectParameter::MapT& objects)
{
	if (objects.empty())
		return;

	ObjectParameter::MapT::const_iterator it = objects.begin();
	for (; it != objects.end(); ++it)
		debugOut(it->second);
}

} // namespace Loader
} // namespace BFG
