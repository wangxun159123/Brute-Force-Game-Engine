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


BOOST_AUTO_TEST_CASE (loadFile)
{
	Base::Logger::Init(Base::Logger::SL_DEBUG, "Logs/LoaderTest.log");

	Path path;
	std::string xml_test_file_path = "";
	xml_test_file_path = path.Expand("LoaderTest.xml");

	boost::shared_ptr<XmlFileHandle> fileHandle;
	fileHandle.reset(new PugiXmlFileHandle(xml_test_file_path));

	

}






//BOOST_AUTO_TEST_CASE (distance)
//{
//	BFG::v3 a(-5.2, 3.8, 4.8);
//	BFG::v3 b(8.7, -4.1, 9.1);

//	BFG::f32 result = BFG::distance(a,b);
//	BFG::f32 expected = 16.5563f;

//	const BFG::f32 ERROR_MARGIN = 0.0001f;

//	bool resultCorrect = std::fabs(result - expected) < ERROR_MARGIN;

//	BOOST_REQUIRE (resultCorrect);
//}

//BOOST_AUTO_TEST_CASE (nearEnough)
//{
//	// distance between `a' and `b' is 16.5563f
//	BFG::v3 a(-5.2, 3.8, 4.8);
//	BFG::v3 b(8.7, -4.1, 9.1);

//	bool resultNull = BFG::nearEnough(a, b, 0.0);
//	bool resultLower = BFG::nearEnough(a, b, 16.5);
//	bool resultUpper = BFG::nearEnough(a, b, 16.6);

//	BOOST_REQUIRE (resultNull == false);
//	BOOST_REQUIRE (resultUpper == true);
//	BOOST_REQUIRE (resultLower == false);
//}

}
}
