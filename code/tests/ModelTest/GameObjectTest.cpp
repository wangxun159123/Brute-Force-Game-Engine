#include <Model/GameObject.h>

#define BOOST_TEST_MODULE GameObjectTest
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (VectorToModuleFromRootTest)
{
	// Absolut random values
	BFG::Adapter a;
	a.mParentPosition = v3(0.1f, 0.2f, 0.3f);
	a.mChildPosition = v3(0.4f, 0.5f, 0.6f);
	a.mParentOrientation = BFG::qv4(0.707, 0, 0.707, 0);
	a.mChildOrientation = BFG::qv4(0.707, 0, 0, 0.707);

	BFG::Adapter b;
	b.mParentPosition = v3(0.7f,0.8f,0.9f);
	b.mChildPosition = v3(1.0f,1.1f,1.2f);
	b.mParentOrientation = BFG::qv4(0.910, 0.109, 0.260, 0.305);
	b.mChildOrientation = BFG::qv4(0.942, 0.260, 0.209, 0.0363);

	BFG::Adapter c;
	c.mParentPosition = v3(1.3f,1.4f,1.5f);
	c.mChildPosition = v3(1.6f,1.7f,1.8f);
	c.mParentOrientation = BFG::qv4(0.952, 0.239, 0.189, 0.0381);
	c.mChildOrientation = BFG::qv4(0.764, 0.570, 0.264, -0.150);

	std::vector<BFG::Adapter> adapters;
	adapters.push_back(a);
	adapters.push_back(b);
	adapters.push_back(c);

	BFG::v3 position;
	BFG::qv4 orientation;

	BFG::vectorToModuleFromRoot(adapters, position, orientation);

	// Precalculated with the working algorithm
	BFG::v3 expectedPos(0.9556f, 3.87371f, 0.599187f);
	BFG::qv4 expectedOri(0.221768f, 0.63988f, -0.017174f, 0.735102f);

	const BFG::f32 ERROR_MARGIN = 0.0001f;

	bool xPosCorrect = std::fabs(position.x - expectedPos.x) < ERROR_MARGIN;
	bool yPosCorrect = std::fabs(position.y - expectedPos.y) < ERROR_MARGIN;
	bool zPosCorrect = std::fabs(position.z - expectedPos.z) < ERROR_MARGIN;
	bool positionCorrect = xPosCorrect && yPosCorrect && zPosCorrect;

	bool wOriCorrect = std::fabs(orientation.w - expectedOri.w) < ERROR_MARGIN;
	bool xOriCorrect = std::fabs(orientation.x - expectedOri.x) < ERROR_MARGIN;
	bool yOriCorrect = std::fabs(orientation.y - expectedOri.y) < ERROR_MARGIN;
	bool zOriCorrect = std::fabs(orientation.z - expectedOri.z) < ERROR_MARGIN;
	bool orientationCorrect = wOriCorrect && xOriCorrect && yOriCorrect && zOriCorrect;

	BOOST_REQUIRE (positionCorrect && orientationCorrect);
}

