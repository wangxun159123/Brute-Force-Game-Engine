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

#ifndef __EVENT_MANAGER_H_
#define __EVENT_MANAGER_H_

#include <string>
#include <map>
#include <queue>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include <Base/Singleton.h>

#include <EventSystem/Core/EventPool.h>
#include <EventSystem/Core/EventChannel.h>

class EventLoop;
class EventManager;

static const int max_threads = 32;

typedef std::map<long, long> EventChannelMaskMap;
typedef std::map<std::string, long> EventIdBinding;

//! The EventManager coordinates the Threads, EventChannels and the EventExecution.
//!
//! Incoming means : Communication from the EventManager to the EventLoop.
//! Outgoing means : Communication from the EventLoop to the EventManager.
//!
//! An EventManager can be used by any class.
//!
//! \note This will only be used in a threaded environment
class EventManager
{
public:
	//! Singleton setup
	friend class BFG::Base::CLazyObject;
	static EventManager* getInstance();

	//! Destructor needs to be public
	~EventManager();

	static bool isInitialized()
	{
		return true;
	}

	long registerEventLoop(EventChannel* ioChannel);
	void unregisterEventLoop(EventChannel* ioChannel);
	void subscribeEvent(long eventId, EventChannel* ioChannel);
	void unsubscribeEvent(long eventId, EventChannel* ioChannel);
	void unsubscribeAll(EventChannel* ioChannel);

	long getDynamicEventId(const std::string& eventIdentifier);

	int getChannelId(EventChannel* ioChannel, bool should_lock = true);

	//! EventPool-Operations
	BaseEventPool* requestPool();
	void freePool(BaseEventPool* pool);

private:
	//! Ctor is only called by Singleton-creation function
	EventManager(void);

	//! Is exchanging Inter-Process data here
	bool doOperate();
	
	//! Entrypoint for the thread
	void runThread();

	//! The "worker" thread
	boost::thread mThread;

	// Determine how many locks actually are needed
	boost::mutex mInternalMutex;
	boost::mutex mEventChannelListMutex;
	boost::mutex mExecutionListMutex;
	boost::mutex mDynamicEventIdMapMutex;

	boost::recursive_mutex mEventPoolMutex;
	size_t mThreadCount;
	EventChannelMaskMap mExecutionList;
	EventChannel* mEventChannelList[max_threads];
	EventIdBinding mDynamicEventIdMap;
	std::deque<BaseEventPool*> mPoolQueue;

	//! Condition for shutting down the EventSystem
	bool mExitFlag;

	long mEventIdDynamicStartValue;
};

#endif // __EVENT_MANAGER_H_
