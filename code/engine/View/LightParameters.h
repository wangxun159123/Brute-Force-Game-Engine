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

#ifndef BFG_VIEW_LIGHTPARAMETERS_H
#define BFG_VIEW_LIGHTPARAMETERS_H

#include <Core/ExternalTypes.h>
#include <Core/v3.h>

#include <View/Enums.hh>

namespace BFG {
namespace View {

struct VIEW_API LightParameters
{
	LightParameters() : 
	mDirection(v3::ZERO),
	mPosition(v3::ZERO),
	mRange(100000.0f),
	mConstant(1.0f),
	mLinear(0.0f),
	mQuadric(0.0f),
	mFalloff(1.0f),
	mInnerRadius(30.0f),
	mOuterRadius(40.0f),
	mDiffuseColor(cv4::White),
	mSpecularColor(cv4::Black),
	mPower(1.0f)
	{}

	std::string   mName;
	GameHandle    mHandle;
	ID::LightType mType;
	v3            mDirection;
	v3            mPosition;
	f32           mRange;
	f32           mConstant;
	f32           mLinear;
	f32           mQuadric;
	f32           mFalloff;
	f32           mInnerRadius;
	f32           mOuterRadius;
	cv4           mDiffuseColor;
	cv4           mSpecularColor;
	f32           mPower;
};

} // namespace View
} // namespace BFG

#endif
