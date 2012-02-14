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

#ifndef LOADER_XML_SECTOR_SERIALIZER_H_
#define LOADER_XML_SECTOR_SERIALIZER_H

#include <Model/Defs.h>
#include <Model/Loader/Types.h>

#include <Model/Loader/SectorSerializer.h>

class TiXmlDocument;
class TiXmlElement;

namespace BFG {
namespace Loader {

class MODEL_API XmlSectorSerializer : public SectorSerializer
{
public:
	//! \param sector Must point to an existing Sector-element for
	//! reading or to a higher parent element for writing a whole
	//! sector element into.
	XmlSectorSerializer(TiXmlElement* sector);
	
	//! \param document Must point to an existing document. A new
	//! Sector-element will be created as root node.
	XmlSectorSerializer(TiXmlDocument* document);
	
	virtual void read(SectorParameter& sp);
	virtual void write(const SectorParameter& sp);
	
private:
	TiXmlElement* mOrigin;
	TiXmlDocument* mDocument;
};

} // namespace Loader
} // namespace BFG

#endif
