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

#ifndef BFG_PUGI_XML_TREE__
#define BFG_PUGI_XML_TREE__

#include <core/XmlTree.h>
#include <pugixml.hpp>

namespace BFG {

class PugiXmlTree : public XmlTree
{

public:

	boost::shared_ptr<XmlTree> child(const std::string& name);
	std::vector<boost::shared_ptr<XmlTree> > childList(const std::string& name);
	std::string attribute(const std::string& name);

	boost::shared_ptr<XmlTree> addElement(const std::string& name, const std::string& value);
	void addAttribute(const std::string& name, const std::string& value);
	void addElement(boost::shared_ptr<XmlTree> value);

private:

	PugiXmlTree(pugi::xml_node& node): mXmlNode(node) {}
	PugiXmlTree(const PugiXmlTree& copy)
	{
		mXmlNode = copy.mXmlNode;
	}

	pugi::xml_node mXmlNode;
};

} // namespace BFG

#endif
