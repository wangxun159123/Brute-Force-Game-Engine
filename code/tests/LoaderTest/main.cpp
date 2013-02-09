#include <boost/shared_ptr.hpp>

#include <Base/Logger.h>
#include <Core/XmlFileHandleFactory.h>
#include <Core/Math.h>
#include <Core/Path.h>

#define BOOST_TEST_MODULE LoaderTest
#include <boost/test/unit_test.hpp>

#include <Model/Data/AdapterXml.h>
#include <Model/Data/ValueXml.h>
#include <Model/Data/ConceptXml.h>

namespace BFG {
namespace Test {

BOOST_AUTO_TEST_CASE (loadFile)
{
    BOOST_TEST_MESSAGE("Test loading of LoaderTest.xml");

    Base::Logger::Init(Base::Logger::SL_DEBUG, "Logs/LoaderTest.log");

	Path path;
	std::string xml_test_file_path = "";
	BOOST_REQUIRE_NO_THROW (xml_test_file_path = path.Expand("LoaderTest.xml"));

	XmlFileHandleT fileHandle;
	BOOST_REQUIRE_NO_THROW(fileHandle = XmlFileHandleFactory::createWithPugiXml(xml_test_file_path));
	BOOST_REQUIRE (fileHandle.get());

	BOOST_REQUIRE (fileHandle->root().get());
}


BOOST_AUTO_TEST_CASE (EditAndSaveFile)
{
	BOOST_TEST_MESSAGE("Function XmlTree::elementList(..) and XmlTree::child(...) .");

	Path path;
	std::string xml_test_file_path = "";
	xml_test_file_path = path.Expand("LoaderTest.xml");

	XmlFileHandleT fileHandle = XmlFileHandleFactory::createWithPugiXml(xml_test_file_path);

	XmlTreeT root;
	root = fileHandle->root()->child("root");
	XmlTreeListT elements;

	XmlTreeT newElement;
	BOOST_REQUIRE_NO_THROW(newElement = root->addElement("AddedElement", "A new element was born!"));
	BOOST_REQUIRE(newElement);
	BOOST_REQUIRE_NO_THROW(fileHandle->save());

	BOOST_REQUIRE_NO_THROW(newElement->editElementData("A new element was born! Edited"));
	BOOST_REQUIRE_NO_THROW(fileHandle->save());

	BOOST_REQUIRE_NO_THROW(newElement->addAttribute("AtrName", "AtrValue"));
	BOOST_REQUIRE_NO_THROW(fileHandle->save());

	BOOST_REQUIRE_NO_THROW(newElement->editAttributeData("AtrName", "AtrValueEdited"));
	BOOST_REQUIRE_NO_THROW(fileHandle->save());
	
	BOOST_REQUIRE_NO_THROW(newElement->removeAttribute("AtrName"));
	BOOST_REQUIRE_NO_THROW(fileHandle->save());

	BOOST_REQUIRE_NO_THROW(root->removeElement("AddedElement"));
	BOOST_REQUIRE_NO_THROW(fileHandle->save());
}


BOOST_AUTO_TEST_CASE (SaveVector)
{
	BOOST_TEST_MESSAGE("Function XmlTree::elementList(..) and XmlTree::child(...) .");

	Path path;
	std::string xml_test_file_path = "";
	xml_test_file_path = path.Expand("LoaderTest.xml");

	XmlFileHandleT fileHandle = XmlFileHandleFactory::createWithPugiXml(xml_test_file_path);

	XmlTreeT root;
	root = fileHandle->root()->child("root");
	XmlTreeListT elements;

	elements = root->childList("element");

	XmlTreeT vectorElement = elements[0]->addElement("vector");
	v3 vec(5,4,3);
	BOOST_REQUIRE_NO_THROW(saveVector3(vec, vectorElement));
	BOOST_REQUIRE_NO_THROW(fileHandle->save());

	fileHandle = XmlFileHandleFactory::createWithPugiXml(xml_test_file_path);
	root = fileHandle->root()->child("root");
	elements = root->childList("element");

	XmlTreeT vectorElementOut = elements[0]->child("vector");
	v3 vecOut = loadVector3(vectorElementOut);

	BOOST_REQUIRE(nearEnough(vecOut, vec, EPSILON_F));

	elements[0]->removeElement("vector");
	BOOST_REQUIRE_NO_THROW(fileHandle->save());
}


BOOST_AUTO_TEST_CASE (elementList)
{
	BOOST_TEST_MESSAGE("Function XmlTree::elementList(..) and XmlTree::child(...) .");

	Path path;
	std::string xml_test_file_path = "";
	xml_test_file_path = path.Expand("LoaderTest.xml");

	XmlFileHandleT fileHandle = XmlFileHandleFactory::createWithPugiXml(xml_test_file_path);

	XmlTreeT searchHandle;
	BOOST_REQUIRE_NO_THROW(searchHandle = fileHandle->root()->child("root"));
	XmlTreeListT elements;

	BOOST_REQUIRE_NO_THROW(elements = searchHandle->childList("element"));
	BOOST_REQUIRE_EQUAL(elements.size(), 3);
}


BOOST_AUTO_TEST_CASE (attribute)
{
	BOOST_TEST_MESSAGE("Function XmlTree::attribute(...) .");

	Path path;
	std::string xml_test_file_path = "";
	xml_test_file_path = path.Expand("LoaderTest.xml");

	XmlFileHandleT fileHandle = XmlFileHandleFactory::createWithPugiXml(xml_test_file_path);

	XmlTreeT searchHandle;
	searchHandle = fileHandle->root()->child("root");
	XmlTreeListT elements;
	elements = searchHandle->childList("element");

	BOOST_REQUIRE_NO_THROW
	(
		for(u32 i = 0; i < elements.size(); ++i)
		{
			if (elements[i]->attribute("name") == "Element2")
			{
				searchHandle = elements[i];
				break;
			}
		}
	);

	BOOST_REQUIRE (searchHandle);
	BOOST_REQUIRE_EQUAL(searchHandle->attribute("name"), "Element2");

	XmlTreeListT subElements = searchHandle->childList("subelement");

	BOOST_REQUIRE_EQUAL(subElements[0]->attribute("name"), "Subelement1");

	std::string result;
	result = subElements[0]->child("Data")->elementData();

	BOOST_REQUIRE_NE(result, "");
	BOOST_REQUIRE_EQUAL(result, "This is a data block!");
}

BOOST_AUTO_TEST_CASE (defaultAdapter)
{
	Path path;
	std::string xml_test_file_path = "";
	xml_test_file_path = path.Get(ID::P_SCRIPTS_LEVELS)+"/default/Adapter.xml";

	XmlFileHandleT fileHandle = XmlFileHandleFactory::createWithPugiXml(xml_test_file_path);
	BOOST_REQUIRE_NO_THROW
	(
		AdapterXml factory = AdapterXml(fileHandle);
		AdapterConfigT acp = factory.create("CubeAllAdapters")
	);
}

BOOST_AUTO_TEST_CASE (defaultValuesTest)
{
	Path path;
	std::string xml_test_file_path = "";
	xml_test_file_path = path.Get(ID::P_SCRIPTS_LEVELS)+"/default/Value.xml";

	XmlFileHandleT fileHandle = XmlFileHandleFactory::createWithPugiXml(xml_test_file_path);
	BOOST_REQUIRE_NO_THROW
	(
		ValueXml factory = ValueXml(fileHandle);
		PropertyConfigT vcp = factory.create("ExplodeOnContact")
	);
}


BOOST_AUTO_TEST_CASE (defaultConceptsTest)
{
	Path path;
	std::string xml_test_file_path = "";
	xml_test_file_path = path.Get(ID::P_SCRIPTS_LEVELS)+"/default/Concept.xml";

	XmlFileHandleT fileHandle = XmlFileHandleFactory::createWithPugiXml(xml_test_file_path);
	BOOST_REQUIRE_NO_THROW
	(
		ConceptXml factory = ConceptXml(fileHandle);
		ConceptConfigT vcp = factory.create("CubeConcept")
	);
}




} // BFG
} // Test
