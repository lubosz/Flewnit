/*
 * BufferInterface.h
 *
 *  Created on: Nov 27, 2010
 *      Author: tychi
 *
 * \brief The base class of all buffers;
 *
 */

#pragma once

//TODO from/to typeid stuff

#include "Common/FlewnitSharedDefinitions.h"

#include "Common/BasicObject.h"

#include "Buffer/BufferSharedDefinitions.h"

#include "Buffer/BufferHelperUtils.h"

#include "Common/Math.h"

#include <exception>



namespace Flewnit
{


class BufferException : public std::exception
{
	String mDescription;
 public:
	BufferException(String description = "unspecified Buffer exception") throw()
	: mDescription(description)
	{ }

	virtual ~BufferException() throw(){}

	virtual const char* what() const throw()
	{
	    return mDescription.c_str();
	}
};


/**
 * some partially redundant information about the buffer;
 */
class BufferInfo
{
public:
	String name;
	ContextTypeFlags usageContexts;
	BufferSemantics bufferSemantics;


	Type elementType; //default TYPE_UNDEF
	cl_GLuint numElements;

	GLBufferType glBufferType; //default NO_GL_BUFFER_TYPE; must have other value if the GL-flag is set in usageContexs
	bool isPingPongBuffer; //default false, set by the appropriate class;
	//guard if some buffer is mapped
	ContextType mappedToCPUContext; //default NO_CONTEXT_TYPE, valid only the latter and OPEN_CL_CONTEXT_TYPE and OPEN_GL_CONTEXT_TYPE

	//value automatically calulated by  numElements * BufferHelper::elementSize(elementType);
	cl_GLuint bufferSizeInByte;

	explicit BufferInfo(String name,
			ContextTypeFlags usageContexts,
			BufferSemantics bufferSemantics,
			Type elementType,
			cl_GLuint numElements,
			GLBufferType glBufferType = NO_GL_BUFFER_TYPE,
			ContextType mappedToCPUContext = NO_CONTEXT_TYPE);
	BufferInfo(const BufferInfo& rhs);
	virtual ~BufferInfo();
	bool operator==(const BufferInfo& rhs) const;
	const BufferInfo& operator=(const BufferInfo& rhs);

};

//additional image-specific information to the "generic" BufferInfo above;
class TextureInfo
 {
public:
	cl_GLuint dimensionality; //interesting for textures: 1,2 or 3 dimensions;
	Vector3Dui dimensionExtends; //must be zero for unused dimensions;

	GLenum textureTarget; //default GL_TEXTURE_2D
	GLint imageInternalChannelLayout; //usually GL_RGBA or GL_LUMINANCE
	GLenum imageInternalDataType;	//usually GL_UNSIGNED_INT or GL_FLOAT

	GLint numMultiSamples; //default 0 to indicate no multisampling
	bool isMipMapped;		//default false;

private:


	explicit TextureInfo(
			cl_GLuint dimensionality,
			Vector3Dui dimensionExtends,

			GLenum textureTarget,
			GLint imageInternalChannelLayout,
			GLenum imageInternalDataType,

			GLint numMultiSamples,
			bool isMipMapped
			);
	TextureInfo(const TextureInfo& rhs);
	virtual ~TextureInfo();
	bool operator==(const TextureInfo& rhs) const;
	const TextureInfo& operator=(const TextureInfo& rhs);
 };


class BufferInterface
: public BasicObject
{
	FLEWNIT_BASIC_OBJECT_DECLARATIONS;
public:

	//planned usage must be determined in the beginning
	explicit BufferInterface(const BufferInfo& buffi);
	virtual ~BufferInterface();


	//check for campatibility: not the contents, but the types, dimensions, allocations etc are compared;
	virtual bool operator==(const BufferInterface& rhs) const = 0;
	//copy contents of the one buffer to the other, but only if they are of the same leaf type, buffer type, same size, element type, dimensions etc;
	const BufferInterface& operator=(const BufferInterface& rhs) throw(BufferException);

	void bind(ContextType type)throw(BufferException);

	//memory manipulating stuff:
	///\{
	// memory must be allocated before (done by constructors of derived classes), else exception;
	//convention: copy data into cpu buffer if cpu buffer is used and allocated;
	//throw exception if cpu context is specified in flags, but not used by the buffer object;
	//the context flags are provided to omit unnecessary copies, e.g. when fresh data is only needed by one "context"
	void setData(const void* data, ContextTypeFlags where)throw(BufferException);

	void readBack()throw(BufferException);

	void* mapBuffer()throw(BufferException)
	{
		//TODO implement when needed
		throw(BufferException("mapCPUAdressSpaceTo() not implemented yet"));
	}
	void unmapBuffer()throw(BufferException)
	{
		//TODO implement when needed
		throw(BufferException("mapCPUAdressSpaceTo() not implemented yet"));
	}
	///\}




	//convenience functions to access bufferInfo data;
	///\{
	String getName()const;

	ContextTypeFlags getContextTypeFlags()const;
	bool hasBufferInContext(ContextType type) const;
	bool isCLGLShared()const;

	int  getNumElements() const;
	size_t  getElementSize() const;
	Type getElementType() const;

	//get the bufferinfo directly:
	const BufferInfo& getBufferInfo() const;
	///\}

	//convenience caster methods:
	///\{
	bool isPingPongBuffer() const;
	PingPongBuffer& toPingPongBuffer() throw(BufferException);

	bool isDefaultBuffer()const;
	Buffer& toDefaultBuffer()throw(BufferException);

	bool isTexture() const;
	bool isTexture1D() const;
	Texture1D& toTexture1D() throw(BufferException);
	bool isTexture2D() const;
	Texture2D& toTexture2D() throw(BufferException);
	bool isTexture3D() const;
	Texture3D& toTexture3D() throw(BufferException);
	bool isRenderBuffer() const;
	RenderBuffer& toRenderBuffer() throw(BufferException);
	///\}


	//getters n setters for the buffer handles (might need to be exposed due to passing as kernel arguments etc)
	///\{
	const CPUBufferHandle getCPUBufferHandle()const;
	GraphicsBufferHandle getGraphicsBufferHandle()const;
	ComputeBufferHandle getComputeBufferHandle()const;
	///\}
protected:

	//to be called by the constructor;
	bool allocMem()throw(BufferException);

	//wrapper functions to GL and CL calls without any error checking,
	//i.e. semantic checks/flag delegation/verifiaction must be done before those calls;
	//those routines are introduced to reduce boilerplate code;
	///\{
	//PingPongBuffer calls following routines of his managed buffers; so let's make him friend
	friend class PingPongBuffer;

	virtual void generateGL()=0;
	virtual void generateCL()=0;
	virtual void generateCLGL()=0;

	//the two non-symmetrci GL-only routines:
	virtual void bindGL()=0;
	virtual void allocGL()=0;

	virtual void writeGL(const void* data)=0;
	virtual void writeCL(const void* data)=0;
	virtual void readGL(void* data)=0;
	virtual void readCL(void* data)=0;
	virtual void copyGLFrom(GraphicsBufferHandle bufferToCopyContentsFrom)=0;
	virtual void copyCLFrom(ComputeBufferHandle bufferToCopyContentsFrom)=0;
	virtual void freeGL()=0;
	virtual void freeCL()=0;

//	//not needed (yet?)
//	virtual void* mapGLToHost()=0;
//	virtual void* mapCLToHost()=0;
//	virtual void* unmapGL()=0;
//	virtual void* unmapCL()=0;

	//virtual void allocCL()=0; <-- bullshaat ;)
	//virtual void bindCL()=0; <-- bullshaat ;)
	///\}

#if (FLEWNIT_TRACK_MEMORY || FLEWNIT_DO_PROFILING)
	//friend Profiler so that he can set the ID of the BasicObjects;
	friend class Profiler;

	void registerBufferAllocation(ContextTypeFlags contextTypeFlags, size_t sizeInByte);
	void unregisterBufferAllocation(ContextTypeFlags contextTypeFlags, size_t sizeInByte);
#endif

	BufferInfo mBufferInfo;

	//NULL if no host pointer exists;
	CPUBufferHandle mCPU_Handle;
	//0 if no openGL buffer exists;
	GraphicsBufferHandle mGraphicsBufferHandle;
	//mComputeBufferHandle(), i.e "cl_mem cl::Memory::operator()" is NULL if no Buffer exists;
	//OpenCL distinguishes between a buffer and an image; I don't like this conceptional
	//separation, as they are both regions in memory; Thus, the interface in this framework
	//is different than in openCL: Buffer is the base class, Texture etc. derive from this,
	//non-image Buffers are derictly implemented and internally acessed by casting
	//the cl::Memory member to cl::Buffer/cl::BufferGL;
	//interop handle classes (e.g. cl::BufferGL, cl::Image2D, cl::Image2DGL etc.) will be handled by statically casting the cl::Memory mCLBuffer member;
	ComputeBufferHandle mComputeBufferHandle;


};

}

