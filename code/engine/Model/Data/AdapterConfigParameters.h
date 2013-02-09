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

#ifndef ADAPTER_CONFIG_PARAMETERS_H_
#define ADAPTER_CONFIG_PARAMETERS_H_

#include <boost/lexical_cast.hpp>
#include <Core/XmlTree.h>

#include <Model/Data/AdapterParameters.h>

namespace BFG
{

struct AdapterConfigParameters
{
	AdapterConfigParameters(XmlTreeT tree)
	{
		load(tree);
	}
	
	std::string mName;
	typedef std::vector<AdapterParametersT> AdapterParameterListT;
	AdapterParameterListT mAdapters;
	
protected:

	void load(XmlTreeT tree)
	{
		mName = tree->attribute("name");
		XmlTreeListT childList = tree->childList("Adapter");
		
		XmlTreeListT::iterator it = childList.begin();
		for (;it != childList.end();++it)
		{
			mAdapters.push_back(AdapterParametersT(new AdapterParameters(*it)));
		}
	}
};

typedef boost::shared_ptr<AdapterConfigParameters> AdapterConfigParametersT;

}

#endif