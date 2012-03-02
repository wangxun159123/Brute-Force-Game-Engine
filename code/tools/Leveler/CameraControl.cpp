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

#include <CameraControl.h>

#include <boost/geometry/arithmetic/cross_product.hpp>

#include <MyGUI_OgrePlatform.h>

#include <Core/Utils.h>
#include <View/CameraCreation.h>
#include <View/Event.h>

namespace Tool
{

void CameraControl::createDefaultCamera()
{
	
	// create mygui panels
	MyGUI::LayoutManager* layMan = MyGUI::LayoutManager::getInstancePtr();
	mContainer = layMan->loadLayout("Camera.layout");

	if (mContainer.empty())
		throw std::runtime_error("Camera.layout found empty!");

	mFullView = mContainer.front()->castType<MyGUI::Canvas>();

	MyGUI::RenderManager* renMan = MyGUI::RenderManager::getInstancePtr();

	MyGUI::IntSize size = renMan->getViewSize();
	MyGUI::IntPoint panelPos = mFullView->getPosition();

	mFullSize = mFullView->getSize();

	mFullView->setSize(size.width - panelPos.left, size.height - panelPos.top);

	mViews[0] = mFullView->findWidget("CameraOne")->castType<MyGUI::Canvas>();
	mViews[1] = mFullView->findWidget("CameraTwo")->castType<MyGUI::Canvas>();
	mViews[2] = mFullView->findWidget("CameraThree")->castType<MyGUI::Canvas>();
	mViews[3] = mFullView->findWidget("CameraFour")->castType<MyGUI::Canvas>();

	resize();

	// create ogre cameras
	mSceneMan->destroyAllCameras();

	mCameraPosition = mSceneMan->getRootSceneNode()->createChildSceneNode();
	mCameraPosition->setPosition(0, 0, 0);

	mCameraRotation = mCameraPosition->createChildSceneNode();

	mCameraDistance = mCameraRotation->createChildSceneNode(BFG::stringify(mData->mCamera));

	BFG::View::CameraCreation cc
	(
		mData->mCamera,
		mData->mCamera,
		true,
		0,
		0
	);
	
	emit<View::Event>(BFG::ID::VE_CREATE_CAMERA, cc, mData->mState);

	for (size_t i = 0; i < 4; ++i)
	{
		mData->mCameras[i] = generateHandle();
		createCam(mData->mCameras[i], &mViewNodes[i]);
		setCanvas(mViews[i], mData->mCameras[i], &mViewTargets[i]);

		BFG::v3 position;
		BFG::qv4 orientation;
		BFG::f32 startDistance = 
			MyGUI::utility::parseFloat(mViews[i]->getUserString("startDistance"));

		const std::string look(mViews[i]->getUserString("lookFrom"));
		if (look == "Free")
		{
			position = BFG::v3::UNIT_X * startDistance;
			orientation = BFG::qv4::IDENTITY;
		}
		else
		{
			position = (-mViewParameterMap[look].mDirection) * startDistance;
			orientation = mViewParameterMap[look].mOrientation;
		}

		emit<View::Event>(BFG::ID::VE_UPDATE_POSITION, position, mData->mCameras[i]);
		emit<View::Event>(BFG::ID::VE_UPDATE_ORIENTATION, orientation, mData->mCameras[i]);

		MyGUI::Widget* title = mViews[i]->findWidget("Title");
		const std::string type(mViews[i]->getUserString("type"));
		title->setProperty("Caption", look + " (" + type + ")");
	}
}

void CameraControl::createCam(BFG::GameHandle camHandle, Ogre::SceneNode** node)
{
	(*node) = mSceneMan->getRootSceneNode()->createChildSceneNode(stringify(camHandle));

	BFG::View::CameraCreation cc
	(
		camHandle,
		camHandle,
		false,
		mFullSize.width / 2,
		mFullSize.height / 2
	);

	emit<View::Event>(BFG::ID::VE_CREATE_CAMERA, cc, mData->mState);
}

void CameraControl::setCanvas(MyGUI::Canvas* canvas, BFG::GameHandle camHandle, Ogre::RenderTarget** target)
{
	destroy(canvas);
	
	addCanvasEvents(canvas);
	canvas->createTexture(MyGUI::Canvas::TRM_PT_VIEW_ALL, MyGUI::TextureUsage::RenderTarget);
	canvas->setUserString("camHandle", stringify(camHandle));
	canvas->setUserData(target);
	canvas->updateTexture();
}

void CameraControl::addCanvasEvents(MyGUI::Canvas* canvas)
{
	canvas->eventPreTextureChanges += MyGUI::newDelegate(this, &CameraControl::eventPreTextureChanges);
	canvas->requestUpdateCanvas = MyGUI::newDelegate(this, &CameraControl::requestUpdateCanvas);
	canvas->eventMouseButtonPressed += MyGUI::newDelegate(this, &CameraControl::onMousePressed);
	canvas->eventMouseButtonReleased += MyGUI::newDelegate(this, &CameraControl::onMouseReleased);
	canvas->eventMouseDrag += MyGUI::newDelegate(this, &CameraControl::onMouseDrag);
	canvas->eventKeyButtonReleased += MyGUI::newDelegate(this, &CameraControl::onKeyReleased);
}

void CameraControl::removeCanvasEvents(MyGUI::Canvas* canvas)
{
	canvas->eventPreTextureChanges -= MyGUI::newDelegate(this, &CameraControl::eventPreTextureChanges);
	canvas->requestUpdateCanvas = nullptr;
	canvas->eventMouseButtonPressed -= MyGUI::newDelegate(this, &CameraControl::onMousePressed);
	canvas->eventMouseButtonReleased -= MyGUI::newDelegate(this, &CameraControl::onMouseReleased);
	canvas->eventMouseDrag -= MyGUI::newDelegate(this, &CameraControl::onMouseDrag);
	canvas->eventKeyButtonReleased -= MyGUI::newDelegate(this, &CameraControl::onKeyReleased);
}

void CameraControl::destroy(MyGUI::Canvas* canvas)
{
	if (canvas)
	{
		removeCanvasEvents(canvas);
		canvas->destroyTexture();
		canvas->clearUserString("camHandle");
	}
}

void CameraControl::eventPreTextureChanges(MyGUI::Canvas* canvas)
{
	removeTexture(canvas);
}

void CameraControl::removeTexture(MyGUI::Canvas* canvas)
{
	Ogre::RenderTarget** target = *canvas->getUserData<Ogre::RenderTarget**>();
	if ((*target) != nullptr)
	{
		(*target)->removeAllViewports();
		(*target) = nullptr;
		if (canvas)
			Ogre::Root::getSingleton().getRenderSystem()->destroyRenderTexture(canvas->getTexture()->getName());
	}
}

void CameraControl::requestUpdateCanvas(MyGUI::Canvas* _canvas, MyGUI::Canvas::Event _event)
{
	const std::string camName(_canvas->getUserString("camHandle"));
	if (camName.empty())
		return;
	if (!mSceneMan->hasCamera(camName))
		return;
	if (!(_event.textureChanged || _event.requested || _event.widgetResized))
		return;

	Ogre::Camera* cam = mSceneMan->getCamera(camName);

	Ogre::RenderTarget** oldTarget = *_canvas->getUserData<Ogre::RenderTarget**>();

	Ogre::TexturePtr texture = static_cast<MyGUI::OgreTexture*>(_canvas->getTexture())->getOgreTexture();
	Ogre::RenderTexture* target = texture->getBuffer()->getRenderTarget();

	if (*oldTarget != target
		&& target != nullptr
		&& cam != nullptr )
	{
		(*oldTarget) = target;
		(*oldTarget)->removeAllViewports();
		Ogre::Viewport* viewport = (*oldTarget)->addViewport(cam);
		viewport->setBackgroundColour(Ogre::ColourValue::Black);
		viewport->setClearEveryFrame(true);
		viewport->setOverlaysEnabled(false);
	}
}

void CameraControl::resize()
{
	mFullSize = mFullView->getSize();

	MyGUI::IntSize panelSize(mFullSize.width / 2, mFullSize.height / 2);

	mViews[0]->setSize(panelSize);

	mViews[1]->setSize(panelSize);
	mViews[1]->setPosition(panelSize.width, 0);

	mViews[2]->setSize(panelSize);
	mViews[2]->setPosition(0, panelSize.height);

	mViews[3]->setSize(panelSize);
	mViews[3]->setPosition(panelSize.width, panelSize.height);

	setAspectRatio(stringify(mData->mCameras[0]), (float)panelSize.width, (float)panelSize.height);
	setAspectRatio(stringify(mData->mCameras[1]), (float)panelSize.width, (float)panelSize.height);
	setAspectRatio(stringify(mData->mCameras[2]), (float)panelSize.width, (float)panelSize.height);
	setAspectRatio(stringify(mData->mCameras[3]), (float)panelSize.width, (float)panelSize.height);

	if (mFullView->isUserString("camHandle"))
	{
		setAspectRatio
		(
			mFullView->getUserString("camHandle"), 
			(float)mFullSize.width, 
			(float)mFullSize.height
		);
	}
}

void CameraControl::disableMultiview(bool disable)
{
	mViews[0]->setVisible(!disable);
	mViews[1]->setVisible(!disable);
	mViews[2]->setVisible(!disable);
	mViews[3]->setVisible(!disable);
}

void CameraControl::setAspectRatio(const std::string& camName, float width, float height)
{
	if (mSceneMan->hasCamera(camName))
	{
		Ogre::Camera* cam = mSceneMan->getCamera(camName);
		cam->setAspectRatio(width / height);
	}
}

void CameraControl::eventHandler(BFG::Controller_::VipEvent* ve)
{

}

void CameraControl::onMousePressed(MyGUI::Widget* widget, int x, int y, MyGUI::MouseButton id)
{
	if (id == MyGUI::MouseButton::Right)
	{
		mDragStart = MyGUI::IntPoint(x, y);
	}
	else if (id == MyGUI::MouseButton::Middle)
	{
		mZoomStart = MyGUI::IntPoint(x, y);
		mZoomSpeed = 0.0f;
		mZoomWidget = widget;
	}
}

void CameraControl::intersectPosition(MyGUI::Widget* widget, int x, int y, BFG::v3& result)
{
	const std::string& look(widget->getUserString("lookFrom"));
	if (look == "Free")
		throw std::runtime_error("Free cameras don't have a plane to intersect!");

	const std::string camName(widget->getUserString("camHandle"));
	if (camName.empty())
		throw std::runtime_error("CamHandle not found in widget " + widget->getName());

	Ogre::Camera* cam = mSceneMan->getCamera(camName);

	MyGUI::IntCoord coord = widget->getAbsoluteCoord();
	BFG::f32 mouseX = (x - coord.left) / (BFG::f32)coord.width;
	BFG::f32 mouseY = (y - coord.top) / (BFG::f32)coord.height;

	Ogre::Ray mouseRay = cam->getCameraToViewportRay(mouseX, mouseY);

	Ogre::Plane& plane = mViewParameterMap[look].mPlane;
	std::pair<bool, Ogre::Real> intersection = mouseRay.intersects(plane);

	if(intersection.first)
	{
		result = BFG::View::toBFG(mouseRay.getPoint(intersection.second));
		return;
	}

	throw std::runtime_error("No intersection with plane in widget " + widget->getName());
}

void CameraControl::onMouseReleased(MyGUI::Widget* widget, int x, int y, MyGUI::MouseButton id)
{
	if (id == MyGUI::MouseButton::Right)
	{
		mDragStart = MyGUI::IntPoint(0, 0);
	}
	else if (id == MyGUI::MouseButton::Middle)
	{
		mZoomStart = MyGUI::IntPoint(0,0);
		mZoomSpeed = 0.0f;
		mZoomWidget = NULL;
	}
}

void CameraControl::onMouseDrag(MyGUI::Widget* widget, int x, int y, MyGUI::MouseButton id)
{
	if (mDragStart.left == x && mDragStart.top == y)
		return;

	if (!widget->isUserString("lookFrom"))
		throw std::runtime_error("lookFrom is not found in widget " + widget->getName());

	const std::string look(widget->getUserString("lookFrom"));
	if (look == "Free")
		return;

	if (id == MyGUI::MouseButton::Right)
	{
		BFG::v3 position;
		intersectPosition(widget, x, y, position);

		BFG::v3 startPosition;
		intersectPosition(widget, mDragStart.left, mDragStart.top, startPosition);

		BFG::v3 dragDifference = startPosition - position;

		const std::string camName(widget->getUserString("camHandle"));
		if (camName.empty())
			throw std::runtime_error("camHandle property empty");

		Ogre::SceneNode* node = mSceneMan->getSceneNode(camName);
		BFG::v3 nodePosition = BFG::View::toBFG(node->getPosition());
		BFG::v3 newPosition(nodePosition + dragDifference);
		node->setPosition(BFG::View::toOgre(newPosition));

		mDragStart = MyGUI::IntPoint(x, y);
	}
	if (id == MyGUI::MouseButton::Middle)
	{
		if (mZoomStart.top == y)
			return;

		mZoomSpeed = 0.1f * (y - mZoomStart.top);
	}
}

void CameraControl::onKeyReleased(MyGUI::Widget* widget, MyGUI::KeyCode key)
{
	if (key == MyGUI::KeyCode::F5)
	{
		disableMultiview(mMultiview);

		if (mMultiview)
		{
			const std::string camName(widget->getUserString("camHandle"));
			if (camName.empty())
				throw std::runtime_error("camHandle property not found");

			mFullView->setUserString("camHandle", camName);

			MyGUI::TextBox* fullTitle = mFullView->findWidget("Title")->castType<MyGUI::TextBox>();
			MyGUI::TextBox* viewTitle = widget->findWidget("Title")->castType<MyGUI::TextBox>();

			fullTitle->setCaption(viewTitle->getCaption());
			fullTitle->setSize(viewTitle->getSize());

			if (widget->isUserString("lookFrom"))
			{
				mFullView->setUserString("lookFrom", widget->getUserString("lookFrom"));
			}

			Ogre::RenderTarget** target = *widget->getUserData<Ogre::RenderTarget**>();

			setCanvas(mFullView, BFG::destringify(camName), target);
			resize();
		}
		else
		{
			MyGUI::Widget* title = mFullView->findWidget("Title");
			if (title)
				title->setProperty("Caption", "");
			removeCanvasEvents(mFullView);
		}

		mMultiview = !mMultiview;
	}
}

void CameraControl::update(const Ogre::FrameEvent& evt)
{
// 	if (mCamOrbit)
// 	{
// 		mCameraRotation->rotate
// 		(
// 			Ogre::Vector3::UNIT_Y,
// 			Ogre::Radian(mDeltaRot.y) * evt.timeSinceLastFrame,
// 			Ogre::Node::TS_WORLD
// 		);
// 
// 		mCameraRotation->rotate
// 		(
// 			Ogre::Vector3::UNIT_X,
// 			Ogre::Radian(mDeltaRot.x) * evt.timeSinceLastFrame,
// 			Ogre::Node::TS_WORLD
// 		);
// 	}
// 	else
// 	{
// 		mCameraRotation->yaw(Ogre::Radian(mDeltaRot.y) * evt.timeSinceLastFrame);
// 		mCameraRotation->pitch(Ogre::Radian(mDeltaRot.x) * evt.timeSinceLastFrame);
// 		mCameraRotation->roll(Ogre::Radian(mDeltaRot.z) * evt.timeSinceLastFrame);
// 	}
// 
// 	mCamDistance += mDeltaDis * evt.timeSinceLastFrame;
// 	if (mCamDistance <= 1.0f)
// 		mCamDistance = 1.0f;
// 
// 	mCameraDistance->setPosition(0.0f, 0.0f, -mCamDistance);
// 
// 	mDeltaRot = v3::ZERO;
// 
// 	if (!mIsZooming)
// 	{
// 		mDeltaDis = 0.0f;
// 	}

	if (!mCamerasCreated)
	{
		if (mSceneMan->hasCamera(stringify(mData->mCameras[0])) &&
		    mSceneMan->hasCamera(stringify(mData->mCameras[1])) &&
			mSceneMan->hasCamera(stringify(mData->mCameras[2])) &&
			mSceneMan->hasCamera(stringify(mData->mCameras[3])))
		{
			mCamerasCreated = true;
			resize();
			for (size_t i = 0; i < 4; ++i)
			{
				if (mViews[i]->getUserString("type") == "orthographic")
				{
					const std::string distance(mViews[i]->getUserString("startDistance"));
					Ogre::Camera* cam = mSceneMan->getCamera(stringify(mData->mCameras[i]));
					cam->setProjectionType(Ogre::PT_ORTHOGRAPHIC);
					cam->setOrthoWindowHeight(MyGUI::utility::parseFloat(distance));
				}
			}
		}
	}

	if (mFullSize != mFullView->getSize())
		resize();

	if (mZoomWidget)
	{
		if (mZoomSpeed > BFG::EPSILON_F || mZoomSpeed < -BFG::EPSILON_F)
		{
			const std::string camName(mZoomWidget->getUserString("camHandle"));
			if (camName.empty())
				throw std::runtime_error("CamHandle not found in widget " + mZoomWidget->getName());

			const std::string camType(mZoomWidget->getUserString("type"));
			if (camType == "orthographic")
			{
				Ogre::Camera* cam = mSceneMan->getCamera(camName);
				f32 height = cam->getOrthoWindowHeight();

				height += mZoomSpeed * evt.timeSinceLastFrame;
				if (height < BFG::EPSILON_F)
					height = BFG::EPSILON_F;

				cam->setOrthoWindowHeight(height);
			}
			else if (camType == "perspective")
			{
				Ogre::SceneNode* node = mSceneMan->getSceneNode(camName);

				node->translate(0, 0, mZoomSpeed * evt.timeSinceLastFrame, Ogre::Node::TS_LOCAL);
			}
		}
	}
}

} // namespace Tool
