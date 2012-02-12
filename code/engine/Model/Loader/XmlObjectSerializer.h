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

#ifndef LOADER_XML_OBJECT_SERIALIZER_H_
#define LOADER_XML_OBJECT_SERIALIZER_H

#include <Model/Defs.h>
#include <Model/Loader/Types.h>
#include <Model/Loader/ObjectSerializer.h>

class TiXmlElement;

namespace BFG {
namespace Loader {

class MODEL_API XmlObjectSerializer : public ObjectSerializer
{
public:
	//! \param objectCollection Must point to an existing ObjectList for
	//! reading, and to a higher parent element for writing a whole
	//! ObjectList element into.
	XmlObjectSerializer(TiXmlElement* objectCollection);
	
	virtual void read(ObjectParameter::MapT& objects);
	virtual void write(const ObjectParameter::MapT& objects);

private:
	void writeCollection(const ObjectParameter::MapT& objects,
	                     TiXmlElement* result);

	void writeOne(const ObjectParameter& op,
	              TiXmlElement* result) const;

	void readCollection(TiXmlElement* objectCollection,
	                    ObjectParameter::MapT& result) const;
	
	void readOne(const TiXmlElement* objectElement,
	             ObjectParameter& result) const;

	TiXmlElement* mCollectionOrigin;
};

} // namespace Loader
} // namespace BFG

#endif
