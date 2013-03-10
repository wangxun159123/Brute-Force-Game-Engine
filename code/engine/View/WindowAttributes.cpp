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

#include <View/WindowAttributes.h>

#include <stdexcept>
#include <OgreRoot.h>
#include <OgreRenderWindow.h>

namespace BFG {
namespace View {

VIEW_API void queryWindowAttributes(WindowAttributes& wa)
{
	Ogre::Root* root = Ogre::Root::getSingletonPtr();
	
	if (! root)
		throw std::logic_error
			("View::windowAttributes: Ogre hasn't been initialized yet.");
	
	Ogre::RenderWindow* win = root->getAutoCreatedWindow();
	if (! win)
		throw std::logic_error
			("View::windowAttributes: Ogre hasn't created the window yet.");

	win->getCustomAttribute("WINDOW", &wa.mHandle);
	wa.mHeight = win->getHeight();
	wa.mWidth = win->getWidth();
}

} // namespace View
} // namespace BFG