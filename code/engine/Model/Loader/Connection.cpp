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

#include <Model/Loader/Connection.h>

#include <sstream>

// In order to make this work for Visual Studio 2008, we disabled (/Zc:wchar_t).
// If you want to dig deeper into the problem, look out for
// BOOST_REGEX_HAS_OTHER_WCHAR_T in <boost/regex/config.hpp> 
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

namespace BFG {
namespace Loader {

bool Connection::good() const
{
	return ! mConnectedExternToModule.empty() &&
		mConnectedExternAt > 0 &&
		mConnectedLocalAt > 0;
}

std::string Connection::str() const
{
	std::stringstream ss;
	ss << mConnectedLocalAt << "@"
	   << mConnectedExternToGameObject << ":"
	   << mConnectedExternToModule << ":"
	   << mConnectedExternAt;
	return ss.str();
}

void parseConnection(const std::string& input,
                     Connection& connection)
{
	if (input.empty())
		return;

	static const std::string rgx(
		"("
			"[[:digit:]]+"
		")"
		"@"
		"(?:"
			"("
				"[^:]+"
			")"
			":"
		")?"
		"("
			"[^:]+"
		")"
		":"
		"("
			"[[:digit:]]+"
		")"
	);

	// This is thread safe when BOOST_HAS_THREADS is defined.
	static const boost::regex e(rgx);

	boost::smatch what;
	if (boost::regex_match(input, what, e))
	{
		connection.mConnectedLocalAt = boost::lexical_cast<u32>(what[1]);
		connection.mConnectedExternToGameObject = what[2];
		connection.mConnectedExternToModule = what[3];
		connection.mConnectedExternAt = boost::lexical_cast<u32>(what[4]);
	}
	
	// Check and return if good
	if (connection.good())
		return;

	std::stringstream err;
	err << "parseConnection: Unable to parse \"" << input << "\"!";
	throw std::runtime_error(err.str());
}

} // namespace Loader
} // namespace BFG
