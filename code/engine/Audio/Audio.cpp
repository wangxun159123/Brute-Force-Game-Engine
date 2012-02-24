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

#include <Audio/Audio.h>

#include <Base/CLogger.h>
#include <EventSystem/Core/EventLoop.h>

#include <Audio/Enums.hh>
#include <Audio/Main.h>
#include <Audio/Transport.h>
#include <Audio/Level.h>
#include <Audio/AudioObject.h>
#include <Audio/Loader.h>
#include <Audio/HelperFunctions.h>


namespace BFG {
namespace Audio {

Audio::Audio():
	mDevice(NULL),
	mContext(NULL)
{
	mCurrentLevel.reset();
	init();
}

Audio::~Audio()
{
	AudioMain::eventLoop()->disconnect(ID::AE_MASTER_GAIN, this);
	AudioMain::eventLoop()->disconnect(ID::AE_POSITION_PLAYER, this);
	AudioMain::eventLoop()->disconnect(ID::AE_CREATE_AUDIO_OBJECT, this);
	AudioMain::eventLoop()->disconnect(ID::AE_ORIENTATION_PLAYER, this);
	AudioMain::eventLoop()->disconnect(ID::AE_VELOCITY_PLAYER, this);
	AudioMain::eventLoop()->disconnect(ID::AE_SET_LEVEL_CURRENT, this);

	mLevels.clear();

	alcDestroyContext(mContext);
	alcCloseDevice(mDevice);

	mContext = NULL;
	mDevice = NULL;

	AudioMain::eventLoop()->setExitFlag(true);
	AudioMain::eventLoop()->doLoop();
}

void Audio::init()
{
	ALenum errorCode = AL_NONE;

	mDevice = alcOpenDevice(0); // 0 is default device.

	if (!mDevice)
	{
		throw std::logic_error("Open audio device failed!");
	}
	else
		infolog << "OpenAl device created";

	alListener3f(AL_POSITION, 0, 0, 0);
	alListener3f(AL_ORIENTATION, 0, 0, 1);
	alListener3f(AL_VELOCITY, 0, 0, 0);

	mContext = alcCreateContext(mDevice, 0);

	if (!mContext)
		throw std::logic_error("Creation of AlContext for level failed! ALerror: "+stringifyAlError(alGetError()));
	else
		infolog << "Al context created";

	if (!alcMakeContextCurrent(mContext))
		throw std::logic_error("Making alContext current failed at Level::load. ALError: "+stringifyAlError(alGetError()));
	else
		infolog << "Set Al context current.";

	AudioMain::eventLoop()->connect(ID::AE_MASTER_GAIN, this, &Audio::eventHandler);
	AudioMain::eventLoop()->connect(ID::AE_POSITION_PLAYER, this, &Audio::eventHandler);
	AudioMain::eventLoop()->connect(ID::AE_CREATE_AUDIO_OBJECT, this, &Audio::eventHandler);
	AudioMain::eventLoop()->connect(ID::AE_ORIENTATION_PLAYER, this, &Audio::eventHandler);
	AudioMain::eventLoop()->connect(ID::AE_VELOCITY_PLAYER, this, &Audio::eventHandler);
	AudioMain::eventLoop()->connect(ID::AE_SET_LEVEL_CURRENT, this, &Audio::eventHandler);

	initLevels();
}

void Audio::initLevels()
{
	DummyLoader loader;
	std::vector<DummyLoader::Level> levels = loader.loadAudioConfig();

	std::vector<DummyLoader::Level>::iterator it = levels.begin();

	for(; it != levels.end(); ++it)
	{
		DummyLoader::Level temp = *it;

		boost::shared_ptr<Level> newLevel;
		newLevel.reset(new Level(temp.second));
		mLevels.insert(std::make_pair(temp.first, newLevel));
	}
}

void Audio::unloadLevel(const std::string& levelName)
{
	LevelMapT::iterator it = mLevels.find(levelName);

	if (it == mLevels.end())
		return;

	mLevels.erase(it);
}

void Audio::setLevelCurrent(const std::string& levelName)
{
	LevelMapT::iterator it = mLevels.find(levelName);

	if (it == mLevels.end())
		throw std::logic_error("Audio: Level: "+ levelName +" doesn't exists!");

	mCurrentLevel = it->second;
	mCurrentLevel->load();
}

void Audio::eventHandler(AudioEvent* AE)
{
	switch (AE->getId())
	{
	case ID::AE_MASTER_GAIN:
		onEventMasterGain(AE->getData());
		break;
	case ID::AE_POSITION_PLAYER:
		onEventPositionPlayer(AE->getData());
		break;
	case ID::AE_ORIENTATION_PLAYER:
		onOrientationPlayer(AE->getData());
		break;
	case ID::AE_VELOCITY_PLAYER:
		onVelocityPlayer(AE->getData());
		break;
	case ID::AE_CREATE_AUDIO_OBJECT:
		onEventCreateAudioObject(AE->getData());
		break;
	case ID::AE_SET_LEVEL_CURRENT:
		onSetLevelCurrent(AE->getData());
		break;
	default:
		throw std::logic_error("Unhandled event!");
	}
}

void Audio::onSetLevelCurrent(const AudioPayloadT& payload)
{
	std::string level = boost::get<std::string>(payload);
	setLevelCurrent(level);
}

void Audio::onVelocityPlayer(const AudioPayloadT& payload)
{
	v3 velocity = boost::get<v3>(payload);
	alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
}

void Audio::onOrientationPlayer(const AudioPayloadT& payload)
{
	v3 orientation = boost::get<v3>(payload);
	alListener3f(AL_ORIENTATION, orientation.x, orientation.y, orientation.z);
}

void Audio::onEventMasterGain(const AudioPayloadT& payload)
{
	ALfloat gain = static_cast<ALfloat>(boost::get<float>(payload));
	alListenerf(AL_GAIN, gain);
}

void Audio::onEventPositionPlayer(const AudioPayloadT& payload)
{
	v3 position = boost::get<v3>(payload);
	alListener3f(AL_POSITION, position.x, position.y, position.z);
}

void Audio::onEventCreateAudioObject(const AudioPayloadT& payload)
{
	AOCreation aoc = boost::get<AOCreation>(payload);

	mCurrentLevel->mObjects[aoc.mHandle] = boost::shared_ptr<AudioObject>
	(
		mCurrentLevel->mObjectFactory->createAudioObject(aoc, mCurrentLevel->mStreamWatch, *this)
	);
}

void Audio::loopEventHandler(LoopEvent* loopEvent)
{
	boost::this_thread::sleep(boost::posix_time::millisec(2));
}

void Audio::deleteAudioObject(GameHandle handle)
{
	Level::ObjectMapT::iterator it;
	it = mCurrentLevel->mObjects.find(handle);

	if (it != mCurrentLevel->mObjects.end())
	{
		mCurrentLevel->mObjects.erase(it);
	}
}

} // namespace Audio
} // namespace BFG
