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

#include <Model/Data/Interpreter.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <Model/Enums.hh>

namespace BFG {
namespace Loader {

bool strToBool(const std::string& input, bool& output)
{
	if (boost::iequals(input, "yes")  || 
		boost::iequals(input, "true") ||
		boost::iequals(input, "1"))
	{
		output = true;
		return true;
	}
	else 
	if (boost::iequals(input, "no")  || 
		boost::iequals(input, "false") ||
		boost::iequals(input, "0"))
	{
		output = false;
		return true;
	}

	return false;
}

Property::Value StringToPropertyValue(const std::string& input)
{
	Property::Value result;
	
	// is qv4
	try {
		qv4 output;
		stringToQuaternion4(input, output);
		result = output;
		return result;
	}
	catch (std::runtime_error) {}
	
	// is v3
	try {
		v3 output;
		stringToVector3(input, output);
		result = output;
		return result;
	}
	catch (std::runtime_error) {}

	// is integer
	try
	{
		result = boost::lexical_cast<s32>(input);
		return result;
	}
	catch (boost::bad_lexical_cast &) {}

	// is float
	try
	{
		result = boost::lexical_cast<f32>(input);
		return result;
	}
	catch (boost::bad_lexical_cast &) {}

	// is ID::CameraMode
	try {
		result = ID::asCameraMode(input);
		return result;
	}
	catch (std::out_of_range) {}
	
	// is bool
	if (strToBool(input, result))
		return result;
	
	// is string
	result = stringToArray<128>(input);
	return result;
}

#if 0
GameHandle Interpreter::interpretPathDefinition(const std::string& waypointDefiniton, 
                                                SectorFactory& sectorFactory) const
{
	std::string copyDefinition = waypointDefiniton;

	size_t signPosition = copyDefinition.find_first_of(Sign::interrupt);

	if (signPosition == std::string::npos)
	{
		std::stringstream strStream;
		strStream << "Waypoint definition misses" << Sign::interrupt << "sign to separate prefix.";
		throw LoaderException(strStream.str());
	}

	std::string prefix = copyDefinition.substr(0, signPosition);
	std::string mainString = copyDefinition.substr(signPosition +1, copyDefinition.size());

	if (prefix == Prefix::object)
	{
		return sectorFactory.createWaypoint(mainString);
	}
	else
	if (prefix == Prefix::position)
	{
		infolog << "Absolute waypoint with position prefix is not implemented atm.";
 		v3 position;
 		stringToVector3(mainString, position);
 
 		return sectorFactory.createWaypoint(position);
	}
	else
	{
		std::stringstream strStream;
		strStream << "Unknown prefix: " << prefix << " at waypoint definition.";
		throw LoaderException(strStream.str());
	}
}
#endif

} // namespace Loader
} // namespace BFG
