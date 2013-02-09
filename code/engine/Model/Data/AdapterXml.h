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

#ifndef ADAPTER_XML_H_
#define ADAPTER_XML_H_

#include <Core/XmlFileHandle.h>
#include <Model/Data/AdapterConfig.h>

namespace BFG
{

class AdapterXml
{
public:
	
	AdapterXml(XmlFileHandleT adapterConfig) : mAdapterConfigFile(adapterConfig)
	{
		load();
	}

	// This typedef will be used for abstraction in FileHandleFactory.h.
	typedef AdapterConfigT ReturnT;
	
	ReturnT create(const std::string& adapterConfigName)
	{
		AdapterConfigsT::iterator it = mAdapterConfigs.find(adapterConfigName);
		
		if (it == mAdapterConfigs.end())
		{
			// return a NULL pointer with boost::shared_ptr type.
			AdapterConfigT t;
			t.reset();
			
			return t;
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
			mAdapterConfigs[(*it)->attribute("name")] = AdapterConfigT(new AdapterConfig(*it));
		}
	}

private:
	
	typedef std::map<std::string, AdapterConfigT> AdapterConfigsT;

	XmlFileHandleT mAdapterConfigFile;
	AdapterConfigsT mAdapterConfigs;
};

typedef boost::shared_ptr<AdapterXml> AdapterXmlT;

}

#endif