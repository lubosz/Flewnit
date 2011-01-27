/*
 * SimulationResourceManager.cpp
 *
 *  Created on: Dec 16, 2010
 *      Author: tychi
 */

#include "SimulationResourceManager.h"
#include "Scene/Scene.h"
#include "Simulator/LightingSimulator/RenderTarget/RenderTarget.h"
#include "UserInterface/WindowManager/WindowManager.h"

#include <boost/foreach.hpp>
#include "WorldObject/InstanceManager.h"
#include "Material/Material.h"
#include "Geometry/Geometry.h"
#include "Buffer/Texture.h"
#include "MPP/MPP.h"


namespace Flewnit
{

SimulationResourceManager::SimulationResourceManager()
:mScene(new Scene()),
 mGlobalRenderTarget(
		 new  RenderTarget(String("globalRenderTarget"),
				 WindowManager::getInstance().getWindowResolution(),
		 	 	 //hardcode
				 true,
				 DEPTH_RENDER_BUFFER,
				 0)
 )
{
	mGlobalRenderTarget->bind();
	mGlobalRenderTarget->requestCreateAndStoreTexture(FINAL_RENDERING_SEMANTICS);
	mGlobalRenderTarget->attachStoredColorTexture(FINAL_RENDERING_SEMANTICS, 0);
	mGlobalRenderTarget->renderToScreen();
	// TODO Auto-generated constructor stub

}

SimulationResourceManager::~SimulationResourceManager()
{
	delete mScene;
	delete mGlobalRenderTarget;

	typedef Map<String, InstanceManager*> InstanceManagerMap;
	BOOST_FOREACH( InstanceManagerMap::value_type & pair, mInstanceManagers)
	{
		delete pair.second;
	}

	typedef Map<String, Material*> MaterialMap;
	BOOST_FOREACH( MaterialMap::value_type & pair, mMaterials)
	{
		delete pair.second;
	}

	typedef Map<String, Geometry*> GeometryMap;
	BOOST_FOREACH( GeometryMap::value_type & pair, mGeometries)
	{
		delete pair.second;
	}

	typedef Map<ID, BufferInterface* > BufferMap;
	BOOST_FOREACH( BufferMap::value_type & pair, mBuffers)
	{
		delete pair.second;
	}

	typedef Map<String, MPP*> MPPMap;
	BOOST_FOREACH( MPPMap::value_type & pair, mMPPs)
	{
		delete pair.second;
	}


}


RenderTarget* SimulationResourceManager::getGlobalRenderTarget()const
{
	return mGlobalRenderTarget;
}

Scene* SimulationResourceManager::getScene()const
{
	return mScene;
}

void SimulationResourceManager::registerInstanceManager(InstanceManager* im)
{
	//TODO
}

InstanceManager* SimulationResourceManager::getInstanceManager(String name)
{
	//TODO
}

//when a Simulation pass nears its end, it should let do the instance managers the
//"compiled rendering", as render()-calls to instanced geometry only registers drawing needs
//to its instance manager; For every  (at least non-deferred non-Skybox Lighting-) rendering pass,
//call this routine after scene graph traversal
void SimulationResourceManager::executeInstancedRendering()
{
	//TODO
}

//automatically called by BufferInterface constructor
void SimulationResourceManager::registerBufferInterface(BufferInterface* bi)
{
	//TODO
}
//be very careful with this function, as ther may be serveral references
//and I don't work with smart pointers; so do a manual deletion only if you are absolutely sure
//that the buffer is't used by other objects;
//void deleteBufferInterface(BufferInterface* bi);

void SimulationResourceManager::registerTexture(Texture* tex)
{
	//TODO
}
//void SimulationResourceManager::deleteTexture(Texture* tex);

void SimulationResourceManager::registerMPP(MPP* mpp)
{
	//TODO
}

//void SimulationResourceManager::SimulationResourceManager::deleteMPP(MPP* mpp);

void SimulationResourceManager::registerMaterial(Material* mat)
{
	//TODO
}
//void SimulationResourceManager::deleteMaterial(Material* mat);

void SimulationResourceManager::registerGeometry(Geometry* geo)
{
	//TODO
}
//void SimulationResourceManager::deleteGeometry(Geometry* geo);

}
