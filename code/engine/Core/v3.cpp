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
#include <boost/lexical_cast.hpp>
#include <Base/Logger.h>

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
		std::string x = tree->child("X")->elementData();
		std::string y = tree->child("Y")->elementData();
		std::string z = tree->child("Z")->elementData();

		return v3(boost::lexical_cast<f32>(x),
		          boost::lexical_cast<f32>(y),
		          boost::lexical_cast<f32>(z));
	}
	catch (std::exception& e)
	{
		throw std::logic_error(e.what() + std::string(" At loadVector3(...)"));
	}
}

void saveVector3(const v3& in, XmlTreeT parentNode)
{
	try
	{
		parentNode->addElement("X", boost::lexical_cast<std::string>(in.x));
		parentNode->addElement("Y", boost::lexical_cast<std::string>(in.y));
		parentNode->addElement("Z", boost::lexical_cast<std::string>(in.z));
	}
	catch(std::exception& e)
	{
		throw std::logic_error(e.what()+std::string(" At saveVector3(...)"));
	}
}

} // namespace BFG
