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

class TiXmlElement;

namespace BFG {
namespace Loader {

class MODEL_API XmlObjectSerializer : public ObjectSerializer
{
public:
	//! \param object Must point to an existing Object for
	//! reading, and to a higher parent element for writing a whole
	//! Object element into.
	XmlObjectSerializer(TiXmlElement* object);
	
	virtual void read(ReadT& object);
	virtual void write(WriteT& object);

private:
	TiXmlElement* mOrigin;
};

class MODEL_API XmlObjectListSerializer : public ObjectListSerializer
{
public:
	//! \param objectCollection Must point to an existing ObjectList for
	//! reading, and to a higher parent element for writing a whole
	//! ObjectList element into.
	XmlObjectListSerializer(TiXmlElement* objectCollection);
	
	virtual void read(ReadT& objects);
	virtual void write(WriteT& objects);

private:
	TiXmlElement* mOrigin;
};

} // namespace Loader
} // namespace BFG

#endif
