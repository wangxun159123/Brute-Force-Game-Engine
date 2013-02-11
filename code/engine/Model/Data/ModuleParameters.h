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

#ifndef MODULE_PARAMETERS_H__
#define MODULE_PARAMETERS_H__

#include <Core/strToBool.h>

#include <Physics/Enums.hh>

#include <Model/Data/Connection.h>
#include <Model/Property/Plugin.h>


namespace BFG {

struct ModuleParameters
{
	ModuleParameters(XmlTreeT tree)
	{
		load(tree);
	}

	std::string mName;
	
	std::string mMesh;
	std::string mAdapter;
	std::string mConcept;
	ID::CollisionMode mCollision;
	bool mVisible;
	f32 mDensity;
    Connection mConnection;	

protected:

	void load(XmlTreeT tree)
	{
		try
		{
			mName = tree->attribute("name");

			mMesh = tree->child("Mesh")->elementData();
			mAdapter = tree->child("Adapters")->elementData();
			mConcept = tree->child("Concepts")->elementData();
			
			std::string collisionMode = tree->child("Collision")->elementData();
			
			if (!collisionMode.empty())
				mCollision = ID::asCollisionMode(collisionMode);
		
			std::string visibleStr = tree->child("Visible")->elementData();
			
			if (!visibleStr.empty())
				strToBool(visibleStr, mVisible);

			std::string density = tree->child("Density")->elementData();
			
			if (!density.empty())
				mDensity = boost::lexical_cast<f32>(density);
			
			parseConnection(tree->child("Connection")->elementData(), mConnection);
		}
		catch (std::exception& e)
		{
			throw std::logic_error(e.what()+std::string(" At ModuleParameters::load(...)"));
		}
	}
};

typedef boost::shared_ptr<ModuleParameters> ModuleParametersT;

} // namespace BFG

#endif
