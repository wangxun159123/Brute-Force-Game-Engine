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

#include <boost/foreach.hpp>

// OGRE
#include <OgreException.h>

// BFG libraries
#include <Base/EntryPoint.h>
#include <Base/Logger.h>
#include <Base/Pause.h>
#include <Controller/Action.h>
#include <Controller/ControllerEvents.h>
#include <Controller/Interface.h>
#include <Core/Path.h>
#include <Core/ShowException.h>
#include <Core/Utils.h>
#include <Model/Environment.h>
#include <Model/GameObject.h>
#include <Model/Loader/GameObjectFactory.h>
#include <Model/Loader/Interpreter.h>
#include <Model/Sector.h>
#include <Model/Loader/Types.h>
#include <Model/Interface.h>
#include <Model/Property/Concept.h>
#include <Model/Property/Plugin.h>
#include <Model/Property/SpacePlugin.h>
#include <Model/State.h>
#include <Network/Event.h>
#include <Network/Interface.h>
#include <Physics/Event_fwd.h>
#include <Physics/Interface.h>
#include <View/ControllerMyGuiAdapter.h>
#include <View/Event.h>
#include <View/Interface.h>
#include <View/State.h>
#include <View/WindowAttributes.h>

// We use Boost.Units for typesafe calculations - which are
// essentially compile time checks for formulas.
using namespace boost::units;

using BFG::s32;
using BFG::f32;

// Client applications should use event IDs higher than 10000 to avoid
// collisions with events used within the engine.
const s32 A_EXIT = 10000;
const s32 SIMULATION_0 = 10001;
const s32 SIMULATION_1 = 10002;
const s32 SIMULATION_2 = 10003;
const s32 SIMULATION_3 = 10004;
const s32 CREATE_TEST_OBJECT = 15000;
const s32 START_SIMULATION_0 = 15001;
const s32 START_SIMULATION_1 = 15002;
const s32 START_SIMULATION_2 = 15003;
const s32 START_SIMULATION_3 = 15004;

const GameHandle SERVER_STATE_HANDLE = 42;
const GameHandle CLIENT_STATE_HANDLE = 43;

// Here comes our first state. Most of the time we use it as Owner of objects
// or as forwarder of input (Controller) events. I.e. a state could be the
// "Main Menu", a "Movie Sequence" or the 3D part of the application.

struct SynchronizationTestState: BFG::State
{
	SynchronizationTestState(GameHandle handle, EventLoop* loop) :
	State(loop),
	mStateHandle(handle),
	mPlayer(NULL_HANDLE),
	mEnvironment(new BFG::Environment)
{
	// create cube
	BFG::Path p;
	std::string def = p.Get(BFG::ID::P_SCRIPTS_LEVELS) + "default/";

	BFG::Loader::LevelConfig lc;

	lc.mModules.push_back(def + "Object.xml");
	lc.mAdapters.push_back(def + "Adapter.xml");
	lc.mConcepts.push_back(def + "Concept.xml");
	lc.mProperties.push_back(def + "Value.xml");

	using BFG::Property::ValueId;

	BFG::PluginId spId = ValueId::ENGINE_PLUGIN_ID;
	boost::shared_ptr<BFG::SpacePlugin> sp(new BFG::SpacePlugin(spId));
	mPluginMap.insert(sp);

	boost::shared_ptr<BFG::Loader::Interpreter> interpreter(new BFG::Loader::Interpreter(mPluginMap));

	mGof.reset(new BFG::Loader::GameObjectFactory(this->loop(), lc, mPluginMap, interpreter, mEnvironment, mStateHandle));

	mSector.reset(new BFG::Sector(this->loop(), 1, "Blah", mGof));
}

virtual ~SynchronizationTestState()
{
}

// You may update objects and other things here.
virtual void onTick(const quantity<si::time, f32> TSLF)
{
	mSector->update(TSLF);

	emit<BFG::Physics::Event>(BFG::ID::PE_STEP, TSLF.value());
}

virtual void createObject(const BFG::Loader::ObjectParameter& param)
{
	boost::shared_ptr<BFG::GameObject> playerShip = mGof->createGameObject(param);
	mSector->addObject(playerShip);

	mPlayer = playerShip->getHandle();
}

protected:
	GameHandle mStateHandle;
	GameHandle mPlayer;

private:
	BFG::Property::PluginMapT mPluginMap;
	boost::shared_ptr<BFG::Sector> mSector;
	boost::shared_ptr<BFG::Environment> mEnvironment;
	boost::shared_ptr<BFG::Loader::GameObjectFactory> mGof;

};

struct ServerState: public SynchronizationTestState
{
	ServerState(GameHandle handle, EventLoop* loop) :
	SynchronizationTestState(handle, loop)
	{
		loop->connect(BFG::ID::NE_RECEIVED, this, &ServerState::networkPacketEventHandler, SERVER_STATE_HANDLE);
		loop->connect(BFG::ID::NE_CONNECTED, this, &ServerState::networkControlEventHandler);
	}
	
	virtual ~ServerState()
	{
		loop()->disconnect(BFG::ID::NE_RECEIVED, this);
		loop()->disconnect(BFG::ID::NE_CONNECTED, this);
	}

// 	// You may update objects and other things here.
// 	virtual void onTick(const quantity<si::time, f32> TSLF)
// 	{
// 
// 	}
	void networkPacketEventHandler(BFG::Network::DataPacketEvent* e)
	{
		switch(e->getId())
		{
		case BFG::ID::NE_RECEIVED:
		{
			const BFG::Network::DataPayload& payload = e->getData();

			switch(payload.mAppEventId)
			{
			case START_SIMULATION_0:
			{
				infolog << "Starting Simulation 0";

				emit<BFG::Physics::Event>(BFG::ID::PE_UPDATE_VELOCITY, v3::ZERO, mObject1);
				emit<BFG::Physics::Event>(BFG::ID::PE_UPDATE_VELOCITY, v3::ZERO, mObject2);
				emit<BFG::Physics::Event>(BFG::ID::PE_UPDATE_ROTATION_VELOCITY, v3::ZERO, mObject1);
				emit<BFG::Physics::Event>(BFG::ID::PE_UPDATE_ROTATION_VELOCITY, v3::ZERO, mObject2);

				break;
			}
			case START_SIMULATION_1:
			{
				infolog << "Starting Simulation 1";
				v3 position = v3(2.0f, 0.0f, 50.0f);
				emit<BFG::Physics::Event>(BFG::ID::PE_UPDATE_POSITION, position, mPlayer);

				break;
			}
			case START_SIMULATION_2:
			{
				infolog << "Starting Simulation 2";
				v3 force = v3(-100000.0f, 0.0f, 0.0f);
				emit<BFG::Physics::Event>(BFG::ID::PE_APPLY_FORCE, force, mPlayer);

				break;
			}
			case START_SIMULATION_3:
			{
				infolog << "Starting Simulation 3";
				v3 torque = v3(5000.0f, 0.0f, 0.0f); // spin around the x-axis
				emit<BFG::Physics::Event>(BFG::ID::PE_APPLY_TORQUE, torque, mPlayer);

				break;
			}
			}

		}
		}
	}

	void networkControlEventHandler(BFG::Network::ControlEvent* e)
	{
		switch(e->getId())
		{
		case BFG::ID::NE_CONNECTED:
		{
			std::stringstream handles;

			// First cube
			BFG::Loader::ObjectParameter op;
			op.mHandle = BFG::generateNetworkHandle();
			op.mName = "TestCube1";
			op.mType = "Cube";
			op.mLocation = v3(0.0f, -1.0f, 50.0f);

			handles << op.mHandle << " ";

			createObject(op);
			mObject1 = op.mHandle;

			emit<BFG::GameObjectEvent>(BFG::ID::GOE_SYNCHRONIZATION_MODE, (s32)BFG::ID::SYNC_MODE_NETWORK_WRITE, op.mHandle);

			op.mHandle = BFG::generateNetworkHandle();
			op.mName = "TestCube2";
			op.mLocation = v3(0.0f, 1.0f, 50.0f);

			handles << op.mHandle;

			createObject(op);
			mObject2 = op.mHandle;
				
			emit<BFG::GameObjectEvent>(BFG::ID::GOE_SYNCHRONIZATION_MODE, (s32)BFG::ID::SYNC_MODE_NETWORK_WRITE, op.mHandle);

			CharArray512T ca512 = stringToArray<512>(handles.str());

			BFG::Network::DataPayload payload
			(
				CREATE_TEST_OBJECT, 
				CLIENT_STATE_HANDLE, 
				SERVER_STATE_HANDLE,
				handles.str().length(),
				ca512
			);

			emit<BFG::Network::DataPacketEvent>(BFG::ID::NE_SEND, payload);

			break;
		}
		}

	}

private:
	std::vector<GameHandle> mClientList;

	GameHandle mObject1;
	GameHandle mObject2;
};

struct ClientState : public SynchronizationTestState
{
	ClientState(GameHandle handle, EventLoop* loop) :
	SynchronizationTestState(handle, loop)
	{
		// This part is quite important. You must connect your event callbacks.
		// If not, the event system doesn't know you're waiting for them.
		loop->connect(A_EXIT, this, &ClientState::controllerEventHandler);
		loop->connect(SIMULATION_0, this, &ClientState::controllerEventHandler);
		loop->connect(SIMULATION_1, this, &ClientState::controllerEventHandler);
		loop->connect(SIMULATION_2, this, &ClientState::controllerEventHandler);
		loop->connect(SIMULATION_3, this, &ClientState::controllerEventHandler);
		loop->connect(BFG::ID::NE_RECEIVED, this, &ClientState::networkEventHandler, CLIENT_STATE_HANDLE);
	}

	virtual ~ClientState()
	{
		infolog << "Tutorial: Destroying GameState.";
		loop()->disconnect(A_EXIT, this);
		loop()->disconnect(SIMULATION_0, this);
		loop()->disconnect(SIMULATION_1, this);
		loop()->disconnect(SIMULATION_2, this);
		loop()->disconnect(SIMULATION_3, this);
		loop()->disconnect(BFG::ID::NE_RECEIVED, this);
	}

// 	// You may update objects and other things here.
// 	virtual void onTick(const quantity<si::time, f32> TSLF)
// 	{
// 
// 	}

	void onExit()
	{
		// Calling this will hold the update process of this State.
		// No further events might be received after this.
		loop()->stop();
	}

	// Callback for Input. The Controller sends input directly to this
	// state, since we told him so (in `initController').
	void controllerEventHandler(BFG::Controller_::VipEvent* e)
	{
		switch(e->getId())
		{
			// This is the event ID we specified at the top
		case A_EXIT:
		{
			onExit();
			break;
		}
		case SIMULATION_0:
		{
			CharArray512T ca512 = CharArray512T();
			BFG::Network::DataPayload payload
			(
				START_SIMULATION_0, 
				SERVER_STATE_HANDLE, 
				CLIENT_STATE_HANDLE,
				0,
				ca512
			);

			emit<BFG::Network::DataPacketEvent>(BFG::ID::NE_SEND, payload);

			break;
		}
		case SIMULATION_1:
		{
			CharArray512T ca512 = CharArray512T();
			BFG::Network::DataPayload payload
			(
				START_SIMULATION_1, 
				SERVER_STATE_HANDLE, 
				CLIENT_STATE_HANDLE,
				0,
				ca512
			);

			emit<BFG::Network::DataPacketEvent>(BFG::ID::NE_SEND, payload);

			break;
		}
		case SIMULATION_2:
		{
			CharArray512T ca512 = CharArray512T();
			BFG::Network::DataPayload payload
			(
				START_SIMULATION_2, 
				SERVER_STATE_HANDLE, 
				CLIENT_STATE_HANDLE,
				0,
				ca512
			);

			emit<BFG::Network::DataPacketEvent>(BFG::ID::NE_SEND, payload);

			break;
		}
		case SIMULATION_3:
		{
			CharArray512T ca512 = CharArray512T();
			BFG::Network::DataPayload payload
			(
				START_SIMULATION_3, 
				SERVER_STATE_HANDLE, 
				CLIENT_STATE_HANDLE,
				0,
				ca512
			);

			emit<BFG::Network::DataPacketEvent>(BFG::ID::NE_SEND, payload);

			break;
		}
		}
	}

	void networkEventHandler(BFG::Network::DataPacketEvent* e)
	{
		switch(e->getId())
		{
		case BFG::ID::NE_RECEIVED:
		{
			const BFG::Network::DataPayload& payload = e->getData();

			switch(payload.mAppEventId)
			{
			case CREATE_TEST_OBJECT:
			{
				std::stringstream oss(payload.mAppData.data());

				BFG::Loader::ObjectParameter op;
				op.mType = "Cube_Remote";

				oss >> op.mHandle;
				op.mName = "TestCube";
				op.mLocation = v3(0.0f, -1.0f, 50.0f);
				createObject(op);
				emit<BFG::GameObjectEvent>(BFG::ID::GOE_SYNCHRONIZATION_MODE, (s32)BFG::ID::SYNC_MODE_NETWORK_READ, op.mHandle);

				oss >> op.mHandle;
				op.mName = "TestCube2";
				op.mLocation = v3(0.0f, 1.0f, 50.0f);
				createObject(op);
				emit<BFG::GameObjectEvent>(BFG::ID::GOE_SYNCHRONIZATION_MODE, (s32)BFG::ID::SYNC_MODE_NETWORK_READ, op.mHandle);

				break;
			}
			}
		}
		}
	}
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
	{
		emit<BFG::View::Event>(BFG::ID::VE_SET_AMBIENT, BFG::cv4(1.0f, 1.0f, 1.0f), handle);
	}

	~ViewState()
	{
		infolog << "Tutorial: Destroying ViewState.";

		// The View module must be shut down manually.
		emit<BFG::View::Event>(BFG::ID::VE_SHUTDOWN, 0);
	}

	virtual void pause()
	{}

	virtual void resume()
	{}
	
private:
	BFG::View::ControllerMyGuiAdapter mControllerMyGuiAdapter;
};

// Initializing input handling here.
void initController(BFG::GameHandle stateHandle, EventLoop* loop)
{
	// The Emitter is the standard tool to send events with.
	BFG::Emitter emitter(loop);

	// At the beginning, the Controller is "empty" and must be filled with
	// states and actions. A Controller state corresponds to a Model state
	// or a View state and in fact, they must have the same handle
	// (GameHandle).
	// This part here is necessary for Action deserialization.
	BFG::Controller_::ActionMapT actions;
	actions[A_EXIT] = "A_EXIT";
	actions[SIMULATION_0] = "SIMULATION_0";
	actions[SIMULATION_1] = "SIMULATION_1";
	actions[SIMULATION_2] = "SIMULATION_2";
	actions[SIMULATION_3] = "SIMULATION_3";
	BFG::Controller_::fillWithDefaultActions(actions);
	BFG::Controller_::sendActionsToController(emitter.loop(), actions);

	// Actions must be configured by XML
	BFG::Path path;
	const std::string configPath = path.Expand("SynchronizationTest.xml");
	const std::string stateName = "SynchronizationTest";

	// The Controller must know about the size of the window for the mouse
	BFG::View::WindowAttributes wa;
	BFG::View::queryWindowAttributes(wa);
	
	// Finally, send everything to the Controller
	BFG::Controller_::StateInsertion si(configPath, stateName, stateHandle, true, wa);
	emitter.emit<BFG::Controller_::ControlEvent>
	(
		BFG::ID::CE_LOAD_STATE,
		si
	);
}

boost::scoped_ptr<ServerState> gServerState;
boost::scoped_ptr<ViewState> gViewState;
boost::scoped_ptr<ClientState> gClientState;

void* createServerState(void* p)
{
	EventLoop* loop = static_cast<EventLoop*>(p);

	// The different states might be seen as different viewing points of
	// one state of an application or game. Thus they always share the same
	// handle since they work closely together.
	gServerState.reset(new ServerState(SERVER_STATE_HANDLE, loop));

	return 0;
}

void* createClientStates(void* p)
{
	EventLoop* loop = static_cast<EventLoop*>(p);
	
	// The different states might be seen as different viewing points of
	// one state of an application or game. Thus they always share the same
	// handle since they work closely together.
	gViewState.reset(new ViewState(CLIENT_STATE_HANDLE, loop));
	gClientState.reset(new ClientState(CLIENT_STATE_HANDLE, loop));

	initController(CLIENT_STATE_HANDLE, loop);
	return 0;
}

template <class T>
bool from_string(T& t, 
                 const std::string& s, 
                 std::ios_base& (*f)(std::ios_base&))
{
	std::istringstream iss(s);
	return !(iss >> f >> t).fail();
}

int main( int argc, const char* argv[] ) try
{
	bool server = false;
	if  (argc == 2)
		server = true;
	else if (argc == 3)
		server = false;
	else
	{
		std::cerr << "For Server use: bfgSynchronizationTest <Port>\nFor Client use: bfgSynchronizationTest <IP> <Port>\n";
		BFG::Base::pause();
		return 0;
	}	

	EventLoop loop1
	(
		true,
		new EventSystem::BoostThread<>("Loop1"),
		new EventSystem::NoCommunication()
	);

	BFG::Base::Logger::SeverityLevel level = BFG::Base::Logger::SL_DEBUG;
	if (server)
	{
		BFG::Path p;
		BFG::Base::Logger::Init(level, p.Get(BFG::ID::P_LOGS) + "/SynchronizationTestServer.log");

		BFG::u16 port = 0;

		if (!from_string(port, argv[1], std::dec))
		{
			std::cerr << "Port not a number: " << argv[1] << std::endl;
			BFG::Base::pause();
			return 0;
		}

		loop1.addEntryPoint(BFG::Network::Interface::getEntryPoint(BFG_SERVER));
		loop1.addEntryPoint(BFG::ModelInterface::getEntryPoint());
		loop1.addEntryPoint(BFG::Physics::Interface::getEntryPoint());
		loop1.addEntryPoint(new BFG::Base::CEntryPoint(&createServerState));
		loop1.run();

		BFG::Emitter e(&loop1);
		e.emit<BFG::Network::ControlEvent>(BFG::ID::NE_LISTEN, port);

		BFG::Base::pause();

		loop1.stop();

		gServerState.reset();
	}
	else
	{
		std::string ip(argv[1]);
		std::string port(argv[2]);

		BFG::Path p;
		BFG::Base::Logger::Init(level, p.Get(BFG::ID::P_LOGS) + "/SynchronizationTestClient.log");

		size_t controllerFrequency = 1000;

		loop1.addEntryPoint(BFG::Network::Interface::getEntryPoint(BFG_CLIENT));
		loop1.addEntryPoint(BFG::ModelInterface::getEntryPoint());
		loop1.addEntryPoint(BFG::Physics::Interface::getEntryPoint());
		loop1.addEntryPoint(BFG::View::Interface::getEntryPoint("SynchronizationTest"));
		loop1.addEntryPoint(BFG::ControllerInterface::getEntryPoint(controllerFrequency));
		loop1.addEntryPoint(new BFG::Base::CEntryPoint(&createClientStates));
		loop1.run();

		BFG::Network::EndpointT payload = make_tuple(stringToArray<128>(ip), stringToArray<128>(port));

		BFG::Emitter e(&loop1);
		e.emit<BFG::Network::ControlEvent>(BFG::ID::NE_CONNECT, payload);

		while(!loop1.shouldExit())
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(100));
		}

		gViewState.reset();
		gClientState.reset();
	}

	boost::this_thread::sleep(boost::posix_time::milliseconds(500));

	infolog << "Good Bye!";



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