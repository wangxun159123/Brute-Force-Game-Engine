//#include <Core/Math.h>

#include <boost/shared_ptr.hpp>

#include <Base/Logger.h>
#include <Core/PugiXmlFileHandle.h>
#include <Core/Path.h>


#define BOOST_TEST_MODULE LoaderTest
#include <boost/test/unit_test.hpp>


namespace BFG {
namespace Test {

BOOST_AUTO_TEST_CASE (loadFile)
{
	Base::Logger::Init(Base::Logger::SL_DEBUG, "Logs/LoaderTest.log");

	Path path;
	std::string xml_test_file_path = "";
	BOOST_REQUIRE_NO_THROW (xml_test_file_path = path.Expand("LoaderTest.xml"));

	boost::shared_ptr<XmlFileHandle> fileHandle;
	BOOST_REQUIRE_NO_THROW(fileHandle.reset(new PugiXmlFileHandle(xml_test_file_path)));
	BOOST_REQUIRE (fileHandle.get());

	BOOST_REQUIRE (fileHandle->root().get());
}


BOOST_AUTO_TEST_CASE (elementList)
{
    //Base::Logger::Init(Base::Logger::SL_DEBUG, "Logs/LoaderTest.log");

    Path path;
    std::string xml_test_file_path = "";
    xml_test_file_path = path.Expand("LoaderTest.xml");

    boost::shared_ptr<XmlFileHandle> fileHandle;
    fileHandle.reset(new PugiXmlFileHandle(xml_test_file_path));

    XmlTreeT searchHandle;
    BOOST_REQUIRE_NO_THROW(searchHandle = fileHandle->root()->child("root"));
    XmlTreeListT elements;

    BOOST_REQUIRE_NO_THROW(elements = searchHandle->childList("element"));

    BOOST_REQUIRE_EQUAL(elements.size(), 3);
}

}
}
