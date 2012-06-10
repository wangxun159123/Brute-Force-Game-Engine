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

#include "BaseFeature.h"

namespace Tool
{

	void BaseFeature::onButtonClicked( MyGUI::Widget* )
	{
		if (!isLoaded())
			load();

		if (isActive())
			deactivate();
		else
			activate();
	}

	void BaseFeature::createButton()
	{
		MyGUI::Widget* menuBox = mGui->findWidgetT("MenuBox");
		if (!menuBox)
			throw std::runtime_error("MenuBox not found!");

		int xPos = 0;
		size_t count = menuBox->getChildCount();
		for (size_t i = 0; i < count; ++i)
		{
			MyGUI::Widget* w = menuBox->getChildAt(i);

			xPos += w->getWidth();
		}

		mMenuButton = menuBox->createWidget<MyGUI::Button>
			("Button", xPos, 0, 24, 24, MyGUI::Align::Default);

		mMenuButton->setCaption(mName);
		MyGUI::IntSize textSize = mMenuButton->getTextSize();
		mMenuButton->setSize(textSize.width + 4, 24);
		mMenuButton->eventMouseButtonClick += 
			MyGUI::newDelegate(this, &BaseFeature::onButtonClicked);
	}

	void BaseFeature::destroyButton()
	{
		if (!mMenuButton)
			return;

		MyGUI::ItemBox* itemBox = mGui->findWidget<MyGUI::ItemBox>("MenuBox");
		if (!itemBox)
			return;

		size_t index = itemBox->getIndexByWidget(mMenuButton);
		itemBox->removeItemAt(index);

		mGui->destroyWidget(mMenuButton);
		mMenuButton = NULL;
	}

}
