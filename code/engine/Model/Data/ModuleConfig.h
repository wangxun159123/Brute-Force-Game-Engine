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

#ifndef BFG_MODULE_CONFIG_H_
#define BFG_MODULE_CONFIG_H_

#include <Core/XmlTree.h>
#include <Model/Data/ModuleParameters.h>


namespace BFG {

struct ModuleConfig
{
	ModuleConfig(XmlTreeT tree)
	{
		load(tree);
	}

	std::string mName;
	typedef std::vector<ModuleParametersT> ModulesT;
	ModulesT mModules;

protected:

	void load(XmlTreeT tree)
	{
		mName = tree->attribute("name");
		XmlTreeListT childList = tree->childList("Module");

		XmlTreeListT::iterator it = childList.begin();
		for (;it != childList.end();++it)
		{
			XmlTreeT tree = *it;
			mModules.push_back(ModuleParametersT(new ModuleParameters(tree)));
		}
	
		if (mModules.empty())
			throw std::runtime_error("ObjectConfigParameters::load(): GameObjects must have at least one Module!");
	}
};

typedef boost::shared_ptr<ModuleConfig> ModuleConfigT;

} // namespace BFG

#endif
