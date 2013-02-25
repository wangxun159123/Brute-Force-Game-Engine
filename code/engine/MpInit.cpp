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

#include "MpInit.h"

#include <sstream>

#include <Base/Logger.h>
#include <Base/Pause.h>
#include <Core/Path.h>
#include <Core/Types.h>
#include <Controller/Interface.h>
#include <EventSystem/Emitter.h>
#include <Network/Interface.h>
#include <Network/Event.h>
#include <View/Interface.h>

namespace BFG {

template <class T>
inline bool from_string(T& t, 
                 const std::string& s, 
                 std::ios_base& (*f)(std::ios_base&))
{
	std::istringstream iss(s);
	return !(iss >> f >> t).fail();
}

void mpInit(BFG::InitHandlerT handler, std::string logName, std::string& port)
{
	u16 intPort = 0;

	if (!from_string(intPort, port, std::dec))
		throw std::runtime_error("Port not a number: " + port);
	
	EventLoop loop
	(
		true,
		new EventSystem::BoostThread<>("Loop1"),
		new EventSystem::InterThreadCommunication()
	);

	Path p;
	Base::Logger::Init(Base::Logger::SL_DEBUG, p.Get(ID::P_LOGS) + "/" + logName);

	dbglog << "Starting as Server";

	loop.addEntryPoint(Network::Interface::getEntryPoint(BFG_SERVER));
	loop.run();
	
	Emitter e(&loop);
	e.emit<Network::ControlEvent>(ID::NE_LISTEN, intPort);
	
	handler(loop);
}

void mpInit(BFG::InitHandlerT handler, std::string logName, std::string& ip, std::string& port)
{
	u16 intPort = 0;

	if (!from_string(intPort, port, std::dec))
		throw std::runtime_error("Port not a number: " + port);
	
	EventLoop loop
	(
		true,
		new EventSystem::BoostThread<>("Loop1"),
		new EventSystem::InterThreadCommunication()
	);
	
	Path p;
	Base::Logger::Init(Base::Logger::SL_DEBUG, p.Get(ID::P_LOGS) + "/" + logName);

	dbglog << "Starting as Client";

	size_t controllerFrequency = 1000;
	loop.addEntryPoint(Network::Interface::getEntryPoint(BFG_CLIENT));
	loop.addEntryPoint(ControllerInterface::getEntryPoint(controllerFrequency));
	loop.addEntryPoint(View::Interface::getEntryPoint("Chat"));
	loop.run();

	boost::this_thread::sleep(boost::posix_time::milliseconds(5000));
	
	Network::EndpointT payload = boost::make_tuple(stringToArray<128>(ip), stringToArray<128>(port));
	Emitter e(&loop);

	e.emit<Network::ControlEvent>(ID::NE_CONNECT, payload);

	handler(loop);
}

} // namespace BFG
