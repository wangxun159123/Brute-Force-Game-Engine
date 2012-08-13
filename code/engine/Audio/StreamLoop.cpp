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

#include <Base/Logger.h>

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
	mThread.join();

	mStreamsOnLoop.clear();
	mNewStreams.clear();
}

StreamLoop::StreamHandleT StreamLoop::driveMyStream(boost::shared_ptr<Stream> stream)
{
	boost::mutex::scoped_lock lock(mRefreshMutex);
	++mStreamHandleCounter;
	mNewStreams[mStreamHandleCounter] = stream;

	return mStreamHandleCounter;
}

void StreamLoop::removeMyStream(StreamHandleT streamHandle)
{
	boost::mutex::scoped_lock lock(mRefreshMutex);
	mFinishedStreams.push_back(streamHandle);
}

void StreamLoop::removeStream(StreamHandleT streamHandle)
{
	StreamsMapT::iterator it;
	it = mStreamsOnLoop.find(streamHandle);
	
	if (it != mStreamsOnLoop.end())
		mStreamsOnLoop.erase(it);
}

void StreamLoop::onStreaming()
{
	while (mIsRunning)
	{
		// Sleep to save cpu time.
		boost::this_thread::sleep(boost::posix_time::millisec(100));
		boost::mutex::scoped_lock lockForStream(mStreamMutex);
		
		StreamsMapT::iterator it;

		// Process streaming...
		for (it = mStreamsOnLoop.begin(); it != mStreamsOnLoop.end(); ++it)
		{
			it->second->nextStreamStep();
		}

		// A second mutex is needed to avoid a deadlock here.
		// The reason ist that removeMyStream() is called by a
		// chain of calls stated with the callback of Stream (if
		// the stream is finished). So the cat would bite its own
		// tail at this place.
		//
		//  StreamLoop       Stream       AudioObject
		//      |               |              |
		// nextStreamStep()-->  #              |
		//      |               # callback --> #
		//      |               |              #
		//      # <--- removeMyStream()------  #
		//   DEADLOCK           |              |
		//      |               |              |
		//      |               |              |
		//      |               |              |

		boost::mutex::scoped_lock lockForRefresh(mRefreshMutex);


		// Remove every finished stream from the loop.
		while (!mFinishedStreams.empty())
		{
			removeStream(mFinishedStreams.back());
			mFinishedStreams.pop_back();
		}
	
		// Add new streams to the loop.
		for (it = mNewStreams.begin(); it != mNewStreams.end(); ++it)
		{
			mStreamsOnLoop[it->first] = it->second;
		}

		// Must be cleared that streams would not be added multible.
		mNewStreams.clear();
	}
}

} // namespace Audio
} // namespace BFG
