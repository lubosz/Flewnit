/*
 * Geometry.cpp
 *
 *  Created on: Dec 21, 2010
 *      Author: tychi
 */

#include "Geometry.h"
#include "Buffer/BufferInterface.h"

namespace Flewnit
{

Geometry::Geometry( String name, GeometryRepresentation geoRep)
:
		SimulationObject( name, GENERIC_SIM_DOMAIN),
		mGeometryRepresentation(geoRep)
{
}

Geometry::~Geometry()
{
	//nothing to do
}


//------------------------------------------------------------------------------------------

BufferBasedGeometry::BufferBasedGeometry(String name, GeometryRepresentation geoRep)
: Geometry(name, geoRep)
{
	for(int runner = 0; runner < __NUM_VALID_GEOMETRY_ATTRIBUTE_SEMANTICS__; runner++)
	{
		mAttributeBuffers[runner]=0;
	}
}

BufferBasedGeometry::~BufferBasedGeometry()
{
	//nothing to delete, everything managed by sim resource manager;
}


void BufferBasedGeometry::setAttributeBuffer(BufferInterface* buffi) throw(BufferException)
{
	mAttributeBuffers[buffi->getBufferInfo().bufferSemantics] = buffi;
	validateBufferIntegrity();
}

//can return NULL pointer if buffer is not registered for the given semantics
BufferInterface* BufferBasedGeometry::getAttributeBuffer(BufferSemantics bs)
{
	return mAttributeBuffers[bs];
}




}
