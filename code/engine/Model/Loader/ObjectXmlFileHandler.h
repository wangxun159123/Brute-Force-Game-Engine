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

#ifndef OBJECT_XML_FILE_HANDLER_H_
#define OBJECT_XML_FILE_HANDLER_H_

#include <map>
#include <boost/weak_ptr.hpp>

#include <EventSystem/Emitter.h>

#include <Model/Loader/Types.h>
#include <Model/Loader/Connection.h>
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
    Loader::Connection mConnection;	

protected:

	void load(XmlTreeT tree)
	{
		mName = tree->attribute("name");

		mMesh = tree->child("Mesh")->elementData();
		mAdapter = tree->child("Adapters")->elementData();
		mConcept = tree->child("Concepts")->elementData();
		mCollision = ID::asCollisionMode(tree->child("Collision")->elementData());
		
		mVisible = boost::lexical_cast<bool>(tree->child("Visible")->elementData());
		mDensity = boost::lexical_cast<f32>(tree->child("Density")->elementData());
		Loader::parseConnection(tree->child("Connection")->elementData(), mConnection);
	}
};

typedef boost::shared_ptr<ModuleParameters> ModuleParametersT;

struct ObjectConfigParameters
{
	ObjectConfigParameters(XmlTreeT tree)
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
		XmlTreeListT childList = tree->childList("ObjectConfig");

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


typedef boost::shared_ptr<ObjectConfigParameters> ObjectConfigParametersT;

//! This class is according to the object.xml.
class ObjectXmlFileHandler
{
public:

	ObjectXmlFileHandler(XmlFileHandleT objectConfig) : mObjectConfigFile(objectConfig)
	{
		load();
	}

	ObjectConfigParametersT create(const std::string& objectConfigName)
	{
		ObjectConfigsT::iterator it = mObjectConfigs.find(objectConfigName);

		if (it == mObjectConfigs.end())
		{
			// todo substiude throwing exception with a more robust error handling. Exception throwing will be moved one level higher.
			// throw std::logic_error("Fileformat in: "+mObjectConfigFile->path()+" Could not find "+objectConfigName);	
			
			ObjectConfigParametersT t;
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
			mObjectConfigs[tree->attribute("name")] = ObjectConfigParametersT(new ObjectConfigParameters(tree));
		}
	}

private:

	typedef std::map<std::string, ObjectConfigParametersT> ObjectConfigsT;

	XmlFileHandleT mObjectConfigFile;
	ObjectConfigsT mObjectConfigs;
};

typedef boost::shared_ptr<ObjectXmlFileHandler> ObjectXmlFileHandlerT;

} // namespace BFG

#endif
