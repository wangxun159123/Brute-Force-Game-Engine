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

#ifndef MODULE_H
#define MODULE_H

#include <boost/shared_ptr.hpp>

#include <MyGUI.h>
#include <boost/filesystem.hpp>

#include <Core/Path.h>
#include <Core/XmlTree.h>

namespace Tool
{

class Module
{
public:
	typedef MyGUI::delegates::IDelegate1<MyGUI::Widget*>* ButtonClickHandler;

	Module(MyGUI::ScrollView* parent, 
	       std::map<std::string, std::vector<BFG::Adapter> >& adapterMap,
	       ButtonClickHandler closeHandler) :
	mAdapters(adapterMap),
	mParent(parent)
	{
		MyGUI::LayoutManager& layMan = MyGUI::LayoutManager::getInstance();
		
		BFG::s32 itemCount = mParent->getChildCount();

		mContainer = layMan.loadLayout("Module.layout", "", mParent);

		MyGUI::Widget* panel = mContainer.front();

		BFG::s32 width = mParent->getCanvasSize().width;
		BFG::s32 y = itemCount * panel->getHeight();
		panel->setPosition(0, y);

		mParent->setCanvasSize(width, y + panel->getHeight());

		mName = panel->findWidget("name")->castType<MyGUI::EditBox>();
		mMesh = panel->findWidget("mesh")->castType<MyGUI::ComboBox>();
		mAdapter = panel->findWidget("adapter")->castType<MyGUI::ComboBox>();
		mClose = panel->findWidget("close")->castType<MyGUI::Button>();

		mName->eventEditSelectAccept +=
			MyGUI::newDelegate(this, &Module::onNameSet);

		mMesh->eventComboChangePosition +=
			MyGUI::newDelegate(this, &Module::onMeshSelected);
		mMesh->setEnabled(false);

		mAdapter->eventComboChangePosition +=
			MyGUI::newDelegate(this, &Module::onAdapterSelected);
		mAdapter->setEnabled(false);

		mClose->eventMouseButtonClick += closeHandler;
		mClose->setUserData(this);
	}

	~Module()
	{
		MyGUI::LayoutManager& layMan = MyGUI::LayoutManager::getInstance();
		layMan.unloadLayout(mContainer);

		for (size_t i = 0; i < mParent->getChildCount(); ++i)
		{
			MyGUI::Widget* child = mParent->getChildAt(i);
			child->setPosition(0, i * child->getSize().height);
		}
	}

	BFG::XmlTreeT toXml(BFG::XmlTreeT parentNode) const
	{
		BFG::XmlTreeT module = parentNode->addElement("Module");
		module->addAttribute("name", mName->getCaption());
		module->addElement("Mesh", mMesh->getCaption());
		module->addElement("Adapters", mAdapter->getCaption());
		return module;
	}

	void fromXml(BFG::XmlTreeT moduleNode)
	{
		std::string name(moduleNode->attribute("name"));
		BFG::XmlTreeT meshNode = moduleNode->child("Mesh");
		std::string mesh(meshNode->elementData());
		BFG::XmlTreeT adapterNode = moduleNode->child("Adapters");
		std::string adapter(adapterNode->elementData());

		mName->setCaption(name);
		onNameSet(mName);

		size_t index = mMesh->findItemIndexWith(mesh);
		if (index == MyGUI::ITEM_NONE)
			throw std::runtime_error("Mesh " + mesh + " not found!");
		mMesh->setIndexSelected(index);

		index = mAdapter->findItemIndexWith(adapter);
		if (index == MyGUI::ITEM_NONE)
			throw std::runtime_error("Adapter " + adapter + " not found!");
		mAdapter->setIndexSelected(index);
	}

	MyGUI::EditBox* mName;
	MyGUI::ComboBox* mMesh;
	MyGUI::ComboBox* mAdapter;

private:

	void onNameSet(MyGUI::EditBox*)
	{
		BFG::Path p;
		std::string location(p.Get(BFG::ID::P_GRAPHICS_MESHES));
		
		boost::filesystem::directory_iterator it(location), end;
		for (; it != end; ++it)
		{
			if (boost::filesystem::is_directory(*it))
				continue;

			const std::string directoryName = it->path().filename().string();
			mMesh->addItem(it->path().filename().string());
		}
		mMesh->setEnabled(true);

		std::map<std::string, std::vector<BFG::Adapter> >::iterator adapterIt = 
			mAdapters.begin();

		for (; adapterIt != mAdapters.end(); ++adapterIt)
		{
			mAdapter->addItem(adapterIt->first);
		}
		mAdapter->setEnabled(true);
	}

	void onMeshSelected(MyGUI::ComboBox*, size_t)
	{}

	void onAdapterSelected(MyGUI::ComboBox*, size_t)
	{}

	std::map<std::string, std::vector<BFG::Adapter> > mAdapters;

	MyGUI::VectorWidgetPtr mContainer;

	MyGUI::ScrollView* mParent;
	MyGUI::Button* mClose;
};
} // namespace Tool

#endif