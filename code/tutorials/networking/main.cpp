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

	This example application demonstrates how to create a networked
	3D environment with two players controlling each an object.
*/

// OGRE
#include <OgreException.h>

// BFG libraries
#include <Base/EntryPoint.h>
#include <Base/Logger.h>
#include <Base/Pause.h>
#include <Controller/Action.h>
#include <Controller/ControllerEvents.h>
#include <Controller/Interface.h>
#include <Core/Math.h>
#include <Core/Path.h>
#include <Core/ShowException.h>
#include <Core/Utils.h>
#include <Model/Environment.h>
#include <Model/Property/Concepts/Camera.h>
#include <Model/Sector.h>
#include <Model/State.h>
#include <Model/Data/GameObjectFactory.h>
#include <Model/Property/SpacePlugin.h>
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

// Use by the Server to notify the Client that it shall create the whole scene
// now.
const s32 CREATE_SCENE = 15000;

const s32 SHIP_AXIS_X = 15001;
const s32 SHIP_AXIS_Y = 15002;
const s32 SHIP_AXIS_Z = 15003;
const s32 SHIP_THRUST = 15004;


// We need to define a fix handle identifier for the server and client states.
const GameHandle SERVER_STATE_HANDLE = 42;
const GameHandle CLIENT_STATE_HANDLE = 43;

bool alwaysTrue(boost::shared_ptr<BFG::GameObject>)
{
	return true;
}

// This is the base for both versions of our program: Server and Client.
struct CommonState : BFG::State
{
	CommonState(GameHandle handle, EventLoop* loop) :
	State(loop),
	mStateHandle(handle),
	mPlayer(NULL_HANDLE),
	mPlayer2(NULL_HANDLE),
	mEnvironment(new BFG::Environment),
	mTimeSinceLastUpdate(0.0f)
	{
		BFG::Path p;
		
		// Load the default object definitions
		BFG::LevelConfig lc;
		std::string def = p.Get(BFG::ID::P_SCRIPTS_LEVELS) + "default/";
		lc.mModules.push_back(def + "Object.xml");
		lc.mAdapters.push_back(def + "Adapter.xml");
		lc.mConcepts.push_back(def + "Concept.xml");
		lc.mProperties.push_back(def + "Value.xml");

		// Load the default engine plugin. Such a plugin usually contains
		// new properties and concepts for game objects.
		using BFG::Property::ValueId;
		BFG::PluginId spId = BFG::ValueId::ENGINE_PLUGIN_ID;
		boost::shared_ptr<BFG::SpacePlugin> sp(new BFG::SpacePlugin(spId));

		mPluginMap.insert(sp);

		// Create a GameObjectFactory which we can use to create new
		// game objects from the XML definition.
		mGof.reset(new BFG::GameObjectFactory(this->loop(), lc, mPluginMap, mEnvironment, mStateHandle));
		
		// Create a sector. By sending events it will create objects
		// automatically for us.
		mSector.reset(new BFG::Sector(this->loop(), 1, "Blah", mGof));
	}
	
	virtual ~CommonState()
	{
		mSector.reset();
		mGof.reset();
	}

	// You may update objects and other things here.
	virtual void onTick(const quantity<si::time, f32> TSLF)
	{
		if (TSLF.value() < BFG::EPSILON_F)
		{
			mTimeSinceLastUpdate += TSLF.value();
			return;
		}
		else
		{
			mTimeSinceLastUpdate = TSLF.value();
		}
		if (mSector)
		{
			mSector->update(mTimeSinceLastUpdate * si::seconds);
			emit<BFG::Physics::Event>(BFG::ID::PE_STEP, mTimeSinceLastUpdate);
		}
	}
	
	virtual void createObject(const BFG::ObjectParameter& param)
	{
		boost::shared_ptr<BFG::GameObject> go = mGof->createGameObject(param);
		mSector->addObject(go);
	}

	virtual void createCamera(const BFG::CameraParameter& param)
	{
		boost::shared_ptr<BFG::GameObject> go = mGof->createCamera(param, param.mParentObject);
		mSector->addObject(go);
	}
	
	virtual void destroyObject(GameHandle handle)
	{
		mSector->removeObject(handle);
	}
	
protected:
	GameHandle mStateHandle;
	GameHandle mPlayer;
	GameHandle mPlayer2;
	boost::shared_ptr<BFG::Environment> mEnvironment;
	
private:
	BFG::Property::PluginMapT mPluginMap;
	boost::shared_ptr<BFG::Sector> mSector;
	boost::shared_ptr<BFG::GameObjectFactory> mGof;
	f32 mTimeSinceLastUpdate;
};

// Here comes the Client state. It performs the following tasks:
//
//  * Receive network updates
//  * Receive control updates
//  * Create the scene when connecting
//
struct ClientState : CommonState
{
	ClientState(EventLoop* loop) :
	CommonState(CLIENT_STATE_HANDLE, loop)
	{
		// This part is quite important. You must connect your event callbacks.
		// If not, the event system doesn't know you're waiting for them.
		loop->connect(A_EXIT, this, &ClientState::ControllerEventHandler);
		loop->connect(SHIP_AXIS_X, this, &ClientState::ControllerEventHandler);
		loop->connect(SHIP_AXIS_Y, this, &ClientState::ControllerEventHandler);
		loop->connect(SHIP_AXIS_Z, this, &ClientState::ControllerEventHandler);
		loop->connect(SHIP_THRUST, this, &ClientState::ControllerEventHandler);
		loop->connect(BFG::ID::NE_RECEIVED, this, &ClientState::networkEventHandler, CLIENT_STATE_HANDLE);
	}
	
	virtual ~ClientState()
	{
		infolog << "Tutorial: Destroying ClientState.";
		loop()->disconnect(A_EXIT, this);
		loop()->disconnect(SHIP_AXIS_X, this);
		loop()->disconnect(SHIP_AXIS_Y, this);
		loop()->disconnect(SHIP_AXIS_Z, this);
		loop()->disconnect(SHIP_THRUST, this);
	}

	void onExit()
	{
		// Calling this will hold the update process of this State.
		// No further events might be received after this.
		loop()->stop();
	}

	// Callback for Input. The Controller sends input directly to this
	// state, since we told him so (in `initController').
	void ControllerEventHandler(BFG::Controller_::VipEvent* e)
	{
		infolog << "Got Event:" << e->id();
		switch(e->getId())
		{
			case SHIP_AXIS_X:
				onShipControlEvent(e->getId(), boost::get<f32>(e->getData()));
				break;

			case SHIP_AXIS_Y:
				onShipControlEvent(e->getId(), boost::get<f32>(e->getData()));
				break;

			case SHIP_AXIS_Z:
				onShipControlEvent(e->getId(), boost::get<f32>(e->getData()));
				break;
			case SHIP_THRUST:
				onShipControlEvent(e->getId(), boost::get<f32>(e->getData()));
				break;
			// This is the event ID we specified at the top
			case A_EXIT:
			{
				onExit();
				break;
			}
		}
	}
	
	void onShipControlEvent(const BFG::s32 id, const BFG::f32 controlValue)
	{
		dbglog << "ControlValue:" << controlValue;
		CharArray512T ca512 = CharArray512T();
		valueToArray(BFG::clamp(controlValue, -1.0f, 1.0f), ca512, 0);
		BFG::Network::DataPayload payload
		(
			id, 
			SERVER_STATE_HANDLE, 
			CLIENT_STATE_HANDLE,
			sizeof(f32),
			ca512
		);
		emit<BFG::Network::DataPacketEvent>(BFG::ID::NE_SEND, payload);
	}

	void onCreateScene(const BFG::Network::DataPayload& payload)
	{
		infolog << "Client: Creating scene";
		
		std::stringstream oss(payload.mAppData.data());

		BFG::ObjectParameter op;
		oss >> op.mHandle;
		op.mName = "Cube";
		op.mType = "Cube";
		op.mLocation = v3(0.0f, 0.0f, 10.0f);

		createObject(op);
		emit<BFG::GameObjectEvent>(BFG::ID::GOE_SYNCHRONIZATION_MODE, (s32)BFG::ID::SYNC_MODE_NETWORK_READ, op.mHandle);

		BFG::ObjectParameter op1;
		oss >> op1.mHandle;
		op1.mName = "Ship";
		op1.mType = "Ship";
		op1.mLocation = v3(5.0f, 0.0f, 15.0f);
		
		createObject(op1);
		emit<BFG::GameObjectEvent>(BFG::ID::GOE_SYNCHRONIZATION_MODE, (s32)BFG::ID::SYNC_MODE_NETWORK_READ, op1.mHandle);

		BFG::ObjectParameter op2;
		oss >> op2.mHandle;
		op2.mName = "Ship2";
		op2.mType = "Ship";
		op2.mLocation = v3(-5.0f, 0.0f, 15.0f);

		createObject(op2);
		emit<BFG::GameObjectEvent>(BFG::ID::GOE_SYNCHRONIZATION_MODE, (s32)BFG::ID::SYNC_MODE_NETWORK_READ, op2.mHandle);

		BFG::CameraParameter cp = BFG::CameraParameter();
		GameHandle parentHandle;
		oss >> parentHandle;
		if (op1.mHandle == parentHandle)
		{
			cp.mOffset = v3(0.0f, 3.0f, -10.0f);
			cp.mParentObject = op1.mName;
		}
		if (op2.mHandle == parentHandle)
		{
			cp.mOffset = v3(0.0f, 3.0f, -10.0f);
			cp.mParentObject = op2.mName;
		}

		createCamera(cp);
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
			case CREATE_SCENE:
				onCreateScene(payload);
				break;
			}
		}
		}
	}
};

struct ServerState : CommonState
{
	typedef std::map<BFG::Network::PeerIdT, BFG::GameHandle> ClientListT;
	
	ServerState(EventLoop* loop) :
	CommonState(SERVER_STATE_HANDLE, loop),
	mSceneCreated(false)
	{
		loop->connect(BFG::ID::NE_RECEIVED, this, &ServerState::networkPacketEventHandler, SERVER_STATE_HANDLE);
		loop->connect(BFG::ID::NE_CONNECTED, this, &ServerState::networkControlEventHandler);
		loop->connect(BFG::ID::NE_DISCONNECTED, this, &ServerState::networkControlEventHandler);
	}
	
	virtual ~ServerState()
	{
		loop()->disconnect(BFG::ID::NE_RECEIVED, this);
		loop()->disconnect(BFG::ID::NE_CONNECTED, this);
		loop()->disconnect(BFG::ID::NE_DISCONNECTED, this);
	}

	void networkPacketEventHandler(BFG::Network::DataPacketEvent* e)
	{

		switch(e->getId())
		{
		case BFG::ID::NE_RECEIVED:
		{
			const BFG::Network::DataPayload& payload = e->getData();

			switch(payload.mAppEventId)
			{
			case SHIP_AXIS_X:
			{
				GameHandle playerHandle = getPlayerHandle(e->sender());
				if (playerHandle == NULL_HANDLE)
					return;
				f32 data;
				arrayToValue(data, payload.mAppData, 0);
				dbglog << "Server received SHIP_AXIS_X (" << data << ")";
				emit<BFG::GameObjectEvent>(BFG::ID::GOE_CONTROL_PITCH, data, playerHandle);
				break;
			}
			case SHIP_AXIS_Y:
			{
				GameHandle playerHandle = getPlayerHandle(e->sender());
				if (playerHandle == NULL_HANDLE)
					return;
				f32 data;
				arrayToValue(data, payload.mAppData, 0);
				dbglog << "Server received SHIP_AXIS_Y (" << data << ")";
				emit<BFG::GameObjectEvent>(BFG::ID::GOE_CONTROL_YAW, data, playerHandle);
				break;
			}
			case SHIP_AXIS_Z:
			{
				GameHandle playerHandle = getPlayerHandle(e->sender());
				if (playerHandle == NULL_HANDLE)
					return;
				f32 data;
				arrayToValue(data, payload.mAppData, 0);
				dbglog << "Server received SHIP_AXIS_Z (" << data << ")";
				emit<BFG::GameObjectEvent>(BFG::ID::GOE_CONTROL_ROLL, data, playerHandle);
				break;
			}
			case SHIP_THRUST:
			{
				GameHandle playerHandle = getPlayerHandle(e->sender());
				if (playerHandle == NULL_HANDLE)
					return;
				f32 data;
				arrayToValue(data, payload.mAppData, 0);
				dbglog << "Server received SHIP_THRUST (" << data << ")";
				emit<BFG::GameObjectEvent>(BFG::ID::GOE_CONTROL_THRUST, data, playerHandle);
				break;
			}

			}
		}
		}

	}
	
	GameHandle getPlayerHandle(BFG::Network::PeerIdT peerId)
	{
		ClientListT::iterator it = mClientList.find(peerId);

		if (it == mClientList.end())
			return NULL_HANDLE;

		return it->second;
	}

	void createScene()
	{
		infolog << "Server: Creating scene";

		std::stringstream handles;

		BFG::ObjectParameter op;
		op.mHandle = BFG::generateNetworkHandle();
		op.mName = "Cube";
		op.mType = "Cube";
		op.mLocation = v3(0.0f, 0.0f, 10.0);
		handles << op.mHandle << " ";

		createObject(op);
		emit<BFG::GameObjectEvent>(BFG::ID::GOE_SYNCHRONIZATION_MODE, (s32)BFG::ID::SYNC_MODE_NETWORK_WRITE, op.mHandle);

		op = BFG::ObjectParameter();
		op.mHandle = BFG::generateNetworkHandle();
		op.mName = "Ship";
		op.mType = "Ship";
		op.mLocation = v3(5.0f, 0.0f, 15.0f);
		handles << op.mHandle << " ";
		mPlayer = op.mHandle;

		createObject(op);
		emit<BFG::GameObjectEvent>(BFG::ID::GOE_SYNCHRONIZATION_MODE, (s32)BFG::ID::SYNC_MODE_NETWORK_WRITE, op.mHandle);

		op = BFG::ObjectParameter();
		op.mHandle = BFG::generateNetworkHandle();
		op.mName = "Ship2";
		op.mType = "Ship";
		op.mLocation = v3(-5.0f, 0.0f, 15.0f);
		handles << op.mHandle << " ";
		mPlayer2 = op.mHandle;

		createObject(op);
		emit<BFG::GameObjectEvent>(BFG::ID::GOE_SYNCHRONIZATION_MODE, (s32)BFG::ID::SYNC_MODE_NETWORK_WRITE, op.mHandle);

		mCreatedHandles = handles.str();
		mSceneCreated = true;
	}

	void destroyScene()
	{
		std::vector<GameHandle> all = mEnvironment->find_all(&alwaysTrue);
		std::vector<GameHandle>::const_iterator it = all.begin();
		for (; it != all.end(); ++it)
		{
			destroyObject(*it);
		}
		
		mCreatedHandles = "";
		mSceneCreated = false;
	}
	
	void onConnected(BFG::Network::PeerIdT peerId)
	{
		infolog << "Client (" << peerId << ") connected.";
		ClientListT::iterator it = mClientList.find(peerId);
		
		// TODO: NetworkModule should prevent this from happening.
		if (it != mClientList.end())
		{
			warnlog << "Client with the same PeerID (" << peerId << ") already connected!";
			return;
		}
		
		if (!mSceneCreated)
		{
			createScene();
		}

		std::string handles = mCreatedHandles;

		if (mClientList.size() < 1)
		{
			mClientList.insert(std::make_pair(peerId, mPlayer));
			handles += BFG::stringify(mPlayer);
		}
		else if (mClientList.size() == 1)
		{
			mClientList.insert(std::make_pair(peerId, mPlayer2));
			handles += BFG::stringify(mPlayer2);
		}

		CharArray512T ca512 = stringToArray<512>(handles);

		BFG::Network::DataPayload payload
		(
			CREATE_SCENE, 
			CLIENT_STATE_HANDLE, 
			SERVER_STATE_HANDLE,
			handles.length(),
			ca512
		);
		
		emit<BFG::Network::DataPacketEvent>(BFG::ID::NE_SEND, payload, peerId);
	}
	
	void onDisconnected(BFG::Network::PeerIdT peerId)
	{
		ClientListT::iterator it = mClientList.find(peerId);

		// TODO: NetworkModule: Is this possible?
		if (it == mClientList.end())
		{
			warnlog << "Client (" << peerId << ") was not connected!";
			return;
		}

		mClientList.erase(it);

		if (mClientList.empty())
		{
			destroyScene();
		}
	}

	void networkControlEventHandler(BFG::Network::ControlEvent* e)
	{
		switch(e->getId())
		{
		case BFG::ID::NE_CONNECTED:
		{
			const BFG::Network::PeerIdT peerId = boost::get<BFG::Network::PeerIdT>(e->getData());
			onConnected(peerId);
			break;
		}
		case BFG::ID::NE_DISCONNECTED:
		{
			const BFG::Network::PeerIdT peerId = boost::get<BFG::Network::PeerIdT>(e->getData());
			onDisconnected(peerId);
			break;
		}
		} // switch
	}

private:
	ClientListT mClientList;
	
	std::string mCreatedHandles;
	bool mSceneCreated;
};

// We won't display anything, so this class remains more or less empty. In this
// engine, Model and View are separated, so as you guessed this is the same as
// the GameState, but this time for render stuff.
struct ViewState : public BFG::View::State
{
public:
	ViewState(EventLoop* loop) :
	State(CLIENT_STATE_HANDLE, loop),
	mControllerMyGuiAdapter(CLIENT_STATE_HANDLE, loop)
	{
		infolog << "Tutorial: Creating ViewState.";
		BFG::View::SkyCreation sc("sky02");
		emit<BFG::View::Event>(BFG::ID::VE_SET_SKY, sc, handle());
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
	actions[SHIP_AXIS_X] = "SHIP_AXIS_X";
	actions[SHIP_AXIS_Y] = "SHIP_AXIS_Y";
	actions[SHIP_AXIS_Z] = "SHIP_AXIS_Z";
	actions[SHIP_THRUST] = "SHIP_THRUST";
	BFG::Controller_::fillWithDefaultActions(actions);
	BFG::Controller_::sendActionsToController(emitter.loop(), actions);

	// Actions must be configured by XML
	BFG::Path path;
	const std::string configPath = path.Expand("TutorialNetworking.xml");
	const std::string stateName = "TutorialNetworking";

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

boost::scoped_ptr<ViewState> gViewState;
boost::scoped_ptr<ClientState> gClientState;
boost::scoped_ptr<ServerState> gServerState;

// Our ClientState needs everything.
void* createClientStates(void* p)
{
	EventLoop* loop = static_cast<EventLoop*>(p);
	
	gViewState.reset(new ViewState(loop));
	gClientState.reset(new ClientState(loop));

	initController(CLIENT_STATE_HANDLE, loop);
	return 0;
}

// Our ServerState doesn't need more than our GameState and the Network module
void* createServerStates(void* p)
{
	EventLoop* loop = static_cast<EventLoop*>(p);

	gServerState.reset(new ServerState(loop));

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
	bool isClient = true;
	if  (argc == 2)
		isClient = false;
	else if (argc == 3)
		isClient = true;
	else
	{
		std::cerr << "For Server use: bfgTutorialNetworking <Port>\n"
			"For Client use: bfgTutorialNetworking <IP> <Port>\n";
		BFG::Base::pause();
		return 0;
	}	
	

	EventLoop loop(true, new EventSystem::BoostThread<>("Loop"));

	const std::string caption = "Tutorial 02: Networking";
	size_t controllerFrequency = 1000;

	// Setting up callbacks for module initialization
	// This is still very inconsistent but a proof for flexibility ;)
	if (isClient)
	{
		// Our logger. Not used here, works like cout, but without the need for
		// endl and with multiple severities: dbglog, infolog, warnlog, errlog.
		BFG::Base::Logger::Init(BFG::Base::Logger::SL_ERROR, "Logs/TutorialNetworkingClient.log");
		infolog << "This is our client!";

		boost::scoped_ptr<BFG::Base::IEntryPoint> epView(BFG::View::Interface::getEntryPoint(caption));
		boost::scoped_ptr<BFG::Base::IEntryPoint> epController(BFG::ControllerInterface::getEntryPoint(controllerFrequency));
		boost::scoped_ptr<BFG::Base::IEntryPoint> epNetwork(BFG::Network::Interface::getEntryPoint(BFG_CLIENT));
		boost::scoped_ptr<BFG::Base::IEntryPoint> epPhysics(BFG::Physics::Interface::getEntryPoint());
		boost::scoped_ptr<BFG::Base::IEntryPoint> epGame(new BFG::Base::CEntryPoint(&createClientStates));
		
		// The order is important.
		loop.addEntryPoint(epView.get());
		loop.addEntryPoint(epController.get());
		loop.addEntryPoint(epNetwork.get());
		loop.addEntryPoint(epPhysics.get());
		loop.addEntryPoint(epGame.get());
		
		loop.run();
		
		std::string ip(argv[1]);
		std::string port(argv[2]);

		BFG::Network::EndpointT payload = make_tuple
		(
			stringToArray<128>(ip),
			stringToArray<128>(port)
		);

		BFG::Emitter e(&loop);
		e.emit<BFG::Network::ControlEvent>(BFG::ID::NE_CONNECT, payload);

		while(!loop.shouldExit())
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(100));
		}
		gViewState.reset();
	}
	else
	{
		// Our logger. Not used here, works like cout, but without the need for
		// endl and with multiple severities: dbglog, infolog, warnlog, errlog.
		BFG::Base::Logger::Init(BFG::Base::Logger::SL_ERROR, "Logs/TutorialNetworkingServer.log");
		infolog << "This is our server!";
		BFG::u16 port = 0;
		if (!from_string(port, argv[1], std::dec))
		{
			std::cerr << "Port not a number: " << argv[1] << std::endl;
			BFG::Base::pause();
			return 0;
		}
		
		boost::scoped_ptr<BFG::Base::IEntryPoint> epNetwork(BFG::Network::Interface::getEntryPoint(BFG_SERVER));
		boost::scoped_ptr<BFG::Base::IEntryPoint> epPhysics(BFG::Physics::Interface::getEntryPoint());
		boost::scoped_ptr<BFG::Base::IEntryPoint> epGame(new BFG::Base::CEntryPoint(&createServerStates));

		// The order is important.
		loop.addEntryPoint(epNetwork.get());
		loop.addEntryPoint(epPhysics.get());
		loop.addEntryPoint(epGame.get());
		
		loop.run();

		BFG::Emitter e(&loop);
		e.emit<BFG::Network::ControlEvent>(BFG::ID::NE_LISTEN, port);

		BFG::Base::pause();
	}

	// The loop does not run anymore. Destroy the states now.
	gClientState.reset();
	gServerState.reset();
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
