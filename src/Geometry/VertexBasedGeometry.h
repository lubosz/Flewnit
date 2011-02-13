/*
 * VertexBasedGeometry.h
 *
 *  Created on: Jan 26, 2011
 *      Author: tychi
 *
 * The most common geometry Representation; Owns and maintains an OpenGL Vertex Buffer Object;
 * Is sharable with OpenCL; Is used for point, line and triangle rendering;
 */

#pragma once

#include "Geometry.h"

namespace Flewnit
{



class VertexBasedGeometry :
	public BufferBasedGeometry
{
	FLEWNIT_BASIC_OBJECT_DECLARATIONS;

public:
	VertexBasedGeometry(String name, GeometryRepresentation geoRep);
	virtual ~VertexBasedGeometry();

	//the semantics of the buffer is readable from its bufferInfo
	//non-pure virtual, as it must be overridden by VertexBasedGeometry to register
	//to the VBO
	virtual void setAttributeBuffer(BufferInterface* buffi) throw(BufferException);

	void setIndexBuffer(BufferInterface* buffi) throw(BufferException);
	//returns NULL if index buffer doesn't exist;
	BufferInterface* getIndexBuffer();

	virtual void draw(
				//SimulationPipelineStage* currentStage, SubObject* currentUsingSuboject,
				unsigned int numInstances,
				GeometryRepresentation desiredGeomRep);


private:

	//handle to the OpenGL Vertex Buffer Object;
	GLuint mGLVBO;

	BufferInterface* mIndexBuffer;

	//compare buffers for sizees, types, number of elements etc;
	virtual void validateBufferIntegrity()throw(BufferException);
};

}


