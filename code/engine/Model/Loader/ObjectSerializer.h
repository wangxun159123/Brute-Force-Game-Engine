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

#ifndef LOADER_OBJECT_SERIALIZER_H_
#define LOADER_OBJECT_SERIALIZER_H

#include <map>

#include <Base/CLogger.h>
#include <Model/Loader/Types.h>

#include <tinyxml.h>

namespace BFG {
namespace Loader {

typedef std::map<std::string, BFG::Loader::ObjectParameter> ObjectParameterMapT;

class ObjectSerializer
{
public:
	virtual ~ObjectSerializer(){};
	
	virtual void read(ObjectParameterMapT& objects) = 0;
	virtual void write(const ObjectParameterMapT& objects) = 0;
};

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

class DummyObjectSerializer : public ObjectSerializer
{
public:
	virtual void read(ObjectParameterMapT& objects)
	{
		// First Test Object
		ObjectParameter op;
		op.mName = "Blah";
		op.mType = "Blub";
		op.mAngularVelocity = v3::ZERO;
		op.mLinearVelocity = v3::UNIT_X;
		
		const std::string connection = "1@Gate 1 inner:Inner Gate:1";
		parseConnection(connection, op.mConnection);

		objects[op.mName] = op;
		
		// Second Test Object
		ObjectParameter op2;
		op2.mName = "Miau";
		op2.mType = "Tralala";
		op2.mAngularVelocity = v3::NEGATIVE_UNIT_Y;
		op2.mLinearVelocity = v3::NEGATIVE_UNIT_Z;
		objects[op2.mName] = op2;
	}

	virtual void write(const ObjectParameterMapT& objects)
	{
		debugOut(objects.begin()->second);
	}
};

} // namespace Loader
} // namespace BFG

#endif
