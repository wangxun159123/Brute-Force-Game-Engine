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

XmlObjectSerializer::XmlObjectSerializer(TiXmlElement* objectCollection) :
mCollectionOrigin(objectCollection)
{
}

void XmlObjectSerializer::read(ObjectParameter::MapT& objects)
{
	TiXmlElement* collectionOrigin = mCollectionOrigin->Clone()->ToElement();
	readCollection(collectionOrigin, objects);
}

void XmlObjectSerializer::write(const ObjectParameter::MapT& objects)
{
	if (objects.empty())
		return;
	
	writeCollection(objects, mCollectionOrigin);
}

void XmlObjectSerializer::writeCollection(const ObjectParameter::MapT& objects,
                                          TiXmlElement* result)
{
	ObjectParameter::MapT::const_iterator it = objects.begin();
	for (; it != objects.end(); ++it)
	{
		writeOne(it->second, result);
	}
}


void XmlObjectSerializer::writeOne(const ObjectParameter& op,
                                   TiXmlElement* result) const
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

	result->LinkEndChild(object);
}


void XmlObjectSerializer::readCollection(TiXmlElement* objectCollection,
                                         ObjectParameter::MapT& result) const
{
	TiXmlElement* next = objectCollection->FirstChildElement();
	while (next)
	{
		try
		{
			ObjectParameter op;
			readOne(next, op);
			result[op.mName] = op;
		}
		catch (std::runtime_error& ex)
		{
			errlog << ex.what();
		}
		
		next = next->NextSiblingElement(Elements::Object);
	}
}


void XmlObjectSerializer::readOne(const TiXmlElement* object,
                                  ObjectParameter& result) const
{
	assert(object);
	
	const std::string* name = object->Attribute(Attributes::name);
	const std::string* type = object->Attribute(Attributes::type);
	const std::string* pos = object->Attribute(Attributes::position);	
	const std::string* ori = object->Attribute(Attributes::orientation);	
	const std::string* ang = object->Attribute(Attributes::angular_velocity);
	const std::string* lin = object->Attribute(Attributes::linear_velocity);
	const std::string* connection = object->Attribute(Attributes::connection);
	
	if (!name || !type || name->empty() || type->empty())
	{
		throw std::runtime_error
		(
			"XmlObjectSerializer: Stumbled upon Object without"
			" name or type attribute (or with empty one)."
		);
	}
	
	result.mName = *name;
	result.mType = *type;
	
	if (pos) BFG::stringToVector3(*pos, result.mLocation.position);
	if (ori) BFG::stringToQuaternion4(*ori, result.mLocation.orientation);
	if (ang) BFG::stringToVector3(*ang, result.mAngularVelocity);
	if (lin) BFG::stringToVector3(*lin, result.mLinearVelocity);
	if (connection) parseConnection(*connection, result.mConnection);	
}

} // namespace Loader
} // namespace BFG
