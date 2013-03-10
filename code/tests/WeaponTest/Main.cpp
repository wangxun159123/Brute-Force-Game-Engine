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

#include <boost/foreach.hpp>
// BFG libraries
#include <Controller/Action.h>
#include <Controller/ControllerEvents.h>
#include <Core/Path.h>
#include <Core/ShowException.h>
#include <Core/Utils.h>
#include <Model/Data/GameObjectFactory.h>
#include <Model/Data/LevelConfig.h>
#include <Model/Data/ObjectParameters.h>
#include <Model/Environment.h>
#include <Model/Property/Concepts/Camera.h>
#include <Model/Property/SpacePlugin.h>
#include <Model/Sector.h>
#include <Model/State.h>
#include <View/ControllerMyGuiAdapter.h>
#include <View/Event.h>
#include <View/Interface.h>
#include <View/State.h>

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

struct WeaponTest : State
{
	WeaponTest(EventLoop* loop, GameHandle handle) :
	State(loop),
	mHandle(handle)
	{
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

		registerEventHandler();
		createScene();
	}

	~WeaponTest()
	{
		unregisterEventHandler();
	}

	void registerEventHandler()
	{
		loop()->connect(A_EXIT, this, &WeaponTest::controllerEventHandler);
		loop()->connect(A_CONSOLE, this, &WeaponTest::controllerEventHandler);
	}

	void unregisterEventHandler()
	{
		loop()->disconnect(A_EXIT, this);
		loop()->disconnect(A_CONSOLE, this);
	}

	void createScene()
	{
		Path p;

		std::string level = p.Get(ID::P_SCRIPTS_LEVELS) + "WeaponTest/";
		std::string def = p.Get(ID::P_SCRIPTS_LEVELS) + "default/";

		LevelConfig lc;

		lc.mModules.push_back(def + "Object.xml");
		lc.mAdapters.push_back(def + "Adapter.xml");
		lc.mConcepts.push_back(def + "Concept.xml");
		lc.mProperties.push_back(def + "Value.xml");

		lc.mModules.push_back(level + "Object.xml");
		lc.mAdapters.push_back(level + "Adapter.xml");
		lc.mConcepts.push_back(level + "Concept.xml");
		lc.mProperties.push_back(level + "Value.xml");

		mEnvironment.reset(new Environment);

		using Property::ValueId;
		PluginId spId = ValueId::ENGINE_PLUGIN_ID;
		PluginId rpId = Property::generatePluginId<PluginId>();

		boost::shared_ptr<SpacePlugin> sp(new SpacePlugin(spId));

		mPluginMap.insert(sp);

		mGameObjectFactory.reset(new GameObjectFactory(loop(), lc, mPluginMap, mEnvironment, mHandle));
		mSector.reset(new Sector(loop(), mHandle, "WeaponTest", mGameObjectFactory));
		
		View::SkyCreation sc("sky02");
		emit<View::Event>(ID::VE_SET_SKY, sc, mHandle);

		View::LightParameters lightParameters;
		lightParameters.mName = "MainLight";
		lightParameters.mDiffuseColor = cv4::White;
		lightParameters.mType = ID::LT_Directional;
		lightParameters.mDirection = v3(24.594410f, -123.637207f, 745.162537f);
		lightParameters.mHandle = generateHandle();

		emit<View::Event>(ID::VE_CREATE_LIGHT, lightParameters, mHandle);
		emit<View::Event>(ID::VE_SET_AMBIENT, cv4(0.1f, 0.1f, 0.1f), mHandle);

		ObjectParameter op;
		op.mType = "Weapon Single";
		op.mHandle = BFG::generateNetworkHandle();
		op.mName = "TestWeapon";
		op.mLocation.position = v3(0.0f, 0.0f, 0.0f);
		op.mLocation.orientation = qv4::IDENTITY;

		mSector->addObject
		(
			mGameObjectFactory->createGameObject(op)
		);

		CameraParameter cameraParameter;
		cameraParameter.mMode = ID::CM_Fixed;
		cameraParameter.mParentObject = op.mName;
		cameraParameter.mOffset = v3(2.0f, 2.0f, -7.0f);
		cameraParameter.mFullscreen = true;

		mSector->addObject
		(
			mGameObjectFactory->createCamera(cameraParameter, cameraParameter.mParentObject)
		);
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

	void onTick(const boost::units::quantity<boost::units::si::time, f32> TSLF)
	{
		mSector->update(TSLF);
	}

private:
	GameHandle mHandle;
	boost::shared_ptr<Environment> mEnvironment;
	boost::shared_ptr<GameObjectFactory> mGameObjectFactory;
	boost::shared_ptr<Sector> mSector;
	Property::PluginMapT mPluginMap;

};

struct ViewState : public View::State
{
public:
	ViewState(GameHandle handle, EventLoop* loop) :
	State(handle, loop),
	mControllerMyGuiAdapter(handle, loop)
	{
		emit<BFG::View::Event>(BFG::ID::VE_SET_AMBIENT, BFG::cv4(1.0f, 1.0f, 1.0f), handle);
	}

	~ViewState()
	{
		infolog << "SynchronizationTest: Destroying ViewState.";
		emit<BFG::View::Event>(BFG::ID::VE_SHUTDOWN, 0);
	}

	virtual void pause()
	{}

	virtual void resume()
	{}

private:
	BFG::View::ControllerMyGuiAdapter mControllerMyGuiAdapter;
};

void WeaponTestInitHandler(EventLoop& loop)
{
	GameHandle handle = generateHandle();
	WeaponTest wt(&loop, handle);
	ViewState vs(handle, &loop);
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
