
/*
 * InstanceManager.h
 *
 *  Created on: Jan 26, 2011
 *      Author: tychi
 *
 *  Class designed to handle "adaptive" instanced rendering, i.e. every instenced is explicitely
 *  registered for drawing every rendering pass; this enables masking/culling of unneeded/unwanted instances;
 */

#pragma once


#include "Simulator/SimulationObject.h"

#include "Simulator/SimulatorForwards.h"

#include "Common/CL_GL_Common.h"

namespace Flewnit
{

class InstanceManager
: public SimulationObject
{
	FLEWNIT_BASIC_OBJECT_DECLARATIONS;
public:
	InstanceManager(String name, GLuint numMaxInstances,
			Material* associatedNonInstancedMaterial, Geometry* nonInstancedGeometryToDraw );

	virtual ~InstanceManager();

	//needed by  VisualMaterial::activate() to see if it is an actual draw call or just a request to register
	//an instance at the manager for later batched drawing
	static inline bool instancedRenderingIsCurrentlyActive(){return sInstancedRenderingIsCurrentlyActive;}

	//creates a new SubObject, containing a new InstancedGeometry and a re-used InstancedMaterial object
	//throws exception if
	SubObject* createInstance()throw(SimulatorException);

	//called by InstancedVisualMaterial::activate();
	//SubObject must own an InstacedGeometry;
	void registerInstanceForNextDrawing(InstancedGeometry* instancedGeo);

	void drawRegisteredInstances();

	inline Buffer* getInstanceTransformationInfoUniformBuffer()const{return mInstanceTransformationInfoUniformBuffer;}

private:

	//called at the end of drawRegisteredInstances() to reset registration state;
	void resetDrawQueue();

	GLuint mMaxManagedInstances;
	GLuint mCreatedManagedInstances;
	GLuint mNumCurrentlyRegisteredInstancesForNextDrawing;


	//Uniform buffer containing the model,modelView,modelViewProjection transforms of the currently
	//registered geometry instances, plus the instance ID
	Buffer* mInstanceTransformationInfoUniformBuffer;

	//the "real" material holding the shading information
	Material* mAssociatedNonInstancedMaterial;
	//the "real" geometry which will be rendered
	Geometry* mNonInstancedGeometryToDraw;

	static bool sInstancedRenderingIsCurrentlyActive;
};

}


