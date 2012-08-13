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


#ifndef BFG_DYNAMIC_VARIABLE__
#define BFG_DYNAMIC_VARIABLE__

namespace BFG
{

/*!
	\brief class for dynamic variables, which are interpolated themselves
 */
class DynamicVariable
{
public:
	DynamicVariable() : mCurrent(0.0f), mTarget(0.0f), mTime(0.0f), mTimeTarget(0.0f) {}
	DynamicVariable(f32 targetValue, f32 targetReachedTime = 1.0f, f32 current = 0.0f) :
	mCurrent(current),
	mTarget(targetValue),
	mTime(0.0f),
	mTimeTarget(targetReachedTime)
	{}

	//! set time in which target is to be reached
	void setTime(f32 time)
	{
		mTime = 0.0f;
		mTimeTarget = time;
	//	mCurrent = 0.0f;
	}

	void setCurrent(f32 current)
	{
		mCurrent = current;
	}

	//! update per frame
	void update(f32 deltaTime)
	{
		mTime += deltaTime;
		if(hasReachedTarget()) mCurrent = mTarget;
		else mCurrent += (deltaTime/mTimeTarget) * mDelta;
	}

	//! f32 operator
	operator f32() const { return mCurrent; }

	//! = operator
	f32 operator = (f32 value)
	{
		mTarget = value;
		mDelta = mTarget - mCurrent;
		return mCurrent;
	}

	//ask, if target is reached or not
	bool hasReachedTarget() const { return mTime >= mTimeTarget; }
private:
	f32 mCurrent;		// current value
	f32 mTarget;		// target value
	f32 mDelta;		// difference between begin and end value
	f32 mTime;		// current Time
	f32 mTimeTarget;	// target time
};
}

#endif //BFG_DYNAMIC_VARIABLE__
