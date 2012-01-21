#include <stdexcept>
#include <iostream>
#include <string>

#include <tinyxml.h>

#include <Base/CLogger.h>
#include <Core/Path.h>
#include <Model/Loader/XmlObjectSerializer.h>
#include <Model/Loader/XmlSectorSerializer.h>

int main() try
{
	BFG::Base::Logger::Init(BFG::Base::Logger::SL_DEBUG, "SerializationTest.log");

	// Find the test file
	BFG::Path p;
	std::string fullFilePath = p.Expand("SerializationTest.xml");
	infolog << "Full File Path: " << fullFilePath;

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
	boost::shared_ptr<BFG::Loader::ObjectSerializer> dummyOs(new BFG::Loader::DummyObjectSerializer);
	dummyOs->write(sp.mObjects);
}
catch (std::exception& ex)
{
	std::cout << ex.what() << std::endl;
}
