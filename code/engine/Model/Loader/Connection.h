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

#ifndef LOADER_CONNECTION_H_
#define LOADER_CONNECTION_H_

#include <string>

#include <Core/Types.h>
#include <Model/Defs.h>

namespace BFG {
namespace Loader {

struct Connection
{
	Connection() : mConnectedLocalAt(0), mConnectedExternAt(0) {}

	bool good() const;
	
	std::string str() const;

	BFG::u32 mConnectedLocalAt;
	std::string mConnectedExternToGameObject;
	std::string mConnectedExternToModule;
	BFG::u32 mConnectedExternAt;
};

//! \brief Utility function for Interpreter
//! \param[in] input Example: "Wing:5" or "Body:1". Both parts are mandatory.
//! \param[out] connection Result of parsing
//! \exception std::runtime_error If the input couldn't be parsed. 
void MODEL_API parseConnection(const std::string& input,
                               Connection& connection);

} // namespace Loader
} // namespace BFG

#endif
