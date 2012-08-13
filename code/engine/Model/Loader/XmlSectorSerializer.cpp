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

#include <Model/Loader/XmlSectorSerializer.h>

#include <tinyxml.h>
#include <Base/Logger.h>
#include <Model/Loader/Types.h>
#include <Model/Loader/XmlObjectSerializer.h>

namespace BFG {
namespace Loader {

namespace Elements
{
	const std::string Sector("Sector");
	const std::string ObjectList("ObjectList");
}

namespace Attributes
{
	const std::string name("name");
}

XmlSectorSerializer::XmlSectorSerializer(TiXmlElement* sector) :
mOrigin(sector),
mDocument(NULL)
{
	assert(sector);
}

XmlSectorSerializer::XmlSectorSerializer(TiXmlDocument* document) :
mOrigin(NULL),
mDocument(document)
{
	assert(document);
}

void XmlSectorSerializer::read(ReadT& sp)
{
	TiXmlElement* origin = mOrigin->Clone()->ToElement();
	
	const std::string* name = origin->Attribute(Attributes::name);
	
	if (name)
		sp.mName = *name;
	
	TiXmlElement* objectList = origin->FirstChildElement(Elements::ObjectList);
	XmlObjectListSerializer xols(objectList);
	xols.read(sp.mObjects);
}

void XmlSectorSerializer::write(WriteT& sp)
{	
	TiXmlElement* sector = new TiXmlElement(Elements::Sector);
	sector->SetAttribute(Attributes::name, sp.mName);
	
	TiXmlElement* objectList = new TiXmlElement(Elements::ObjectList);
	XmlObjectListSerializer xos(objectList);
	xos.write(sp.mObjects);
	
	if (mOrigin)
		mOrigin->LinkEndChild(sector);
	else
		mDocument->LinkEndChild(sector);

	sector->LinkEndChild(objectList);
}

} // namespace Loader
} // namespace BFG
