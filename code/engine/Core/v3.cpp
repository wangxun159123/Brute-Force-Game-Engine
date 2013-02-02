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

#include <Core/v3.h>
#include <stdexcept>
#include <boost\lexical_cast.hpp>
#include <base/Logger.h>

namespace BFG {

void stringToVector3(const std::string& in, v3& out)
{
	std::stringstream ss(in);
	
	ss >> out.x >> std::ws;

	if (ss.peek() == 'f')
		ss.ignore();

	if (ss.peek() == ',')
		ss.ignore();

	ss >> out.y >> std::ws;

	if (ss.peek() == 'f')
		ss.ignore();

	if (ss.peek() == ',')
		ss.ignore();

	ss >> out.z;

	if (ss.fail())
		throw std::runtime_error("stringToVector3: Error while parsing.");
}

v3 loadVector3(XmlTreeT tree)
{
	try
	{
		std::string x = tree->child("x")->elementData();
		errlog << "X: " << x;
		std::string y = tree->child("y")->elementData();
		errlog << "Y: " << y;
		std::string z = tree->child("z")->elementData();
		errlog << "Z: " << z;

		return v3(boost::lexical_cast<f32>(x), 
				  boost::lexical_cast<f32>(y),
				  boost::lexical_cast<f32>(z));
	}
	catch (std::exception e)
	{
		throw std::logic_error(e.what()+std::string(" At loadVector3(...)"));
	}
}

} // namespace BFG
