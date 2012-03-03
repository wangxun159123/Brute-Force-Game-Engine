#include <stdexcept>
#include <iostream>
#include <string>

#include <tinyxml.h>

#include <Base/CLogger.h>
#include <Core/Path.h>
#include <Core/qv4.h>
#include <Model/Loader/DummyObjectSerializer.h>
#include <Model/Loader/XmlObjectSerializer.h>
#include <Model/Loader/XmlSectorSerializer.h>
#include <Model/Loader/XmlAdapterSerializer.h>

namespace Test {

namespace Elements
{
	const std::string root = "root";
}

void readSector(const std::string& fullFilePath)
{
	// Open the test file
	TiXmlDocument document(fullFilePath);
	document.LoadFile();

	// Load whole sector
	TiXmlElement* sector = document.FirstChildElement();
	boost::shared_ptr<BFG::Loader::SectorSerializer> xmlSs(new BFG::Loader::XmlSectorSerializer(sector));
	BFG::Loader::SectorParameter sp;
	xmlSs->read(sp);

	// Test output
	infolog << "Name: " << sp.mName;
	boost::shared_ptr<BFG::Loader::ObjectListSerializer> dummyOs(new BFG::Loader::DummyObjectListSerializer);
	dummyOs->write(sp.mObjects);
}

void writeSector(const std::string& fullFilePath)
{
	// Open the test file
	TiXmlDocument document(fullFilePath);
	document.LoadFile();

	// Create a sector
	BFG::Loader::SectorParameter sp;
	sp.mName = "My Sector";

	// Read some test data (from memory)
	boost::shared_ptr<BFG::Loader::ObjectListSerializer> dummyOs(new BFG::Loader::DummyObjectListSerializer);
	dummyOs->read(sp.mObjects);

	// Write test data as XML
	boost::shared_ptr<BFG::Loader::SectorSerializer> xmlSs(new BFG::Loader::XmlSectorSerializer(&document));
	xmlSs->write(sp);

	document.SaveFile();
}

void readAdapterConfig(const std::string& fullFilePath)
{
	// Open the test file
	TiXmlDocument document(fullFilePath);
	document.LoadFile();

	TiXmlElement* root = document.FirstChildElement("root");
	TiXmlElement* adapterConfig = root->FirstChildElement("AdapterConfig");

	BFG::Loader::XmlAdapterConfigSerializer xacs(adapterConfig);
	BFG::Loader::AdapterConfigT ac;
	xacs.read(ac);
	
	BFG::Loader::AdapterConfigT::second_type::const_iterator it = ac.second.begin();

	infolog << ac.first;	
	infolog << "==============";	
	for (; it != ac.second.end(); ++it)
	{
		infolog << "Identifier: " << it->mIdentifier;
		infolog << "Parent Position: " << it->mParentPosition;
		infolog << "Parent Orientation: " << it->mParentOrientation;
	}
}

void writeAdapterConfig(const std::string& fullFilePath)
{
	// Open the test file
	TiXmlDocument document(fullFilePath);
	document.LoadFile();

	// Make a <root> node
	TiXmlElement* root = new TiXmlElement(Elements::root);
	document.LinkEndChild(root);

	// Prepare Test Data
	BFG::Loader::AdapterConfigT ac;
	ac.first = "TestAdapters";

	BFG::Adapter a;
	BFG::Adapter b;

	a.mIdentifier = 123;
	a.mParentPosition = v3::NEGATIVE_UNIT_Z;
	a.mParentOrientation = BFG::qv4(1.0, 0.1, 0.2, 0.3);

	b.mIdentifier = 77777;
	b.mParentPosition = v3(1.0f,0.9f,0.8f);
	b.mParentOrientation = BFG::qv4(0.0, 1.0, 0.5, 0.1);
	
	ac.second.push_back(a);
	ac.second.push_back(b);
	
	BFG::Loader::XmlAdapterConfigSerializer xacs(root);
	xacs.write(ac);
	
	document.SaveFile();
}

} // namespace Test

int main() try
{
	BFG::Base::Logger::Init(BFG::Base::Logger::SL_DEBUG, "SerializationTest.log");

	// Find the test file
	BFG::Path p;
	std::string fullFilePath = p.Expand("SerializationTest.xml");
	infolog << "Full File Path: " << fullFilePath;
	
	Test::writeSector(fullFilePath);
	Test::readSector(fullFilePath);	
	
	Test::writeAdapterConfig(fullFilePath);
	Test::readAdapterConfig(fullFilePath);
}
catch (std::exception& ex)
{
	std::cout << ex.what() << std::endl;
}
