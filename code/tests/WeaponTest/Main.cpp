/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2013 Brute-Force Games GbR

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

// BFG libraries
#include <Controller/Action.h>
#include <Controller/ControllerEvents.h>
#include <Core/Path.h>
#include <Core/ShowException.h>
#include <Core/Utils.h>
#include <View/ControllerMyGuiAdapter.h>
#include <View/Event.h>
#include <View/Interface.h>

#define BFG_USE_CONTROLLER
#define BFG_USE_PHYSICS
#define BFG_USE_VIEW
#include <EngineInit.h>

using namespace BFG;

const s32 A_EXIT = 10000;
const s32 A_CONSOLE = 10001;

void waitingForWindow(View::WindowAttributes& wa)
{
	bool waiting = true;
	u32 timeWaited = 0;
	do 
	{
		try
		{
			View::queryWindowAttributes(wa);
			waiting = false;
		}
		catch (std::exception&)
		{
			if (timeWaited < 5000)
			{
				warnlog << "Waiting for Window creation";
				boost::this_thread::sleep(boost::posix_time::milliseconds(200));
				timeWaited += 200;
			}
			else
			{
				errlog << "Window creation took too long";
				return;
			}
		}
	} while (waiting);
}

struct WeaponTest : Emitter
{
	WeaponTest(EventLoop* loop) :
	Emitter(loop),
	mControllerAdapter(generateHandle(), loop)
	{
		loop->connect(A_EXIT, this, &WeaponTest::controllerEventHandler);
		loop->connect(A_CONSOLE, this, &WeaponTest::controllerEventHandler);

		Controller_::ActionMapT actions;
		actions[A_EXIT] = "A_EXIT";
		actions[A_CONSOLE] = "A_CONSOLE";
		Controller_::fillWithDefaultActions(actions);
		Controller_::sendActionsToController(loop, actions);
	
		Path path;
		const std::string config_path = path.Expand("WeaponTest.xml");
		const std::string state_name = "WeaponTest";

		View::WindowAttributes wa;

		waitingForWindow(wa);

		Controller_::StateInsertion si(config_path, state_name, generateHandle(), true, wa);

		EventFactory::Create<Controller_::ControlEvent>
		(
			loop,
			ID::CE_LOAD_STATE,
			si
		);
	}

	~WeaponTest()
	{
		loop()->disconnect(A_EXIT, this);
		loop()->disconnect(A_CONSOLE, this);
	}

	void controllerEventHandler(BFG::Controller_::VipEvent* e)
	{
		switch(e->getId())
		{
		case A_EXIT:
			onExit();
			break;
		case A_CONSOLE:
		{
			emit<BFG::View::Event>(BFG::ID::VE_CONSOLE, boost::get<bool>(e->getData()));
			break;
		}
		}
	}

	void onExit()
	{
		loop()->stop();
	}

private:
	View::ControllerMyGuiAdapter mControllerAdapter;

};

void WeaponTestInitHandler(EventLoop& loop)
{
	WeaponTest wt(&loop);
	boost::this_thread::sleep(boost::posix_time::milliseconds(500));
	while(!loop.shouldExit())
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(300));
	}
	loop.stop();
}

int main(int argc, const char* argv[]) try
{
	BFG::Configuration cfg("bfgWeaponTest");
	cfg.handler = boost::bind(&WeaponTestInitHandler, _1);

	BFG::engineInit(cfg);

	// Give EventSystem some time to stop all loops
	boost::this_thread::sleep(boost::posix_time::milliseconds(500));
	dbglog << "Good bye!";
}
catch (std::exception& ex)
{
	showException(ex.what());
}
catch (...)
{
	showException("Unknown exception");
}
