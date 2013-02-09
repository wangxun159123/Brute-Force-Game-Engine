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

#ifndef BFG_OBJECT_XML_H_
#define BFG_OBJECT_XML_H_

#include <map>

#include <Model/Data/Types.h>
#include <Model/Data/Connection.h>
#include <Model/Property/Plugin.h>

#include <Model/Data/ModuleConfig.h>

namespace BFG {

//! This class is according to the object.xml.
class ObjectXml
{
public:

	// This typedef will be used for abstraction in FileHandleFactory.h.
	typedef ModuleConfigT ReturnT;

	ObjectXml(XmlFileHandleT objectConfig) : mObjectConfigFile(objectConfig)
	{
		load();
	}

	ReturnT create(const std::string& objectConfigName)
	{
		ModuleConfigsT::iterator it = mObjectConfigs.find(objectConfigName);

		if (it == mObjectConfigs.end())
		{
			// return a NULL pointer with boost::shared_ptr type.
			ModuleConfigT t;
			t.reset();
			
			return t;
		}

		return it->second;
	}

protected:

	void load()
	{
		XmlTreeListT objectConfigs = mObjectConfigFile->root()->child("ObjectConfigs")->childList("ObjectConfig");
		XmlTreeListT::iterator it = objectConfigs.begin();

		for (;it != objectConfigs.end(); ++it)
		{
			XmlTreeT tree = *it;
			mObjectConfigs[tree->attribute("name")] = ModuleConfigT(new ModuleConfig(tree));
		}
	}

private:

	typedef std::map<std::string, ModuleConfigT> ModuleConfigsT;

	XmlFileHandleT mObjectConfigFile;
	ModuleConfigsT mObjectConfigs;
};

typedef boost::shared_ptr<ObjectXml> ObjectXmlT;

} // namespace BFG

#endif
