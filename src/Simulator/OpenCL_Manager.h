/*
 * OpenCL_Manager.h
 *
 *  Created on: Dec 19, 2010
 *      Author: tychi
 */

#pragma once

#include "Common/Singleton.h"
#include "Common/BasicObject.h"


//forward declarations might be too error prone (to redefine all the typedefs etc.; Forwarding the gl/cl-stuff can be done in the future as a refactoring step);
#include "Common/CL_GL_Common.h"

#ifdef _DEBUG
	//Macro to permanently check for errors in debug mode
#	define GUARD(expression) \
	expression; \
	OpenCL_Manager::getInstance().checkCLGLErrors()
#else
#	define GUARD(expression) expression
#endif

namespace Flewnit
{

class OpenCL_Manager
	: 	public Singleton<OpenCL_Manager>,
		public BasicObject
{
	FLEWNIT_BASIC_OBJECT_DECLARATIONS;

	//a cl::Platform member needed?

    cl::Device mUsedDevice;

    cl::Context mCLContext;
    cl::CommandQueue mCommandQueue;

    //debugging variables
    cl_int mLastCLError;
    GLenum mLastGLError;
    cl::Event mLastEvent;

    bool init(bool useCPU =false);

public:
	OpenCL_Manager( bool useCPU =false){init(useCPU);}

	virtual ~OpenCL_Manager();

	void checkCLGLErrors();

	//modifiable references
	cl::Context& getCLContext();
	cl::CommandQueue& getCommandQueue();
	cl::Device& getUsedDevice();
	cl::Event& getLastEvent();

    inline cl_int& getLastCLError(){return mLastCLError;}
    inline GLenum getLastGLError(){return mLastGLError;}



};

}

