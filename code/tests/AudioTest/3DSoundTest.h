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

#ifndef X3DSOUND_TEST_H_
#define X3DSOUND_TEST_H_

#include <tests/AudioTest/functions.h>


void test3DSound()
{
	using namespace BFG;
	EventLoop* myEventLoop = Audio::AudioMain::eventLoop();

	std::vector<std::string> fileNameList = soundFileNames();

	Audio::Audio Audio;
	dbglog << "Audio created.";

	EventFactory ef;

	ef.Create<Audio::AudioEvent>(myEventLoop, ID::AE_SET_LEVEL_CURRENT, std::string("DummyLevel1"));
	myEventLoop->doLoop();

	AOCreation aoc1
	(
		generateHandle(),
		fileNameList[1]
	);

	int direction = 1;
	int pos = 0;

	ef.Create<Audio::AudioEvent>(myEventLoop, ID::AE_CREATE_AUDIO_OBJECT, aoc1);
	ef.Create<Audio::AudioEvent>(myEventLoop, ID::AE_PLAY, true, aoc1.mHandle, NULL_HANDLE);
	myEventLoop->doLoop();

	for(unsigned int i = 5; i < 75; ++i)
	{
		boost::this_thread::sleep(boost::posix_time::millisec(400));
		
		if (i % 10 == 0)
			direction = direction*(-1);

		pos += direction;

		ef.Create<Audio::AudioEvent>(myEventLoop, ID::AE_POSITION_PLAYER, v3(pos,0,0));
 		ef.Create<Audio::AudioEvent>(myEventLoop, ID::AE_PLAY, true, aoc1.mHandle, NULL_HANDLE);
 		myEventLoop->doLoop();
	}

	block();

}

#endif