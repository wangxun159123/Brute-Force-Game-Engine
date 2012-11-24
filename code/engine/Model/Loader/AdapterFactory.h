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

#ifndef MODULE_PARAMETER_H_
#define MODULE_PARAMETER_H_

#include <boost/lexical_cast.hpp>

#include <Core\v3.h>
#include <Core\qv4.h>


namespace BFG
{

v3 loadVector3(XmlTreeT tree)
{
	std::string x = tree->child("x")->elementData();
	std::string y = tree->child("y")->elementData();
	std::string z = tree->child("z")->elementData();

	return v3(boost::lexical_cast<f32>(x), 
	          boost::lexical_cast<f32>(y),
	          boost::lexical_cast<f32>(z));
}

qv4 loadQuaternion(XmlTreeT tree)
{
	std::string w = tree->child("w")->elementData();
	std::string x = tree->child("x")->elementData();
	std::string y = tree->child("y")->elementData();
	std::string z = tree->child("z")->elementData();

	return qv4(boost::lexical_cast<f32>(w),
	           boost::lexical_cast<f32>(x),
	           boost::lexical_cast<f32>(y),
	           boost::lexical_cast<f32>(z));
}

struct AdapterParameters
{
	AdapterParameters(XmlTreeT tree)
	{
		load(tree);
	}
	
	u32 mId;
	std::string mName;
	v3 mPosition;
	qv4 mOrientation;

protected:
	
	void load(XmlTreeT tree)
	{
		mId = boost::lexical_cast<u32>(tree->attribute("id"));
		mName = tree->attribute("name");

		mPosition = loadVector3(tree->child("position"));
		mOrientation = loadQuaternion(tree->child("orientation"));
	}
};

struct AdapterConfigParameters
{
	AdapterConfigParameters(XmlTreeT tree)
	{
		load(tree);
	}
	
	std::string mName;
	std::vector<AdapterParameters> mAdapter;
	
protected:

	void load(XmlTreeT tree)
	{
		mName = tree->attribute("name");
		XmlTreeListT childList = tree->childList("Adapter");
		
		XmlTreeListT::iterator it = childList.begin();
		for (;it != childList.end();++it)
		{
			mAdapter.push_back(AdapterParameters(*it));
		}
	}
};


typedef boost::shared_ptr<AdapterConfigParameters> AdapterConfigParametersT;

class AdapterFactory
{
public:
	
	AdapterFactory(XmlFileHandleT adapterConfig) : mAdapterConfigFile(adapterConfig)
	{
		load();
	}

	AdapterConfigParametersT createAdapters(const std::string& adapterConfigName)
	{
		AdapterConfigsT::iterator it = mAdapterConfigs.find(adapterConfigName);
		
		if (it == mAdapterConfigs.end())
		{
			throw std::logic_error("Fileformat in: "+mAdapterConfigFile->path()+" Could not find "+adapterConfigName);	
		}

		return it->second;
	}
	
protected:

	void load()
	{
		XmlTreeListT adapterConfigs = mAdapterConfigFile->root()->child("AdapterConfigs")->childList("AdapterConfig");
		XmlTreeListT::iterator it = adapterConfigs.begin();
		
		for (;it != adapterConfigs.end(); ++it)
		{
			mAdapterConfigs[(*it)->attribute("name")] = boost::shared_ptr<AdapterConfigParameters>(new AdapterConfigParameters(*it));
		}
	}

private:
	
	typedef std::map<std::string, AdapterConfigParametersT> AdapterConfigsT;

	XmlFileHandleT mAdapterConfigFile;
	AdapterConfigsT mAdapterConfigs;
};

}

#endif