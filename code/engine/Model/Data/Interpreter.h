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
#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include <Model/Defs.h>
#include <Model/Property/Value.h>

namespace BFG {


namespace Loader {

//! \brief Utility function for Interpreter
//! Not all types of the GoePayloadT are supported though.
//! \exception std::runtime_error If the input couldn't be converted. 
Property::Value MODEL_API StringToPropertyValue(const std::string& input);

bool MODEL_API strToBool(const std::string& input, bool& output);


//! \todo Move it to racer!
#if 0	                                 
	GameHandle interpretPathDefinition(const std::string& waypointDefiniton,
	                                   SectorFactory& sectorFactory) const;	
#endif

} // namespace Loader
} // namespace BFG

#endif
