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

#include <Audio/StreamLoop.h>

#include <Base/CLogger.h>

namespace BFG {
namespace Audio {

StreamLoop::StreamLoop() : 
	mIsRunning(true),
	mThread(&StreamLoop::onStreaming, this),
	mStreamHandleCounter(0)
{}

StreamLoop::~StreamLoop() 
{
	mIsRunning = false;
	boost::this_thread::sleep(boost::posix_time::milliseconds(500));
	
	mStreamsOnLoop.clear();
}

StreamLoop::StreamHandleT StreamLoop::driveMyStream(boost::shared_ptr<Stream> stream)
{
	boost::mutex::scoped_lock lock(mMutex);
	++mStreamHandleCounter;

	mStreamsOnLoop[mStreamHandleCounter] = stream;
	return mStreamHandleCounter;
}

void StreamLoop::removeMyStream(StreamHandleT streamHandle)
{
	boost::mutex::scoped_lock lock(mMutex);

	StreamsOnLoopT::iterator it;
	it = mStreamsOnLoop.find(streamHandle);
	
	if (it != mStreamsOnLoop.end())
		mStreamsOnLoop.erase(it);
}

void StreamLoop::onStreaming()
{
	while (mIsRunning)
	{
		boost::this_thread::sleep(boost::posix_time::millisec(100));
		boost::mutex::scoped_lock lock(mMutex);
		
		StreamsOnLoopT::iterator it; 

		for(it = mStreamsOnLoop.begin(); it != mStreamsOnLoop.end(); ++it)
		{
			it->second->nextStreamStep();
		}
	}
}

} // namespace Audio
} // namespace BFG
