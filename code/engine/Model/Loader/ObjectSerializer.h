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

#include <Model/Loader/Types.h>

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

void debugOut(const BFG::Loader::ObjectParameter& op);

class DummyObjectSerializer : public ObjectSerializer
{
public:
	virtual void read(ObjectParameterMapT& objects);
	virtual void write(const ObjectParameterMapT& objects);};

} // namespace Loader
} // namespace BFG

#endif
