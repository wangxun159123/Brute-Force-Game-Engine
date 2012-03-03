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

#include <Model/Loader/XmlObjectSerializer.h>

#include <map>
#include <tinyxml.h>
#include <Base/CLogger.h>

namespace BFG {
namespace Loader {

namespace Elements
{
	const std::string Object("Object");
}

namespace Attributes
{
	const std::string name("name");
	const std::string type("type");
	const std::string position("position");
	const std::string orientation("orientation");
	const std::string angular_velocity("angular_velocity");
	const std::string linear_velocity("linear_velocity");
	const std::string connection("connection");
}

XmlObjectSerializer::XmlObjectSerializer(TiXmlElement* object) :
mOrigin(object)
{
	assert(object);
}

void XmlObjectSerializer::read(ReadT& object)
{
	assert(mOrigin);
	
	const std::string* name = mOrigin->Attribute(Attributes::name);
	const std::string* type = mOrigin->Attribute(Attributes::type);
	const std::string* pos = mOrigin->Attribute(Attributes::position);	
	const std::string* ori = mOrigin->Attribute(Attributes::orientation);	
	const std::string* ang = mOrigin->Attribute(Attributes::angular_velocity);
	const std::string* lin = mOrigin->Attribute(Attributes::linear_velocity);
	const std::string* connection = mOrigin->Attribute(Attributes::connection);
	
	if (!name || !type || name->empty() || type->empty())
	{
		throw std::runtime_error
		(
			"XmlObjectSerializer: Stumbled upon Object without"
			" name or type attribute (or with empty one)."
		);
	}
	
	object.mName = *name;
	object.mType = *type;
	
	if (pos) BFG::stringToVector3(*pos, object.mLocation.position);
	if (ori) BFG::stringToQuaternion4(*ori, object.mLocation.orientation);
	if (ang) BFG::stringToVector3(*ang, object.mAngularVelocity);
	if (lin) BFG::stringToVector3(*lin, object.mLinearVelocity);
	if (connection) parseConnection(*connection, object.mConnection);	
}

void XmlObjectSerializer::write(WriteT& op)
{
	TiXmlElement* object = new TiXmlElement(Elements::Object);

	std::stringstream position;
	std::stringstream orientation;
	std::stringstream linear_velocity;
	std::stringstream angular_velocity;

	position << op.mLocation.position;
	orientation << op.mLocation.orientation;
	linear_velocity << op.mLinearVelocity;
	angular_velocity << op.mAngularVelocity;

	object->SetAttribute(Attributes::name, op.mName);
	object->SetAttribute(Attributes::type, op.mType);
	object->SetAttribute(Attributes::position, position.str());
	object->SetAttribute(Attributes::orientation, orientation.str());
	object->SetAttribute(Attributes::linear_velocity, linear_velocity.str());
	object->SetAttribute(Attributes::angular_velocity, angular_velocity.str());
	
	if (op.mConnection.good())
		object->SetAttribute(Attributes::connection, op.mConnection.str());

	mOrigin->LinkEndChild(object);
}

XmlObjectListSerializer::XmlObjectListSerializer(TiXmlElement* objectList) :
mOrigin(objectList)
{
	assert(objectList);
}

void XmlObjectListSerializer::read(ReadT& objects)
{
	TiXmlElement* collectionOrigin = mOrigin->Clone()->ToElement();

	TiXmlElement* next = collectionOrigin->FirstChildElement();
	while (next)
	{
		try
		{
			ObjectParameter op;
			XmlObjectSerializer xos(next);
			xos.read(op);
			objects.insert(std::make_pair(op.mName, op));
		}
		catch (std::runtime_error& ex)
		{
			errlog << ex.what();
		}
		
		next = next->NextSiblingElement(Elements::Object);
	}
}

void XmlObjectListSerializer::write(WriteT& objects)
{
	if (objects.empty())
		return;
	
	WriteT::const_iterator it = objects.begin();
	for (; it != objects.end(); ++it)
	{
		XmlObjectSerializer xos(mOrigin);
		xos.write(it->second);
	}
}

} // namespace Loader
} // namespace BFG
