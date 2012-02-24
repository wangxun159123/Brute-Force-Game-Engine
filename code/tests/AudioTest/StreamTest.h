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

#ifndef STREAM_TEST_H_
#define STREAM_TEST_H_

#include <tests/AudioTest/functions.h>
#include <Audio/Audio.h>
#include <Audio/Loader.h>


//! \Hack this won't work in further development!
void printMenu()
{
	using namespace BFG;

	Audio::DummyLoader loader;
	std::vector<Audio::DummyLoader::Level> level =  loader.loadAudioConfig();

	std::cout << level[0].second.size() << ": Exit\n";

	for(int i = 0; i < level[0].second.size(); ++i)
		std::cout << i << ": " << level[0].second[i] << "\n";
}

void streamTest()
{
	using namespace BFG;

	dbglog << "Get EventLoop";
	EventLoop* myEventLoop = Audio::AudioMain::eventLoop();

	std::vector<std::string> fileNameList = soundFileNames();

	dbglog << "Create Audio";
	Audio::Audio Audio;

	EventFactory ef;

	ef.Create<Audio::AudioEvent>(myEventLoop, ID::AE_SET_LEVEL_CURRENT, std::string("DummyLevel1"));
	myEventLoop->doLoop();

	bool noExit = true;

	while(noExit)
	{
		printMenu();

		std::cout << "Choose an audio file: ";
		int choice = -1;
		std::cin >> choice;

		if (choice == -1)
		{
			std::cout << "Not a valid choice. Do it again!\n";
			continue;
		}

		if(choice == fileNameList.size())
		{
			noExit = false;
			continue;
		}

		AOCreation aoc
		(
			generateHandle(),
			fileNameList[choice]
		);

		ef.Create<Audio::AudioEvent>(myEventLoop, ID::AE_CREATE_AUDIO_OBJECT, aoc);
		ef.Create<Audio::AudioEvent>(myEventLoop, ID::AE_PLAY, true, aoc.mHandle, NULL_HANDLE);
		myEventLoop->doLoop();

		choice = -1;

		while(choice != 0)
		{
			std::cout << "Enter '0' to stop music and proceed: ";
			std::cin >> choice;
		}

		ef.Create<Audio::AudioEvent>(myEventLoop, ID::AE_STOP, true, aoc.mHandle, NULL_HANDLE);
		myEventLoop->doLoop();
	}
}


#endif