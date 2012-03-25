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

/**
	@file

	This example application demonstrates how to create a very simple
	render window which reacts on input from Keyboard and Mouse.
*/

// We use Boost.Units for typesafe calculations, that is compile time checks
// for formulas.
#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/time.hpp>

// OGRE
#include <OgreException.h>

// BFG libraries
#include <Base/CEntryPoint.h>
#include <Base/CLogger.h>
#include <Controller/Action.h>
#include <Controller/ControllerEvents.h>
#include <Controller/Interface.h>
#include <Core/ClockUtils.h>
#include <Core/Path.h>
#include <Core/ShowException.h>
#include <Core/Utils.h>
#include <EventSystem/Emitter.h>
#include <View/ControllerMyGuiAdapter.h>
#include <View/Event.h>
#include <View/Interface.h>
#include <View/State.h>
#include <View/WindowAttributes.h>

using namespace boost::units;

using BFG::s32;
using BFG::f32;

// Client applications should use event IDs higher than 10000 to avoid
// collisions with events used within the engine.
const s32 A_EXIT = 10000;

// Here comes our first state. Most of the time we use it as Owner of objects
// or as forwarder of input (Controller) events. 
struct GameState : BFG::Emitter
{
	GameState(GameHandle handle, EventLoop* loop) :
	Emitter(loop),
	mClock(new BFG::Clock::StopWatch(BFG::Clock::milliSecond)),
	mExitNextTick(false)
	{
		mClock->start();
	}

	// Most games have a game loop, in which the most important modules
	// have an update function. The following is such an update function,
	// but it's not called by the game loop - the event system takes care
	// of it by simply sending a LoopEvent at the right time.
	//
	// So what we're doing here is emulating OGRE's timeSinceLastFrame
	// variable which is a delta to the previous absolute time. Also, we
	// check if we decided to end the update process of this module, and
	// notice the event system if true.
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

	// The actual update function. You may update objects and other things
	// here.
	void tick(const f32 timeSinceLastFrame)
	{
		// Ignore too small time deltas. This may cause crashes on very
		// fast systems if we don't.
		// TODO: It may be that this check is now redundant due to the
		// cast to long in LoopEventHandler.
		if (timeSinceLastFrame < BFG::EPSILON_F)
			return;

		// Redundant in this case, but useful later, so let's introduce
		// it now. A quantity is a wrapper around T, which adds
		// dimensions, like time, mass and so on.
		quantity<si::time, f32> TSLF = timeSinceLastFrame * si::seconds;
	}

	// Callback for Input. The Controller sends input directly to this
	// state, since we told him so (below).
	void ControllerEventHandler(BFG::Controller_::VipEvent* iCE)
	{
		switch(iCE->getId())
		{
			// This is the event ID we specified at the top
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

// We won't display anything, so this class remains more or less empty. In this
// engine, Model and View are separated, so as you guessed this is the same as
// the GameState, but this time for render stuff.
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

// Initializing input handling here.
void initController(BFG::Emitter& emitter, BFG::GameHandle stateHandle)
{
	// At the beginning, the Controller is "empty" and must be filled with
	// states and actions. A Controller state corresponds to a Model state
	// or a View state and in fact, they must have the same handle
	// (GameHandle).
	// This part here is necessary for Action deserialization.
	BFG::Controller_::ActionMapT actions;
	actions[A_EXIT] = "A_EXIT";
	BFG::Controller_::fillWithDefaultActions(actions);
	BFG::Controller_::sendActionsToController(emitter.loop(), actions);

	// Actions must be configured by XML
	BFG::Path path;
	const std::string configPath = path.Expand("TutorialBasics.xml");
	const std::string stateName = "TutorialBasics";

	// The Controller must know about the size of the window for the mouse
	size_t handle; BFG::u32 width; BFG::u32 height;
	BFG::View::windowAttributes(handle, width, height);
	
	// Finally, send everything to the Controller
	BFG::Controller_::StateInsertion si(configPath, stateName, stateHandle, true, width, height, handle);
	emitter.emit<BFG::Controller_::ControlEvent>
	(
		BFG::ID::CE_LOAD_STATE,
		si
	);
}

// Just a callback for initialization
void* SingleThreadEntryPoint(void *iPointer)
{
	EventLoop* loop = static_cast<EventLoop*>(iPointer);
	BFG::Emitter emitter(loop);
	
	// Create the states.
	// Hack: Using leaking pointers, because vars would go out of scope
	GameHandle stateHandle = BFG::generateHandle();
	GameState* gs = new GameState(stateHandle, loop);
	ViewState* vps = new ViewState(stateHandle, loop);

	initController(emitter, stateHandle);
	
	// This part is quite important. You must connect your event callbacks.
	// If not, the event system doesn't know you're waiting for them.
	loop->connect(A_EXIT, gs, &GameState::ControllerEventHandler);

	// Setting up callbacks for the game loop, only those which must be
	// updated regularily by a LoopEvent
	loop->registerLoopEventListener(gs, &GameState::LoopEventHandler);	
	return 0;
}

int main( int argc, const char* argv[] ) try
{
	// Our logger. Not used here, works like cout, but without the need for
	// endl and with multiple severities: dbglog, infolog, warnlog, errlog.
	BFG::Base::Logger::Init(BFG::Base::Logger::SL_DEBUG, "Logs/TutorialBasics.log");
	infolog << "This is our logger!";

	EventLoop iLoop(true);

	const std::string caption = "Tutorial 01: Basics";
	size_t controllerFrequency = 1000;

	boost::scoped_ptr<BFG::Base::IEntryPoint> epView(BFG::View::Interface::getEntryPoint(caption));

	// Setting up callbacks for module initialization
	// This is still very inconsistent but a proof for flexibility ;)
	iLoop.addEntryPoint(epView.get());
	iLoop.addEntryPoint(BFG::ControllerInterface::getEntryPoint(controllerFrequency));
	iLoop.addEntryPoint(new BFG::Base::CEntryPoint(SingleThreadEntryPoint));

	iLoop.run();
}
catch (Ogre::Exception& e)
{
	// showException shows the exception. On Windows you'll get a MessageBox.
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
