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

#ifndef CONCEPT_FACTORY_H_
#define CONCEPT_FACTORY_H_

#include <boost/lexical_cast.hpp>

#include <map>

#include <Core/XmlTree.h>
#include <Core/XmlFileHandle.h>

#include <Core/v3.h>
#include <Core/qv4.h>

#include <Model/Property/ConceptId.h>


namespace BFG
{

struct ConceptParameter
{
	ConceptParameter(XmlTreeT tree)
	{
		load(tree);
	}

	Property::ConceptId mName;
	std::string mProperties;	

protected:
	
	void load(XmlTreeT tree)
	{
		mName = tree->attribute("name");
		mProperties = tree->elementData();
	}
};
typedef boost::shared_ptr<ConceptParameter> ConceptParameterT;


struct ConceptConfig
{
	ConceptConfig(const XmlTreeListT& treeList)
	{
		load(treeList);
	}

	typedef std::vector<ConceptParameterT> ConceptParameterListT;
	ConceptParameterListT mConceptParameters;

protected:
	
	void load(const XmlTreeListT& treeList)
	{
		XmlTreeListT::const_iterator it = treeList.begin();

		for(; it != treeList.end(); ++it)
		{
			mConceptParameters.push_back(ConceptParameterT(new ConceptParameter(*it)));
		}
	}
};

typedef boost::shared_ptr<ConceptConfig> ConceptConfigT;


class ConceptFactory
{

public:
	
	ConceptFactory(XmlFileHandleT conceptConfig) : mConceptConfigFile(conceptConfig)
	{
		load();
	}

	ConceptConfigT create(const std::string& conceptName)
	{
		ConceptConfigMapT::iterator it = mConcepts.find(conceptName);

		if (it == mConcepts.end())
		{
			throw std::logic_error("ConceptFactory: "+conceptName+" not found.");
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

typedef boost::shared_ptr<ConceptFactory> ConfigFileHandlerT;

}

#endif