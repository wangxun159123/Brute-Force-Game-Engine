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

#ifndef CONNECTION_H
#define CONNECTION_H

#include <MyGUI.h>
#include <map>
#include <vector>
#include <tinyxml.h>

#include <Model/Adapter.h>

#include <Module.h>

namespace Tool
{
class Connection
{
public:
	typedef MyGUI::delegates::IDelegate1<MyGUI::Widget*>* ButtonClickHandler;

	Connection(MyGUI::ScrollView* parent,
	           std::vector<Module*>& modules,
	           std::map<std::string, std::vector<BFG::Adapter> >& adapters,
	           ButtonClickHandler closeHandler) :
	mModules(modules),
	mAdapters(adapters),
	mParent(parent)
	{
		MyGUI::LayoutManager& layMan = MyGUI::LayoutManager::getInstance();

		BFG::s32 itemCount = mParent->getChildCount();

		mContainer = layMan.loadLayout("Connection.layout", "", mParent);

		MyGUI::Widget* panel = mContainer.front();

		BFG::s32 width = mParent->getCanvasSize().width;
		BFG::s32 y = itemCount * panel->getHeight();
		panel->setPosition(0, y);

		mParent->setCanvasSize(width, y + panel->getHeight());

		mFrom = panel->findWidget("from")->castType<MyGUI::ComboBox>();
		mFrom->eventComboChangePosition += MyGUI::newDelegate(this, &Connection::onFromChanged);

		mFromAdapter = panel->findWidget("fromAdapter")->castType<MyGUI::ComboBox>();
		mFromAdapter->eventComboChangePosition += MyGUI::newDelegate(this, &Connection::onFromAdapterChanged);
		mFromAdapter->setEnabled(false);

		mTo = panel->findWidget("to")->castType<MyGUI::ComboBox>();
		mTo->eventComboChangePosition += MyGUI::newDelegate(this, &Connection::onToChanged);

		mToAdapter = panel->findWidget("toAdapter")->castType<MyGUI::ComboBox>();
		mToAdapter->eventComboChangePosition += MyGUI::newDelegate(this, &Connection::onToAdapterChanged);
		mToAdapter->setEnabled(false);

		mClose = panel->findWidget("close")->castType<MyGUI::Button>();
		mClose->eventMouseButtonClick = closeHandler;
		mClose->setUserData(this);

		std::vector<Module*>::iterator modIt = mModules.begin();
		for (; modIt != mModules.end(); ++modIt)
		{
			Module* module = (*modIt);
			mFrom->addItem(module->mName->getCaption());
			mTo->addItem(module->mName->getCaption());
		}
	}

	~Connection()
	{
		MyGUI::LayoutManager& layMan = MyGUI::LayoutManager::getInstance();
		layMan.unloadLayout(mContainer);

		for (size_t i = 0; i < mParent->getChildCount(); ++i)
		{
			MyGUI::Widget* child = mParent->getChildAt(i);
			child->setPosition(0, i * child->getSize().height);
		}
	}

	void toXml(TiXmlElement* xmlElement) const
	{
		xmlElement->SetAttribute
		(
			"connection",
			std::string
			(
				mFromAdapter->getCaption() + "@" + 
				mTo->getCaption() + ":" +
				mToAdapter->getCaption()
			)
		);
	}

	void fromXml(TiXmlElement* xmlElement)
	{
		std::string name(xmlElement->Attribute("name"));

		size_t index = mFrom->findItemIndexWith(name);
		if (index == MyGUI::ITEM_NONE)
			throw std::runtime_error("Module " + name + " not found!");
		mFrom->setIndexSelected(index);
		onFromChanged(mFrom, index);

		std::string connection(xmlElement->Attribute("connection"));
		if (connection == "")
			throw std::runtime_error("Connection string empty!");
		
		size_t atPos = connection.find('@');
		if (atPos == std::string::npos)
			throw std::runtime_error("No '@' found in connection");
		
		size_t colPos = connection.find(':', atPos);
		if (colPos == std::string::npos)
			throw std::runtime_error("No ':' found in connection");
		
		std::string fromAdapter(connection.substr(0, atPos));
		std::string to(connection.substr(atPos + 1, colPos - (atPos + 1)));
		std::string toAdapter(connection.substr(colPos + 1, connection.length() - (colPos + 1)));

		index = mFromAdapter->findItemIndexWith(fromAdapter);
		if (index == MyGUI::ITEM_NONE)
			throw std::runtime_error("From-Adapter " + fromAdapter + " not found!");
		mFromAdapter->setIndexSelected(index);

		index = mTo->findItemIndexWith(to);
		if (index == MyGUI::ITEM_NONE)
			throw std::runtime_error("Module " + to + " not found!");
		mTo->setIndexSelected(index);
		onToChanged(mTo, index);

		index = mToAdapter->findItemIndexWith(toAdapter);
		if (index == MyGUI::ITEM_NONE)
			throw std::runtime_error("To-Adapter " + toAdapter + " not found!");
		mToAdapter->setIndexSelected(index);
	}

	MyGUI::ComboBox* mFrom;
	MyGUI::ComboBox* mTo;
	MyGUI::ComboBox* mFromAdapter;
	MyGUI::ComboBox* mToAdapter;

private:

	void onFromChanged(MyGUI::ComboBox* sender, size_t index)
	{
		if (index != MyGUI::ITEM_NONE)
		{
			std::string moduleName(sender->getItemNameAt(index));

			Module* module = NULL;
			std::vector<Module*>::iterator modIt = mModules.begin();
			for (; modIt != mModules.end(); ++modIt)
			{
				module = (*modIt);
				std::string modName(module->mName->getCaption());
				if (modName == moduleName)
					break;
			}

			if (modIt != mModules.end())
			{
				size_t adapIndex = module->mAdapter->getIndexSelected();
				std::string adapName(module->mAdapter->getItemNameAt(adapIndex));
				std::vector<BFG::Adapter> & adapter = mAdapters[adapName];
				std::vector<BFG::Adapter>::iterator adapIt = adapter.begin();
				for (; adapIt != adapter.end(); ++adapIt)
				{
					mFromAdapter->addItem(MyGUI::utility::toString((*adapIt).mIdentifier));
				}
				mFromAdapter->setEnabled(true);
			}
		}
	}

	void onToChanged(MyGUI::ComboBox* sender, size_t index)
	{
		if (index != MyGUI::ITEM_NONE)
		{
			std::string moduleName(sender->getItemNameAt(index));

			Module* module = NULL;
			std::vector<Module*>::iterator modIt = mModules.begin();
			for (; modIt != mModules.end(); ++modIt)
			{
				module = (*modIt);
				std::string modName(module->mName->getCaption());
				if (modName == moduleName)
					break;
			}

			if (modIt != mModules.end())
			{
				size_t adapIndex = module->mAdapter->getIndexSelected();
				std::string adapName(module->mAdapter->getItemNameAt(adapIndex));
				std::vector<BFG::Adapter> & adapter = mAdapters[adapName];
				std::vector<BFG::Adapter>::iterator adapIt = adapter.begin();
				for (; adapIt != adapter.end(); ++adapIt)
				{
					mToAdapter->addItem(MyGUI::utility::toString((*adapIt).mIdentifier));
				}
				mToAdapter->setEnabled(true);
			}
		}
	}

	void onFromAdapterChanged(MyGUI::ComboBox* sender, size_t index)
	{
		if (index != MyGUI::ITEM_NONE)
		{

		}
	}

	void onToAdapterChanged(MyGUI::ComboBox* sender, size_t index)
	{
		if (index != MyGUI::ITEM_NONE)
		{
		}
	}

	std::vector<Module*> mModules;
	std::map<std::string, std::vector<BFG::Adapter> > mAdapters;

	MyGUI::VectorWidgetPtr mContainer;

	MyGUI::ScrollView* mParent;
	MyGUI::Button* mClose;

};
}

#endif
