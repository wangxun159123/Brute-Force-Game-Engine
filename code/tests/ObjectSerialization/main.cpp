
#include <stdexcept>
#include <string>

#include <tinyxml.h>

#include <Base/CLogger.h>
#include <Core/Path.h>
#include <Model/Loader/XmlObjectSerializer.h>

int main() try
{
	BFG::Path p;
	std::string fullFilePath = "Test.xml";
	
	BFG::Base::Logger::Init(BFG::Base::Logger::SL_DEBUG, "ObjectSerialization.log");
	
	infolog << "Full File Path: " << fullFilePath;
	
	TiXmlDocument document(fullFilePath);
	document.LoadFile();
	TiXmlElement* objectCollection = document.FirstChildElement();

	boost::shared_ptr<BFG::Loader::ObjectSerializer> dummyOs(new BFG::Loader::DummyObjectSerializer);
	boost::shared_ptr<BFG::Loader::ObjectSerializer> xmlOs(new BFG::Loader::XmlObjectSerializer(objectCollection));

	BFG::Loader::ObjectParameterMapT objects;
	dummyOs->read(objects);
	xmlOs->write(objects);
	document.SaveFile();
}
catch (std::exception& ex)
{
	errlog << ex.what();
}
