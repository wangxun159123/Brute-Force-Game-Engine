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

#include "PowerUpEffect.h"

#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreParticleSystem.h>

#include "Globals.h"


PowerupEffect::PowerupEffect(const v3& position, f32 intensity) :
	mPosition(position),
	mIntensity(intensity),
	mHandle(generateHandle()),
	mNode(NULL),
	mParticleSystem(NULL),
	mElapsedTime(0.0f)
{
	Ogre::SceneManager* sceneMgr =
		Ogre::Root::getSingletonPtr()->getSceneManager(BFG_SCENEMANAGER);

	mNode = static_cast<Ogre::SceneNode*>
		(sceneMgr->getRootSceneNode()->createChild(stringify(mHandle)));

	mNode->setPosition(Ogre::Vector3(mPosition.ptr()));

	mParticleSystem = sceneMgr->createParticleSystem
	(
		stringify(mHandle) + "particle",
		"Powerup_1"
	);

	if (!mParticleSystem)
		throw std::runtime_error("Explosion: creating mParticleSystem failed!");
}

PowerupEffect::~PowerupEffect()
{
	Ogre::SceneManager* sceneMgr =
		Ogre::Root::getSingleton().getSceneManager(BFG_SCENEMANAGER);

	sceneMgr->destroyParticleSystem(mParticleSystem);
	sceneMgr->destroySceneNode(mNode);
}

bool PowerupEffect::frameStarted( const Ogre::FrameEvent& evt )
{
	if (done())
		return true;

	mElapsedTime += evt.timeSinceLastFrame;

	fireParticles
	(
		mParticleSystem,
		mNode,
		0.0f,
		2.0f,
		mElapsedTime
	);

	if (mElapsedTime > 2.0f)
		finished();

	return true;
}
