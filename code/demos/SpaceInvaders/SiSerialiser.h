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


#ifndef SI_SERIALISER
#define SI_SERIALISER

#include <Model/Property/Concept.h>
#include "Globals.h"

using namespace BFG;

struct SiSerialiser : BFG::Property::Serialiser
{
	virtual ~SiSerialiser() {}

	//! \brief VarId to String serialisation
	//! \param[in] in VarId which shall be serialised to a String
	//! \param[out] out Result
	//! \return true if serialisation was successful, false otherwise
	virtual bool varToString(BFG::Property::ValueId::VarIdT in,
							 std::string& out) const
	{
		std::cout << "STUB: SiSerialiser::varToString [" << in << "]" << std::endl;
		return false;
	}

	//! \brief String to VarId serialisation
	//! \param[in] in String which shall be serialised to a VarId
	//! \param[out] out Result
	//! \return true if serialisation was successful, false otherwise
	virtual bool stringToVar(const std::string& in,
							 BFG::Property::ValueId::VarIdT& out) const
	{
		if (in == "ProjectileSpeed")
		{
			out = ID_PROJECTILE_SPEED;
			return true;
		}
		else if (in == "ProjectileSpawnDistance")
		{
			out = ID_PROJECTILE_SPAWN_DISTANCE;
			return true;
		}
		return false;
	}
};

#endif