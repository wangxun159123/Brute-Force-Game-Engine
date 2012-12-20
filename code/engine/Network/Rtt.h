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

#ifndef BFG_NETWORK_RTT_H
#define BFG_NETWORK_RTT_H

#include <list>
#include <numeric>
#include <Core/Types.h>

namespace BFG {
namespace Network {

//! Helper for calculation of round-trip time mean values
template <typename RttT, int Size>
class Rtt
{
public:
	typedef std::list<RttT> RttHistoryContainerT;
	
	Rtt() :
	mMean(0)
	{
	}

	//! Adds a rtt value to the collection
	void add(RttT rtt)
	{
		mRttHistory.push_back(rtt);

		while (mRttHistory.size() > Size)
			mRttHistory.pop_front();

		calculateMean();
	}
	
	RttT last() const
	{
		if (mRttHistory.empty())
			return 0;
		else
			return mRttHistory.back();
	}
	
	
	//! Returns the mean (or average) of all added rtt values in constant time.
	//! \return The mean or 0 if no values have been added yet.
	RttT mean() const
	{
		return mMean;
	}

	//! Returns the whole history
	const RttHistoryContainerT& history() const
	{
		return mRttHistory;
	}

private:
	void calculateMean()
	{
		if (mRttHistory.empty())
			return;
		
		s32 total = std::accumulate(mRttHistory.begin(), mRttHistory.end(), 0);
		mMean = total / mRttHistory.size();
	}

	RttHistoryContainerT mRttHistory;
	RttT mMean;
};

} // namespace Network
} // namespace BFG

#endif // BFG_NETWORK_RTT_H
