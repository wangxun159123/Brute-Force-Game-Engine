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

#include <ModuleControl.h>

#include <Core/Utils.h>
#include <Model/Property/SpacePlugin.h>



namespace Tool
{
	void ModuleControl::load()
	{
		if (mLoaded)
			return;

		MyGUI::LayoutManager* layMan = MyGUI::LayoutManager::getInstancePtr();
		mContainer = layMan->loadLayout("ModuleControl.layout");

		MyGUI::Widget* panel = mContainer.front();

		mAddModule = panel->findWidget("addModule")->castType<MyGUI::Button>();

		mModules = panel->findWidget("modules")->castType<MyGUI::ScrollView>();

		mAddConnection = panel->findWidget("addConnection")->castType<MyGUI::Button>();

		mConnections = panel->findWidget("connections")->castType<MyGUI::ScrollView>();

		mPreview = panel->findWidget("preview")->castType<MyGUI::Button>();

		mAddModule->eventMouseButtonClick =
			MyGUI::newDelegate(this, &ModuleControl::onAddModuleClicked);

		mAddConnection->eventMouseButtonClick =
			MyGUI::newDelegate(this, &ModuleControl::onAddConnectionClicked);

		mPreview->eventMouseButtonClick =
			MyGUI::newDelegate(this, &ModuleControl::onPreviewClicked);

		mLoaded = true;
		deactivate();
	}

	void ModuleControl::unload()
	{
		if (!mLoaded)
			return;

		if (mActive)
			deactivate();

		SharedData::ModulesT::iterator it = mData->mModules.begin();
		for (; it != mData->mModules.end(); ++it)
		{
			delete *it;
		}
		mData->mModules.clear();

		mLoaded = false;
	}

	void ModuleControl::activate()
	{
		MyGUI::VectorWidgetPtr::iterator it = mContainer.begin();
		for (; it != mContainer.end(); ++it)
		{
			(*it)->setVisible(true);
		}

		mActive = true;
	}

	void ModuleControl::deactivate()
	{
		MyGUI::VectorWidgetPtr::iterator it = mContainer.begin();
		for (; it != mContainer.end(); ++it)
		{
			(*it)->setVisible(false);
		}

		mActive = false;
	}

	void ModuleControl::onAddModuleClicked(MyGUI::Widget*)
	{
		mData->mModules.push_back(new Module
		(
			mModules,
			mData->mAdapters,
			MyGUI::newDelegate(this, &ModuleControl::onCloseModuleClicked))
		);
	}

	void ModuleControl::onCloseModuleClicked(MyGUI::Widget* sender)
	{
		SharedData::ModulesT::iterator modIt = 
			std::find
			(
				mData->mModules.begin(),
				mData->mModules.end(),
				*sender->getUserData<Module*>()
			);

		if (modIt != mData->mModules.end())
		{
			Module* ptr = (*modIt);
			mData->mModules.erase(modIt);
			delete ptr;
		}
	}

	void ModuleControl::onAddConnectionClicked( MyGUI::Widget* )
	{
		mData->mConnections.push_back(new Connection
		(
			mConnections,
			mData->mModules,
			mData->mAdapters,
			MyGUI::newDelegate(this, &ModuleControl::onCloseConnectionClicked))
		);

	}

	void ModuleControl::onCloseConnectionClicked( MyGUI::Widget* sender )
	{
		SharedData::ConnectionsT::iterator conIt = 
			std::find
			(
				mData->mConnections.begin(),
				mData->mConnections.end(),
				*sender->getUserData<Connection*>()
			);

		if (conIt != mData->mConnections.end())
		{
			Connection* ptr = (*conIt);
			mData->mConnections.erase(conIt);
			delete ptr;
		}
	}

	void ModuleControl::onPreviewClicked( MyGUI::Widget* )
	{
		std::string rootModule(findRootModule());

		createRoot(rootModule);

		addModuleTo(rootModule);
	}

	void ModuleControl::destroyGameObject()
	{
		if (mData->mGameObject)
			delete mData->mGameObject;
	}

	Module* ModuleControl::findModule(const std::string& name)
	{
		SharedData::ModulesT::iterator modIt = mData->mModules.begin();
		for (; modIt != mData->mModules.end(); ++modIt)
		{
			if ((*modIt)->mName->getCaption() == name)
				break;
		}

		if (modIt == mData->mModules.end())
			throw std::runtime_error("Module not found");

		return (*modIt);
	}

	void ModuleControl::createRoot(const std::string& name)
	{
		Module* modDesc = findModule(name);

		std::string meshName(modDesc->mMesh->getItemNameAt(modDesc->mMesh->getIndexSelected()));
		
		if (!(mData->mRootMesh))
		{
			mData->mRootMesh = BFG::generateHandle();
		}

		mData->mRenderObjects[mData->mRootMesh].reset();
		mData->mRenderObjects[mData->mRootMesh].reset(new BFG::View::RenderObject
		(
			NULL_HANDLE,
			mData->mRootMesh,
			meshName,
			BFG::v3::ZERO,
			BFG::qv4::IDENTITY
		));

		mData->mMeshNames[mData->mRootMesh] = meshName;
		mData->mActiveMesh = mData->mRootMesh;

		// Create Property Plugin (Space)
		BFG::PluginId spId = BFG::ValueId::ENGINE_PLUGIN_ID;
		boost::shared_ptr<BFG::SpacePlugin> sp(new BFG::SpacePlugin(spId));

		BFG::Property::PluginMapT plugMap;
		plugMap.insert(sp);
		mData->mGameObject = 
			new BFG::GameObject(mLoop, mData->mRootMesh, "Test", plugMap, mData->mEnvironment);
		boost::shared_ptr<BFG::Module> module;
		module.reset(new BFG::Module(mData->mRootMesh));

		std::string adapterName(modDesc->mAdapter->getItemNameAt(modDesc->mAdapter->getIndexSelected()));
		std::vector<BFG::Adapter>& adapterVec = mData->mAdapters[adapterName];

		mData->mGameObject->attachModule(module, adapterVec, 0, NULL_HANDLE, 0);

		mModuleMap[name] = mData->mRootMesh;
	}

	void ModuleControl::addModuleTo(const std::string& parentName)
	{
		SharedData::ConnectionsT::iterator conIt = mData->mConnections.begin();
		for (; conIt != mData->mConnections.end(); ++conIt)
		{
			// find root module
			Connection* conn = (*conIt);
			std::string toName(conn->mTo->getItemNameAt(conn->mTo->getIndexSelected()));
			if (toName == parentName)
			{
				std::string fromName(conn->mFrom->getItemNameAt(conn->mFrom->getIndexSelected()));

				Module* childModule = findModule(fromName);

				BFG::GameHandle moduleHandle = BFG::generateHandle();

				mData->mRenderObjects[moduleHandle].reset();
				mData->mRenderObjects[moduleHandle].reset(new BFG::View::RenderObject
				(
					mModuleMap[parentName],
					moduleHandle,
					childModule->mMesh->getItemNameAt(childModule->mMesh->getIndexSelected()),
					BFG::v3::ZERO,
					BFG::qv4::IDENTITY
				));

				boost::shared_ptr<BFG::Module> module;
				module.reset(new BFG::Module(moduleHandle));

				std::string adapName(childModule->mAdapter->getItemNameAt(childModule->mAdapter->getIndexSelected()));
				std::vector<BFG::Adapter>& adapterVec = mData->mAdapters[adapName];

				mData->mGameObject->attachModule
				(
					module,
					adapterVec,
					MyGUI::utility::parseUInt(conn->mFromAdapter->getCaption()),
					mModuleMap[parentName],
					MyGUI::utility::parseUInt(conn->mToAdapter->getCaption())
				);

				mModuleMap[fromName] = moduleHandle;

				addModuleTo(fromName);
			}
		}
	}

	std::string ModuleControl::findRootModule()
	{
		std::set<std::string> possibleRoots;
		Connection* conn = NULL;
		SharedData::ConnectionsT::iterator conIt = mData->mConnections.begin();
		for (; conIt != mData->mConnections.end(); ++conIt)
		{
			// find root module
			conn = (*conIt);
			possibleRoots.insert(conn->mTo->getItemNameAt(conn->mTo->getIndexSelected()));
		}

		conIt = mData->mConnections.begin();
		for (; conIt != mData->mConnections.end(); ++conIt)
		{
			// find root module
			conn = (*conIt);
			std::string fromStr(conn->mFrom->getItemNameAt(conn->mFrom->getIndexSelected()));
			possibleRoots.erase(fromStr);
		}

		if (possibleRoots.size() > 1)
		{
			throw std::runtime_error("More than one possible roots");
		}

		return (*possibleRoots.begin());
	}

} // namespace Tool
