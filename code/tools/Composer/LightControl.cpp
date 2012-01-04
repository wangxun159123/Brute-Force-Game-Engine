/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2011 Brute-Force Games GbR

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

#include <LightControl.h>

#include <OgreRoot.h>

#include <MyGUI_ComboBox.h>
#include <MyGUI_Button.h>

#include <Core/Types.h>
#include <Core/Utils.h>
#include <View/Convert.h>
#include <View/LightCreation.h>


namespace Tool
{
using BFG::f32;
using BFG::v3;
using BFG::cv4;

void LightControl::load()
{
	if (mLoaded)
		return;

	MyGUI::LayoutManager* layMan = MyGUI::LayoutManager::getInstancePtr();
	mContainer = layMan->loadLayout("Light.layout");

	mDiffuse = mGui->findWidget<MyGUI::EditBox>("diffuse");
	mDiffuse->eventEditTextChange += 
		MyGUI::newDelegate(this, &LightControl::onDiffuseChange);

	mSpecular = mGui->findWidget<MyGUI::EditBox>("specular");
	mSpecular->eventEditTextChange +=
		MyGUI::newDelegate(this, &LightControl::onSpecularChange);

	mPower = mGui->findWidget<MyGUI::EditBox>("power");
	mPosition = mGui->findWidget<MyGUI::EditBox>("position");
	mDirection = mGui->findWidget<MyGUI::EditBox>("direction");
	mRange = mGui->findWidget<MyGUI::EditBox>("range");
	mConstant = mGui->findWidget<MyGUI::EditBox>("constant");
	mLinear = mGui->findWidget<MyGUI::EditBox>("linear");
	mQuadric = mGui->findWidget<MyGUI::EditBox>("quadric");
	mFalloff = mGui->findWidget<MyGUI::EditBox>("falloff");
	mInnerRadius = mGui->findWidget<MyGUI::EditBox>("innerRadius");
	mOuterRadius = mGui->findWidget<MyGUI::EditBox>("outerRadius");

	mDif = mGui->findWidget<MyGUI::TextBox>("tDif");
	mSpe = mGui->findWidget<MyGUI::TextBox>("tSpe");
	mPow = mGui->findWidget<MyGUI::TextBox>("tPow");
	mPos = mGui->findWidget<MyGUI::TextBox>("tPos");
	mDir = mGui->findWidget<MyGUI::TextBox>("tDir");
	mRan = mGui->findWidget<MyGUI::TextBox>("tRan");
	mCon = mGui->findWidget<MyGUI::TextBox>("tCon");
	mLin = mGui->findWidget<MyGUI::TextBox>("tLin");
	mQua = mGui->findWidget<MyGUI::TextBox>("tQua");
	mFal = mGui->findWidget<MyGUI::TextBox>("tFal");
	mInn = mGui->findWidget<MyGUI::TextBox>("tInn");
	mOut = mGui->findWidget<MyGUI::TextBox>("tOut");

	mLightBox = mGui->findWidget<MyGUI::ComboBox>("lightBox");
	mLightBox->eventComboChangePosition +=
		MyGUI::newDelegate(this, &LightControl::onLightIndexChanged);

	mApplyChange = mGui->findWidget<MyGUI::Button>("applyChange");
	mApplyChange->eventMouseButtonClick = 
		MyGUI::newDelegate(this, &LightControl::onApplyChange);
	mApplyChange->setEnabled(false);

	mDeleteLight = mGui->findWidget<MyGUI::Button>("deleteLight");
	mDeleteLight->eventMouseButtonClick =
		MyGUI::newDelegate(this, &LightControl::onDeleteLight);
	mDeleteLight->setEnabled(false);

	mSetAmbient = mGui->findWidget<MyGUI::Button>("setAmbient");
	mSetAmbient->eventMouseButtonClick = 
		MyGUI::newDelegate(this, &LightControl::onSetAmbient);
	mSetAmbient->eventMouseSetFocus =
		MyGUI::newDelegate(this, &LightControl::onSetFocus);
	mSetAmbient->eventMouseLostFocus =
		MyGUI::newDelegate(this, &LightControl::onLostFocus);

	mCreateDirection = mGui->findWidget<MyGUI::Button>("createDirection");
	mCreateDirection->eventMouseButtonClick = 
		MyGUI::newDelegate(this, &LightControl::onCreateDirection);
	mCreateDirection->eventMouseSetFocus =
		MyGUI::newDelegate(this, &LightControl::onSetFocus);
	mCreateDirection->eventMouseLostFocus =
		MyGUI::newDelegate(this, &LightControl::onLostFocus);

	mCreatePoint = mGui->findWidget<MyGUI::Button>("createPoint");
	mCreatePoint->eventMouseButtonClick =
		MyGUI::newDelegate(this, &LightControl::onCreatePoint);
	mCreatePoint->eventMouseSetFocus =
		MyGUI::newDelegate(this, &LightControl::onSetFocus);
	mCreatePoint->eventMouseLostFocus =
		MyGUI::newDelegate(this, &LightControl::onLostFocus);

	mCreateSpot = mGui->findWidget<MyGUI::Button>("createSpot");
	mCreateSpot->eventMouseButtonClick =
		MyGUI::newDelegate(this, &LightControl::onCreateSpot);
	mCreateSpot->eventMouseSetFocus =
		MyGUI::newDelegate(this, &LightControl::onSetFocus);
	mCreateSpot->eventMouseLostFocus =
		MyGUI::newDelegate(this, &LightControl::onLostFocus);

	mColDif = mGui->findWidgetT("colDif");
	mColSpec = mGui->findWidgetT("colSpec");

	mLoaded = true;
	deactivate();
}

void LightControl::unload()
{
	if (!mLoaded)
		return;

	if (mActive)
		deactivate();

	LightMapT::iterator it = mLights.begin();
	for (; it != mLights.end(); ++it)
	{
		delete it->second;
	}
	mLights.clear();
	
	mLoaded = false;
}

void LightControl::activate()
{
	MyGUI::VectorWidgetPtr::iterator it = mContainer.begin();
	for (; it != mContainer.end(); ++it)
	{
		(*it)->setVisible(true);
	}

	mActive = true;
}

void LightControl::deactivate()
{
	MyGUI::VectorWidgetPtr::iterator it = mContainer.begin();
	for (; it != mContainer.end(); ++it)
	{
		(*it)->setVisible(false);
	}

	mActive = false;
}

void LightControl::onDiffuseChange(MyGUI::EditBox* edit)
{
	cv4 col = MyGUI::utility::parseValueEx3<cv4, float>(edit->getCaption());
	mColDif->setColour(MyGUI::Colour(col.r, col.g, col.b));
}

void LightControl::onSpecularChange(MyGUI::EditBox* edit)
{
	cv4 col = MyGUI::utility::parseValueEx3<cv4, float>(edit->getCaption());
	mColSpec->setColour(MyGUI::Colour(col.r, col.g, col.b));
}

void LightControl::onSetAmbient(MyGUI::Widget* button)
{
	cv4 diffuse = MyGUI::utility::parseValueEx3<cv4, float>(mDiffuse->getCaption());
	
	Ogre::SceneManager* sceneMgr = 
		Ogre::Root::getSingletonPtr()->getSceneManager(BFG_SCENEMANAGER);

	sceneMgr->setAmbientLight(diffuse);
}

BFG::View::DirectionalLightCreation LightControl::getDirectionalLightValues(BFG::GameHandle handle)
{
	if (handle == NULL_HANDLE)
		handle = BFG::generateHandle();

	v3 direction = MyGUI::utility::parseValueEx3<v3, float>(mDirection->getCaption());
	cv4 diffuse = MyGUI::utility::parseValueEx3<cv4, float>(mDiffuse->getCaption());
	cv4 specular = MyGUI::utility::parseValueEx3<cv4, float>(mSpecular->getCaption());
	f32 power = MyGUI::utility::parseFloat(mPower->getCaption());

	return BFG::View::DirectionalLightCreation
	(
		handle,
		direction,
		diffuse,
		specular,
		power
	);
}

void LightControl::onCreateDirection(MyGUI::Widget* button)
{
	BFG::View::DirectionalLightCreation dlc = getDirectionalLightValues(NULL_HANDLE);

	mLights.insert(std::make_pair(dlc.mHandle, new BFG::View::Light(dlc)));
	updateLightBox();
}

BFG::View::PointLightCreation LightControl::getPointLightValues(BFG::GameHandle handle)
{
	if (handle == NULL_HANDLE)
		handle = BFG::generateHandle();

	v3 position = MyGUI::utility::parseValueEx3<v3, float>(mPosition->getCaption());
	cv4 diffuse = MyGUI::utility::parseValueEx3<cv4, float>(mDiffuse->getCaption());
	cv4 specular = MyGUI::utility::parseValueEx3<cv4, float>(mSpecular->getCaption());
	f32 power = MyGUI::utility::parseFloat(mPower->getCaption());
	f32 range = MyGUI::utility::parseFloat(mRange->getCaption());
	f32 constant = MyGUI::utility::parseFloat(mConstant->getCaption());
	f32 linear = MyGUI::utility::parseFloat(mLinear->getCaption());
	f32 quadric = MyGUI::utility::parseFloat(mQuadric->getCaption());

	return BFG::View::PointLightCreation
	(
		handle,
		position,
		range,
		constant,
		linear,
		quadric,
		diffuse,
		specular,
		power
	);
}

void LightControl::onCreatePoint(MyGUI::Widget* button)
{
	BFG::View::PointLightCreation plc = getPointLightValues(NULL_HANDLE);

	mLights.insert(std::make_pair(plc.mHandle, new BFG::View::Light(plc)));
	updateLightBox();
}

BFG::View::SpotLightCreation LightControl::getSpotLightValues(BFG::GameHandle handle)
{
	if (handle == NULL_HANDLE)
		handle = BFG::generateHandle();

	v3 position = MyGUI::utility::parseValueEx3<v3, float>(mPosition->getCaption());
	v3 direction = MyGUI::utility::parseValueEx3<v3, float>(mDirection->getCaption());
	cv4 diffuse = MyGUI::utility::parseValueEx3<cv4, float>(mDiffuse->getCaption());
	cv4 specular = MyGUI::utility::parseValueEx3<cv4, float>(mSpecular->getCaption());
	f32 power = MyGUI::utility::parseFloat(mPower->getCaption());
	f32 range = MyGUI::utility::parseFloat(mRange->getCaption());
	f32 constant = MyGUI::utility::parseFloat(mConstant->getCaption());
	f32 linear = MyGUI::utility::parseFloat(mLinear->getCaption());
	f32 quadric = MyGUI::utility::parseFloat(mQuadric->getCaption());
	f32 falloff = MyGUI::utility::parseFloat(mFalloff->getCaption());
	f32 inner = MyGUI::utility::parseFloat(mInnerRadius->getCaption());
	f32 outer = MyGUI::utility::parseFloat(mOuterRadius->getCaption());

	return BFG::View::SpotLightCreation
	(
		handle,
		position,
		direction,
		range,
		constant,
		linear,
		quadric,
		falloff,
		inner,
		outer,
		diffuse,
		specular,
		power
	);
}

void LightControl::onCreateSpot(MyGUI::Widget* button)
{
	BFG::View::SpotLightCreation slc = getSpotLightValues(NULL_HANDLE);
	
	mLights.insert(std::make_pair(slc.mHandle, new BFG::View::Light(slc)));
	updateLightBox();
}

void LightControl::onLostFocus(MyGUI::Widget* button, MyGUI::Widget*)
{
	MyGUI::LanguageManager* langMan = MyGUI::LanguageManager::getInstancePtr();
	std::string tag = langMan->getTag("BFE_Text_ColourNormal");
	MyGUI::Colour col(tag);

	setTextColour(button, col);
}

void LightControl::onSetFocus(MyGUI::Widget* button, MyGUI::Widget*)
{
	MyGUI::LanguageManager* langMan = MyGUI::LanguageManager::getInstancePtr();
	std::string tag = langMan->getTag("BFE_Text_ColourFocused");
	MyGUI::Colour col(tag);

	setTextColour(button, col);
}

void LightControl::onApplyChange(MyGUI::Widget* button)
{	
	size_t index = mLightBox->getIndexSelected();
	std::string handleString = mLightBox->getItemNameAt(index);

	Ogre::SceneManager* sceneMan = 
		Ogre::Root::getSingletonPtr()->getSceneManager(BFG_SCENEMANAGER);
	Ogre::Light* light = sceneMan->getLight(handleString);

	Ogre::Light::LightTypes lightType = light->getType();

	BFG::GameHandle handle = BFG::destringify(handleString);

	delete mLights[handle];

	switch (lightType)
	{
	case Ogre::Light::LT_DIRECTIONAL:
		{
			BFG::View::DirectionalLightCreation dlc = getDirectionalLightValues(handle);
			mLights[handle] = new BFG::View::Light(dlc);
		}
		break;
	case Ogre::Light::LT_POINT:
		{
			BFG::View::PointLightCreation plc = getPointLightValues(handle);
			mLights[handle] = new BFG::View::Light(plc);
		}
		break;
	case Ogre::Light::LT_SPOTLIGHT:
		{
			BFG::View::SpotLightCreation slc = getSpotLightValues(handle);
			mLights[handle] = new BFG::View::Light(slc);
		}
		break;
	}
}

void LightControl::onLightIndexChanged(MyGUI::ComboBox* sender, size_t index)
{
	if (index != MyGUI::ITEM_NONE)
	{
		mApplyChange->setEnabled(true);
		mDeleteLight->setEnabled(true);

		std::string handleString = sender->getItemNameAt(index);

		Ogre::SceneManager* sceneMan = 
			Ogre::Root::getSingletonPtr()->getSceneManager(BFG_SCENEMANAGER);
		Ogre::Light* light = sceneMan->getLight(handleString);

		fillBoxes(light);
	}
	else
	{
		mApplyChange->setEnabled(false);
		mDeleteLight->setEnabled(false);
	}

}

void LightControl::fillBoxes(Ogre::Light* light)
{
	BFG::cv4 diffuse = light->getDiffuseColour();
	BFG::cv4 specular = light->getSpecularColour();
	BFG::f32 power = light->getPowerScale();
	BFG::v3 position = BFG::View::toBFG(light->getPosition());
	BFG::v3 direction = BFG::View::toBFG(light->getDirection());
	BFG::f32 range = light->getAttenuationRange();
	BFG::f32 constant = light->getAttenuationConstant();
	BFG::f32 linear = light->getAttenuationLinear();
	BFG::f32 quadric = light->getAttenuationQuadric();
	BFG::f32 falloff = light->getSpotlightFalloff();
	BFG::f32 innerRadius = light->getSpotlightInnerAngle().valueDegrees();
	BFG::f32 outerRadius = light->getSpotlightOuterAngle().valueDegrees();

	std::stringstream ss;
	
	ss << diffuse.r << " " << diffuse.g << " " << diffuse.b;
	mDiffuse->setCaption(ss.str());
	ss.str("");

	ss << specular.r << " " << specular.g << " " << specular.b;
	mSpecular->setCaption(ss.str());
	ss.str("");

	mPower->setCaption(MyGUI::utility::toString(power));

	ss << position.x << " " << position.y << " " << position.z;
	mPosition->setCaption(ss.str());
	ss.str("");

	ss << direction.x << " " << direction.y << " " << direction.z;
	mDirection->setCaption(ss.str());
	ss.str("");

	mRange->setCaption(MyGUI::utility::toString(range));
	mConstant->setCaption(MyGUI::utility::toString(constant));
	mLinear->setCaption(MyGUI::utility::toString(linear));
	mQuadric->setCaption(MyGUI::utility::toString(quadric));
	mFalloff->setCaption(MyGUI::utility::toString(falloff));
	mInnerRadius->setCaption(MyGUI::utility::toString(innerRadius));
	mOuterRadius->setCaption(MyGUI::utility::toString(outerRadius));
}

void LightControl::updateLightBox()
{
	mLightBox->removeAllItems();

	LightMapT::iterator it = mLights.begin();
	for (; it != mLights.end(); ++it)
	{
		mLightBox->addItem(BFG::stringify(it->first));
	}
}

void LightControl::setTextColour(MyGUI::Widget* button, const MyGUI::Colour& colour)
{
	if (button == mSetAmbient)
	{
		mDif->setTextColour(colour);
	}
	else if (button == mCreateDirection)
	{
		mDif->setTextColour(colour);
		mSpe->setTextColour(colour);
		mPow->setTextColour(colour);
		mDir->setTextColour(colour);
	}
	else if (button == mCreatePoint)
	{
		mDif->setTextColour(colour);
		mSpe->setTextColour(colour);
		mPow->setTextColour(colour);
		mPos->setTextColour(colour);
		mRan->setTextColour(colour);
		mCon->setTextColour(colour);
		mLin->setTextColour(colour);
		mQua->setTextColour(colour);
	}
	else if (button == mCreateSpot)
	{
		mDif->setTextColour(colour);
		mSpe->setTextColour(colour);
		mPow->setTextColour(colour);
		mPos->setTextColour(colour);
		mDir->setTextColour(colour);
		mRan->setTextColour(colour);
		mCon->setTextColour(colour);
		mLin->setTextColour(colour);
		mQua->setTextColour(colour);
		mFal->setTextColour(colour);
		mInn->setTextColour(colour);
		mOut->setTextColour(colour);
	}
}

void LightControl::onDeleteLight(MyGUI::Widget* button)
{
	size_t index = mLightBox->getIndexSelected();
	std::string handleString = mLightBox->getItemNameAt(index);

	BFG::GameHandle handle = BFG::destringify(handleString);

	LightMapT::iterator it = mLights.find(handle);
	if (it == mLights.end())
		return;

	delete it->second;

	mLights.erase(it);

	mLightBox->clearIndexSelected();
	onLightIndexChanged(mLightBox, MyGUI::ITEM_NONE);

	updateLightBox();
}

} // namespace Tool
