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

#include <Model/Loader/XmlAdapterSerializer.h>

#include <tinyxml.h>
#include <boost/lexical_cast.hpp>
#include <Base/CLogger.h>

namespace BFG {
namespace Loader {

namespace Elements
{
	const std::string Adapter("Adapter");
	const std::string AdapterConfig("AdapterConfig");
}

namespace Attributes
{
	// <Adapter>
	const std::string id("id");
	const std::string position("position");
	const std::string orientation("orientation");

	// <AdapterConfig>
	const std::string name("name");

}

XmlAdapterSerializer::XmlAdapterSerializer(TiXmlElement* adapter) :
mOrigin(adapter)
{
	assert(adapter);
}

void XmlAdapterSerializer::read(ReadT& a)
{
	TiXmlElement* origin = mOrigin->Clone()->ToElement();
	
	const std::string* id = origin->Attribute(Attributes::id);
	const std::string* position = origin->Attribute(Attributes::position);
	const std::string* orientation = origin->Attribute(Attributes::orientation);
	
	if (id)
		a.mIdentifier = boost::lexical_cast<u32>(*id);
	if (position)
		stringToVector3(*position, a.mParentPosition);
	if (orientation)
		stringToQuaternion4(*orientation, a.mParentOrientation);
}

void XmlAdapterSerializer::write(WriteT& a)
{
	TiXmlElement* adapter = new TiXmlElement(Elements::Adapter);
	adapter->SetAttribute(Attributes::id, a.mIdentifier);
	
	std::stringstream pos, ori;
	pos << a.mParentPosition;
	ori << a.mParentOrientation;
	
	adapter->SetAttribute(Attributes::position, pos.str());
	adapter->SetAttribute(Attributes::orientation, ori.str());
	
	mOrigin->LinkEndChild(adapter);
}

XmlAdapterConfigSerializer::XmlAdapterConfigSerializer(TiXmlElement* adapterConfig) :
mOrigin(adapterConfig)
{
	assert(adapterConfig);
}

void XmlAdapterConfigSerializer::read(ReadT& a)
{
	TiXmlElement* origin = mOrigin->Clone()->ToElement();

	const std::string* name = origin->Attribute(Attributes::name);
	
	if (!name)
		throw std::runtime_error
			("Stumbled upon AdapterConfig without name attribute.");
	
	TiXmlElement* next = origin->FirstChildElement(Elements::Adapter);
	while (next)
	{
		XmlAdapterSerializer xas(next);
		Adapter adapter;
		xas.read(adapter);
		a.first = *name;
		a.second.push_back(adapter);
		next = next->NextSiblingElement(Elements::Adapter);
	}
}

void XmlAdapterConfigSerializer::write(WriteT& a)
{
	TiXmlElement* adapterConfig = new TiXmlElement(Elements::AdapterConfig);

	adapterConfig->SetAttribute(Attributes::name, a.first);

	typedef AdapterConfigT::second_type AdapterListT;
	const AdapterListT& adapterList = a.second;

	XmlAdapterSerializer xas(adapterConfig);
	AdapterListT::const_iterator it = adapterList.begin();
	for (; it != adapterList.end(); ++it)
	{
		xas.write(*it);
	}
	
	mOrigin->LinkEndChild(adapterConfig);
}

} // namespace Loader
} // namespace BFG
