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


#include <Core/PugiXmlTree.h>

namespace BFG {

	XmlTreeT PugiXmlTree::child(const std::string& name)
	{
		pugi::xml_node node = mXmlNode.child(name.c_str());
		return XmlTreeT(new PugiXmlTree(node));
	}

	XmlTreeListT PugiXmlTree::childList(const std::string& name)
	{
		XmlTreeListT treeList;

		for (pugi::xml_node node = mXmlNode.child(name.c_str()); node; node = node.next_sibling(name.c_str()))
		{
			treeList.push_back(XmlTreeT(new PugiXmlTree(node)));
		}

		return treeList;
	}

	std::string PugiXmlTree::attribute(const std::string& name)
	{
		return mXmlNode.attribute(name.c_str()).value();
	}

    std::string PugiXmlTree::elementData()
    {
        return mXmlNode.text().get();
    }

    XmlTreeT PugiXmlTree::addElement(const std::string& name, const std::string& value)
	{
		pugi::xml_node node = mXmlNode.append_child();
		node.set_name(name.c_str());
		node.set_value(value.c_str());

		return boost::shared_ptr<XmlTree>(new PugiXmlTree(node));
	}

	void PugiXmlTree::addAttribute(const std::string& name, const std::string& value)
	{
		mXmlNode.append_attribute(name.c_str()).set_value(value.c_str());
	}

	void PugiXmlTree::addElement(XmlTreeT value)
	{
		//tbd
	}

} // namespace BFG
