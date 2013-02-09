/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2012 Brute-Force Games GbR

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

#ifndef CONCEPT_XML_H_
#define CONCEPT_XML_H_

#include <map>

#include <Core/XmlFileHandle.h>
#include <Model/Data/ConceptConfig.h>


namespace BFG
{

class ConceptXml
{

public:
	
	// This typedef will be used for abstraction in FileHandleFactory.h.
	typedef ConceptConfigT ReturnT;

	ConceptXml(XmlFileHandleT conceptConfig) : mConceptConfigFile(conceptConfig)
	{
		load();
	}

	ConceptConfigT create(const std::string& conceptName)
	{
		ConceptConfigMapT::iterator it = mConcepts.find(conceptName);

		if (it == mConcepts.end())
		{
			// return a NULL pointer with boost::shared_ptr type.
			ConceptConfigT t;
			t.reset();
			
			return t;
		}

		return it->second;
	}

protected:

	void load()
	{
		XmlTreeListT conceptConfigs = mConceptConfigFile->root()->child("ConceptConfigs")->childList("ConceptConfig");
		XmlTreeListT::iterator it = conceptConfigs.begin();
	
		for (;it < conceptConfigs.end(); ++it)
		{
			mConcepts[(*it)->attribute("name")] = ConceptConfigT(new ConceptConfig((*it)->childList("PC")));
		}
	}

	XmlFileHandleT mConceptConfigFile;
	typedef std::map<std::string, ConceptConfigT> ConceptConfigMapT;
	ConceptConfigMapT mConcepts;
};

typedef boost::shared_ptr<ConceptXml> ConfigFileHandlerT;

}

#endif