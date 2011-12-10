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

#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/time.hpp>

#include <OgreException.h>

#include <Base/CEntryPoint.h>
#include <Base/CLogger.h>
#include <Controller/Action.h>
#include <Controller/ControllerEvents.h>
#include <Controller/ControllerInterface.h>
#include <Core/ClockUtils.h>
#include <Core/Path.h>
#include <Core/ShowException.h>
#include <Core/Utils.h>
#include <EventSystem/Emitter.h>
#include <View/ControllerMyGuiAdapter.h>
#include <View/Event.h>
#include <View/Interface.h>
#include <View/State.h>

using namespace boost::units;

using BFG::s32;
using BFG::f32;

const s32 A_EXIT = 10000;

struct GameState : BFG::Emitter
{
	GameState(GameHandle handle, EventLoop* loop) :
	Emitter(loop),
	mClock(new BFG::Clock::StopWatch(BFG::Clock::milliSecond)),
	mExitNextTick(false)
	{
		mClock->start();
	}

	void LoopEventHandler(LoopEvent* iLE)
	{
		if (mExitNextTick)
			iLE->getData().getLoop()->setExitFlag();

		long timeSinceLastFrame = mClock->stop();
		if (timeSinceLastFrame)
			mClock->start();

		f32 timeInSeconds = static_cast<f32>(timeSinceLastFrame) / BFG::Clock::milliSecond;
		tick(timeInSeconds);
	}
		
	void tick(const f32 timeSinceLastFrame)
	{
		if (timeSinceLastFrame < BFG::EPSILON_F)
			return;

		quantity<si::time, f32> TSLF = timeSinceLastFrame * si::seconds;
	}

	void ControllerEventHandler(BFG::Controller_::VipEvent* iCE)
	{
		switch(iCE->getId())
		{
			case A_EXIT:
			{
				mExitNextTick = true;
				emit<BFG::View::Event>(BFG::ID::VE_SHUTDOWN, 0);
				break;
			}
		}
	}

	boost::scoped_ptr<BFG::Clock::StopWatch> mClock;
	
	bool mExitNextTick;
};

struct ViewState : public BFG::View::State
{
public:
	ViewState(GameHandle handle, EventLoop* loop) :
	State(handle, loop),
	mControllerMyGuiAdapter(handle, loop)
	{}

	~ViewState()
	{}

	virtual void pause()
	{}

	virtual void resume()
	{}
	
private:
	BFG::View::ControllerMyGuiAdapter mControllerMyGuiAdapter;
};

void initController(BFG::Emitter& emitter, BFG::GameHandle stateHandle)
{
	BFG::Controller_::ActionMapT actions;
	actions[A_EXIT] = "A_EXIT";
	BFG::Controller_::fillWithDefaultActions(actions);
	BFG::Controller_::sendActionsToController(emitter.loop(), actions);

	BFG::Path path;
	const std::string configPath = path.Expand("TutorialBasics.xml");
	const std::string stateName = "TutorialBasics";
	
	BFG::Controller_::StateInsertion si(configPath, stateName, stateHandle, true);

	emitter.emit<BFG::Controller_::ControlEvent>
	(
		BFG::ID::CE_LOAD_STATE,
		si
	);
}

void* SingleThreadEntryPoint(void *iPointer)
{
	EventLoop* loop = static_cast<EventLoop*>(iPointer);
	BFG::Emitter emitter(loop);
	
	GameHandle stateHandle = BFG::generateHandle();

	// Hack: Using leaking pointers, because vars would go out of scope
	GameState* gs = new GameState(stateHandle, loop);
	ViewState* vps = new ViewState(stateHandle, loop);

	initController(emitter, stateHandle);
	
	loop->connect(A_EXIT, gs, &GameState::ControllerEventHandler);
	loop->registerLoopEventListener(gs, &GameState::LoopEventHandler);	
	return 0;
}

int main( int argc, const char* argv[] ) try
{
	BFG::Base::Logger::Init(BFG::Base::Logger::SL_DEBUG, "Logs/TutorialBasics.log");

	EventLoop iLoop(true);

	const std::string caption = "Tutorial 01: Basics";
	size_t controllerFrequency = 1000;

	boost::scoped_ptr<BFG::Base::IEntryPoint> epView(BFG::View::Interface::getEntryPoint(caption));

	iLoop.addEntryPoint(epView.get());
	iLoop.addEntryPoint(BFG::ControllerInterface::getEntryPoint(controllerFrequency));
	iLoop.addEntryPoint(new BFG::Base::CEntryPoint(SingleThreadEntryPoint));

	iLoop.run();
}
catch (Ogre::Exception& e)
{
	BFG::showException(e.getFullDescription().c_str());
}
catch (std::exception& ex)
{
	BFG::showException(ex.what());
}
catch (...)
{
	BFG::showException("Unknown exception");
}
