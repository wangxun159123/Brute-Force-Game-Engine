/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2012 Brute-Force Games GbR

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

#ifndef AUDIO_STREAM_LOOP
#define AUDIO_STREAM_LOOP

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include <Core/Types.h>

#include <Audio/Defines.h>
#include <Audio/Stream.h>

namespace BFG {
namespace Audio {

//! This class is the owner of the stream thread. This class
//! does not stream the data by itsself. It just provides an
//! interface to register and unregister streams to the
//! loop (the thread).
class BFG_AUDIO_API StreamLoop
{

public:

	StreamLoop();
	~StreamLoop();

	typedef u32 StreamHandleT;

	//! Registers a Stream to the loop.
	StreamHandleT driveMyStream(boost::shared_ptr<Stream> stream);

	//! Unregisters a Stream from the loop.
	void removeMyStream(StreamHandleT streamHandle);

private:
	bool mIsRunning;
	
	boost::thread mThread;
	boost::mutex mStreamMutex;
	boost::mutex mRefreshMutex;
	
	void init(const std::vector<std::string>& filelist);
	void onStreaming();
	void removeStream(StreamHandleT streamHandle);

	typedef std::map<StreamHandleT, boost::shared_ptr<Stream> > StreamsMapT;
	typedef std::vector<StreamHandleT> FinishedStreamsT;
	
	StreamHandleT mStreamHandleCounter;

	StreamsMapT mStreamsOnLoop;
	FinishedStreamsT mFinishedStreams;
	StreamsMapT mNewStreams;
};

} // namespace Audio
} // namespace BFG

#endif
