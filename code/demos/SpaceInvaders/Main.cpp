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

#include <OgreException.h>

#include <EventSystem/Core/EventLoop.h>

#include <Core/Utils.h>
#include <Core/Path.h>
#include <Core/ShowException.h>

#include <Controller/Action.h>
#include <Controller/StateInsertion.h>
#include <Controller/Interface.h>

#include <Audio/Interface.h>

#include <Physics/Event.h>
#include <Physics/Interface.h>

#include <Model/Interface.h>

#include <View/Event.h>
#include <View/Interface.h>
#include <View/Main.h>
#include <View/State.h>
#include <View/WindowAttributes.h>

#include "MainState.h"
#include "Globals.h"


// This is the Ex-'GameStateManager::SingleThreadEntryPoint(void*)' function
void* SingleThreadEntryPoint(void *iPointer)
{
	EventLoop* loop = static_cast<EventLoop*>(iPointer);

	GameHandle siHandle = BFG::generateHandle();

	// Hack: Using leaking pointers, because vars would go out of scope
	MainState* ps = new MainState(siHandle, loop);
	ViewMainState* vps = new ViewMainState(siHandle, loop);

	// Init Controller
	GameHandle handle = generateHandle();

	{
		BFG::Controller_::ActionMapT actions;
		actions[A_SHIP_AXIS_Y] = "A_SHIP_AXIS_Y";
		actions[A_SHIP_FIRE]   = "A_SHIP_FIRE";
		actions[A_QUIT]        = "A_QUIT";
		actions[A_FPS]         = "A_FPS";
		BFG::Controller_::sendActionsToController(loop, actions);

		Path path;
		const std::string config_path = path.Expand("SpaceInvaders.xml");
		const std::string state_name = "SpaceInvaders";

		BFG::View::WindowAttributes wa;
		BFG::View::queryWindowAttributes(wa);

		Controller_::StateInsertion si(config_path, state_name, handle, true, wa);

		EventFactory::Create<Controller_::ControlEvent>
		(
			loop,
			ID::CE_LOAD_STATE,
			si
		);

		loop->connect(A_SHIP_AXIS_Y, ps, &MainState::ControllerEventHandler);
		loop->connect(A_SHIP_FIRE, ps, &MainState::ControllerEventHandler);
		loop->connect(A_QUIT, ps, &MainState::ControllerEventHandler);
		loop->connect(A_FPS, ps, &MainState::ControllerEventHandler);
	}

	assert(loop);
	loop->registerLoopEventListener(ps, &MainState::LoopEventHandler);
	return 0;
}

int main( int argc, const char* argv[] ) try
{
	srand(time(NULL));

#if defined(_DEBUG) || !defined(NDEBUG)
	Base::Logger::Init(Base::Logger::SL_DEBUG, "Logs/Si.log");
#else
	Base::Logger::Init(Base::Logger::SL_INFORMATION, "Logs/Si.log");
#endif

	EventLoop iLoop(true);

	size_t controllerFrequency = 1000;

	const std::string caption = "Engine Test 02: Space Invaders";

	// Hack: Actually, EventLoop should take ownership over an entry point
	//       pointer, but it doesn't yet. So here comes the workaround:
	boost::scoped_ptr<Base::IEntryPoint> epView(View::Interface::getEntryPoint(caption));

	iLoop.addEntryPoint(epView.get());
	iLoop.addEntryPoint(ModelInterface::getEntryPoint());
	iLoop.addEntryPoint(ControllerInterface::getEntryPoint(controllerFrequency));
	iLoop.addEntryPoint(Physics::Interface::getEntryPoint());
	iLoop.addEntryPoint(Audio::AudioInterface::getEntryPoint());
	iLoop.addEntryPoint(new Base::CEntryPoint(SingleThreadEntryPoint));

	iLoop.run();
}
catch (Ogre::Exception& e)
{
	showException(e.getFullDescription().c_str());
}
catch (std::exception& ex)
{
	showException(ex.what());
}
catch (...)
{
	showException("Unknown exception");
}
