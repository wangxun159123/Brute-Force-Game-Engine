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

#ifndef SI_CONCEPT_FACTORY
#define SI_CONCEPT_FACTORY

#include <Model/GameObject.h>
#include <Model/Property/Concept.h>

#include "ShipControl.h"
#include "InvaderControl.h"
#include "Cannon.h"
#include "Collectable.h"


using namespace BFG;

class SiConceptFactory : public Property::ConceptFactory
{
public:
	SiConceptFactory(Property::PluginId pid) :
	ConceptFactory(pid)
	{}

	virtual boost::shared_ptr<Property::Concept>
	createConcept(Property::ConceptId pc, GameObject& go)
	{
		PROPERTYCONCEPT_BUILD_LIST_BEGIN
		PROPERTYCONCEPT_CASE(ShipControl)
		PROPERTYCONCEPT_CASE(InvaderControl)
		PROPERTYCONCEPT_CASE(Cannon)
		PROPERTYCONCEPT_CASE(Collectable)
		PROPERTYCONCEPT_BUILD_LIST_END
	}
};

#endif