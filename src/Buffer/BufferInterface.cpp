/*
 * BufferInterface.cpp
 *
 *  Created on: Nov 27, 2010
 *      Author: tychi
 */

#include "BufferInterface.h"

#include "Common/Profiler.h"

#include "Common/Math.h"

//for casting:
///\{
#include "PingPongBuffer.h"
#include "Buffer.h"

#include "Texture.h"
#include "Texture1D.h"
#include "Texture2D.h"
#include "Texture3D.h"
#include "RenderBuffer.h"
///\}


namespace Flewnit
{


BufferInfo::~BufferInfo()
{

}

BufferInfo::BufferInfo(String name,ContextTypeFlags usageContexts)
	: name(name),
	  bufferTypeFlags(EMPTY_BUFFER_FLAG),
	  usageContexts(usageContexts),
	  isPingPongBuffer(false),isTexture(false),
	  isRenderBuffer(false),
	  isSharedByCLAndGL(false),
	  elementType(TYPE_UNDEF),
	  numElements(0),
	  dimensionality(1),
	  dimensionExtends(Vector3Dui(0,0,0)),
	  textureTarget(GL_TEXTURE_2D),
	  imageInternalChannelLayout(GL_RGBA),
	  imageInternalDataType(GL_FLOAT),
	  mappedToCPUContext(NO_CONTEXT_TYPE)
{
	allocationGuards[HOST_CONTEXT_TYPE] = false;
	allocationGuards[OPEN_CL_CONTEXT_TYPE] = false;
	allocationGuards[OPEN_GL_CONTEXT_TYPE] = false;
}

BufferInfo::BufferInfo(const BufferInfo& rhs)
{
	(*this) = rhs;
}

bool BufferInfo::operator==(const BufferInfo& rhs) const
{
		return bufferTypeFlags == rhs.bufferTypeFlags &&
				usageContexts == rhs.usageContexts &&
				isPingPongBuffer == rhs.isPingPongBuffer &&
				isTexture == rhs.isTexture &&
				isRenderBuffer == rhs.isRenderBuffer &&
				isSharedByCLAndGL == rhs.isSharedByCLAndGL &&
				allocationGuards[HOST_CONTEXT_TYPE] == rhs.allocationGuards[HOST_CONTEXT_TYPE] &&
				allocationGuards[OPEN_CL_CONTEXT_TYPE] == rhs.allocationGuards[OPEN_CL_CONTEXT_TYPE] &&
				allocationGuards[OPEN_GL_CONTEXT_TYPE] == rhs.allocationGuards[OPEN_GL_CONTEXT_TYPE] &&
				elementType == rhs.elementType &&
				numElements == rhs.numElements &&
				dimensionality == rhs.dimensionality &&
				dimensionExtends.x == rhs.dimensionExtends.x &&
				dimensionExtends.y == rhs.dimensionExtends.y &&
				dimensionExtends.z == rhs.dimensionExtends.z &&
				textureTarget == rhs.textureTarget &&
				imageInternalChannelLayout  == rhs.imageInternalChannelLayout &&
				imageInternalDataType == rhs.imageInternalDataType &&
				mappedToCPUContext == rhs.mappedToCPUContext;
}

const BufferInfo& BufferInfo::operator=(const BufferInfo& rhs)
{
		bufferTypeFlags = rhs.bufferTypeFlags;
		usageContexts = rhs.usageContexts ;
		isPingPongBuffer = rhs.isPingPongBuffer ;
		isTexture = rhs.isTexture;
		isRenderBuffer = rhs.isRenderBuffer;
		isSharedByCLAndGL = rhs.isSharedByCLAndGL ;
		allocationGuards[HOST_CONTEXT_TYPE] = rhs.allocationGuards[HOST_CONTEXT_TYPE];
		allocationGuards[OPEN_CL_CONTEXT_TYPE] = rhs.allocationGuards[OPEN_CL_CONTEXT_TYPE];
		allocationGuards[OPEN_GL_CONTEXT_TYPE] = rhs.allocationGuards[OPEN_GL_CONTEXT_TYPE];
		elementType = rhs.elementType ;
		numElements = rhs.numElements ;
		dimensionality = rhs.dimensionality ;
		dimensionExtends.x = rhs.dimensionExtends.x ,
		dimensionExtends.y = rhs.dimensionExtends.y ;
		dimensionExtends.z = rhs.dimensionExtends.z ;
		textureTarget = rhs.textureTarget;
		imageInternalChannelLayout  = rhs.imageInternalChannelLayout;
		imageInternalDataType = rhs.imageInternalDataType;
		mappedToCPUContext = rhs.mappedToCPUContext;

		return *this;
}



BufferInterface::BufferInterface(String name,ContextTypeFlags usageContexts)
:mBufferInfo(new BufferInfo(name, usageContexts)), mCPU_Handle(0), mGraphicsBufferHandle(0)
{
	//the compute-handle manages its initialization for itself due to the cl-c++-bindings :)
}

BufferInterface::~BufferInterface()
{
	delete mBufferInfo;

	//delete mCPU_Handle;
	free(mCPU_Handle);

	//some guard in order to check if the implementor of a derived class has thought about the release of the gl-object
	//(via glDeleteBuffers(), glDeleteTextures() or glDeleteRenderbuffers())
	assert("derived classes have to release the GL handle appropriately if the use it!" && mGraphicsBufferHandle==0);
}


//---------------------------------------------------------------------------------------------------------
#if (FLEWNIT_TRACK_MEMORY || FLEWNIT_DO_PROFILING)

void BufferInterface::registerBufferAllocation(ContextTypeFlags contextTypeFlags, size_t sizeInByte)
{
	Profiler::getInstance().registerBufferAllocation(contextTypeFlags,sizeInByte);
}

void BufferInterface::unregisterBufferAllocation(ContextTypeFlags contextTypeFlags, size_t sizeInByte)
{
	Profiler::getInstance().unregisterBufferAllocation(contextTypeFlags,sizeInByte);
}

#endif
//---------------------------------------------------------------------------------------------------------



bool BufferInterface::hasBufferInContext(ContextType type) const
{
	//return mBufferInfo->allocationGuards[type];
	return (mBufferInfo->bufferTypeFlags & FLEWNIT_FLAGIFY(type)) != 0;
}



//get the bufferinfo directly:
const BufferInfo& BufferInterface::getBufferInfo() const
{
	return *mBufferInfo;
}


//convenience functions to access bufferInfo data;

BufferTypeFlags BufferInterface::getBufferTypeFlags()const
{
	return mBufferInfo->bufferTypeFlags;
}


String BufferInterface::getName() const
{
	return mBufferInfo->name;
}




int  BufferInterface::getNumElements() const
{
	return mBufferInfo->numElements;
}


size_t  BufferInterface::getElementSize() const
{
	return BufferHelper::elementSize( mBufferInfo->elementType );
}


Type BufferInterface::getElementType() const
{
	return mBufferInfo->elementType;
}

bool BufferInterface::isDefaultBuffer()const
{
	return 		(!(mBufferInfo->isPingPongBuffer))
			&& 	(!(mBufferInfo->isTexture))
			&& 	(!(mBufferInfo->isRenderBuffer));
}

Buffer& BufferInterface::toDefaultBuffer()throw(BufferException)
{
	Buffer* toCastPtr = dynamic_cast<Buffer*>(this);
	if(toCastPtr) return *toCastPtr;
	else throw(BufferException("Bad cast to default Buffer"));
}


//convenience caster methods:
bool BufferInterface::isPingPongBuffer()const
{
	return mBufferInfo->isPingPongBuffer;
}

PingPongBuffer& BufferInterface::toPingPongBuffer() throw(BufferException)
{
	PingPongBuffer* toCastPtr = dynamic_cast<PingPongBuffer*>(this);
	if(toCastPtr) return *toCastPtr;
	else throw(BufferException("Bad cast to PingPongBuffer"));
}


bool BufferInterface::isTexture() const
{
	return mBufferInfo->isTexture;
}
bool BufferInterface::isTexture1D() const
{
	return isTexture() && mBufferInfo->dimensionality == 1;
}

Texture1D& BufferInterface::toTexture1D() throw(BufferException)
{
	Texture1D* toCastPtr = dynamic_cast<Texture1D*>(this);
	if(toCastPtr) return *toCastPtr;
	else throw(BufferException("Bad cast to Texture1D"));
}

bool BufferInterface::isTexture2D() const
{
	return isTexture() && mBufferInfo->dimensionality == 2;
}

Texture2D& BufferInterface::toTexture2D() throw(BufferException)
{
	Texture2D* toCastPtr = dynamic_cast<Texture2D*> (this);
	if (toCastPtr)
		return *toCastPtr;
	else
		throw(BufferException("Bad cast to Texture2D"));
}

bool BufferInterface::isTexture3D() const
{
	return isTexture() && mBufferInfo->dimensionality == 3;
}
Texture3D& BufferInterface::toTexture3D() throw(BufferException)
{
	Texture3D* toCastPtr = dynamic_cast<Texture3D*> (this);
	if (toCastPtr)
		return *toCastPtr;
	else
		throw(BufferException("Bad cast to Texture3D"));
}

bool BufferInterface::isRenderBuffer() const
{
	return mBufferInfo->isRenderBuffer;
}

RenderBuffer& BufferInterface::toRenderBuffer() throw(BufferException)
{
	RenderBuffer* toCastPtr = dynamic_cast<RenderBuffer*> (this);
	if (toCastPtr)
		return *toCastPtr;
	else
		throw(BufferException("Bad cast to RenderBuffer"));
}

const CPUBufferHandle BufferInterface::getCPUBufferHandle()const{return mCPU_Handle;}
GraphicsBufferHandle BufferInterface::getGraphicsBufferHandle()const{return mGraphicsBufferHandle;}
ComputeBufferHandle BufferInterface::getComputeBufferHandle()const{return mComputeBufferHandle;}




}
