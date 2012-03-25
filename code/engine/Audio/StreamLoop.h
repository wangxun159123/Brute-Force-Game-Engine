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

#ifndef AB_STREAM_WATCH
#define AB_STREAM_WATCH

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include <Core/Types.h>

#include <Audio/Defines.h>
#include <Audio/Stream.h>

namespace BFG {
namespace Audio {

class BFG_AUDIO_API StreamLoop
{

public:

	StreamLoop();
	~StreamLoop();

	typedef u32 StreamHandleT;

	//! An audio-object can demand a stream.
	StreamHandleT driveMyStream(boost::shared_ptr<Stream> stream);
	void removeMyStream(StreamHandleT streamHandle);

private:

	boost::thread mThread;
	boost::mutex mMutex; 
	bool mIsRunning;

	void init(const std::vector<std::string>& filelist);
	void onStreaming();

	typedef std::map<StreamHandleT, boost::shared_ptr<Stream> > StreamsOnLoopT;
	StreamHandleT mStreamHandleCounter;

	StreamsOnLoopT mStreamsOnLoop;
};

} // namespace Audio
} // namespace BFG

#endif
