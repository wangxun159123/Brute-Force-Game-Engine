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

#ifndef STREAM_WATCH_TEST_H_
#define STREAM_WATCH_TEST_H_

#include <tests/AudioTest/functions.h>
#include <Audio/StreamWatch.h>


void streamWatchTest()
{
	using namespace BFG;

	ALCcontext* context = NULL;
	ALCdevice* device = NULL;

	std::vector<std::string> fileNameList = soundFileNames();

	initSound(context, device);

 	boost::shared_ptr<Audio::StreamWatch> streamWatch(new Audio::StreamWatch(fileNameList));

 	ALuint musicSource = streamWatch->demandStream(fileNameList[0]);
    alSourcePlay(musicSource);
 
 	boost::this_thread::sleep(boost::posix_time::seconds(6));

	const size_t num = 100;
	ALuint sourceId;

	for (int i = 0; i < num; ++i)
	{
		boost::this_thread::sleep(boost::posix_time::millisec(200));
		sourceId = streamWatch->demandStream(fileNameList[1]);
		alSourcePlay(sourceId);
		std::cout << i << "\n";
	}

	block();

	alcMakeContextCurrent(NULL); 
	alcDestroyContext(context); 
	alcCloseDevice(device);

	context = NULL;
	device = NULL;
}

#endif