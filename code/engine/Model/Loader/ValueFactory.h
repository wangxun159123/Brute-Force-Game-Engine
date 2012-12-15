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

#ifndef LOADER_VALUE_FACTORY_H_
#define LOADER_VALUE_FACTORY_H_

#include <boost/lexical_cast.hpp>

#include <map>

#include <Core/XmlTree.h>
#include <Core/XmlFileHandle.h>

#include <Core/v3.h>
#include <Core/qv4.h>

#include <Model/Property/ConceptId.h>
#include <Model/Loader/Interpreter.h>
#include <Model/Defs.h>


namespace BFG
{

struct ValueParam
{
	ValueParam(XmlTreeT tree)
	{
		load(tree);
	}
	
	std::string mName;
	Property::Value mValue;

protected:
	
	void load(XmlTreeT tree)
	{
		
		mName = tree->attribute("name");
		std::string type = tree->attribute("type");
		
		if (type == "v3")
		{
			mValue = loadVector3(tree);
		}
		else 
		if (type == "qv4")
		{
			mValue = loadQuaternion(tree);
		}
		else
		{
			mValue = Loader::StringToPropertyValue(tree->elementData());
		}
	}
};

typedef boost::shared_ptr<ValueParam> ValueParameterT;


struct ValueConfig
{
	ValueConfig(const XmlTreeListT& treeList)
	{
		load(treeList);
	}

	typedef std::vector<ValueParameterT> ValueParameterListT;
	ValueParameterListT mValueParameters;

protected:
	
	void load(const XmlTreeListT& treeList)
	{
		XmlTreeListT::const_iterator it = treeList.begin();

		for(; it != treeList.end(); ++it)
		{
			mValueParameters.push_back(ValueParameterT(new ValueParam(*it)));
		}
	}
};

typedef boost::shared_ptr<ValueConfig> ValueConfigT;


class ValueFactory
{

public:
	
	ValueFactory(XmlFileHandleT valueConfig) : mValueConfigFile(valueConfig)
	{
		load();
	}

	~ValueFactory() {}

	ValueConfigT createValueParameter(const std::string& valueName)
	{
		ValueMapT::iterator it = mValues.find(valueName);

		if (it == mValues.end())
		{
			throw std::logic_error("ValueFactory: "+valueName+" not found.");
		}

		return it->second;
	}

protected:

	void load()
	{
		XmlTreeListT valueConfigs = mValueConfigFile->root()->child("ValueConfigs")->childList("ValueConfig");
		XmlTreeListT::iterator it = valueConfigs.begin();
	
		for (;it < valueConfigs.end(); ++it)
		{
			mValues[(*it)->attribute("name")] = ValueConfigT(new ValueConfig((*it)->childList("PV")));
		}
	}

	XmlFileHandleT mValueConfigFile;
	typedef std::map<std::string, ValueConfigT> ValueMapT;
	ValueMapT mValues;
};

}

#endif