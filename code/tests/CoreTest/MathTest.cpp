#include <Core/Math.h>

#define BOOST_TEST_MODULE MathTest
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (distance)
{
	BFG::v3 a(-5.2, 3.8, 4.8);
	BFG::v3 b(8.7, -4.1, 9.1);

	BFG::f32 result = BFG::distance(a,b);
	BFG::f32 expected = 16.5563f;

	const BFG::f32 ERROR_MARGIN = 0.0001f;

	bool resultCorrect = std::fabs(result - expected) < ERROR_MARGIN;

	BOOST_REQUIRE (resultCorrect);
}
