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

#ifndef _BFG_BASE_COMPILE_DATE_TIME_H_
#define _BFG_BASE_COMPILE_DATE_TIME_H_

#include <sstream>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

namespace BFG {

// From http://stackoverflow.com/questions/3854496
boost::posix_time::time_duration utcOffset()
{
	using namespace boost::posix_time;

	// boost::date_time::c_local_adjustor uses the C-API to adjust a
	// moment given in utc to the same moment in the local time zone.
	typedef boost::date_time::c_local_adjustor<ptime> local_adj;

	const ptime utc_now = second_clock::universal_time();
	const ptime now = local_adj::utc_to_local(utc_now);

	return now - utc_now;
}


//! \brief Merges __DATE__ and __TIME__ into one ptime.
//!
//! __DATE__ and __TIME__ are macros which get substituted with the compile
//! date and the compile time of the respective compile unit.
//!
//! Make sure to either clean and rebuild your project to avoid getting
//! cached values. CCache correctly recognizes the use of these macros.
//!
//! Also keep in mind that these values will probably differ between
//! multiple compile units.
//!
//! For above mentioned reasons this code should stay inline.
//!
inline boost::posix_time::ptime compileDateTime()
{
	std::istringstream ss;

	// __DATE__ to date
	boost::gregorian::date_input_facet *dif = new boost::gregorian::date_input_facet("%b %e %Y"); 

	ss.str(__DATE__);
	ss.imbue(std::locale(ss.getloc(), dif));

	boost::gregorian::date d; 
	ss >> d;
	
	// __TIME__ to ptime
	boost::posix_time::time_input_facet* tif = new boost::posix_time::time_input_facet("%H:%M:%S");

	ss.str(__TIME__);
	ss.imbue(std::locale(ss.getloc(), tif));

	boost::posix_time::ptime p;
	ss >> p;

	// Merge both into one ptime
	boost::posix_time::ptime full(d, p.time_of_day());

	// Convert to UTC
	full -= utcOffset();

	return full;
}

} // namespace BFG

#endif

