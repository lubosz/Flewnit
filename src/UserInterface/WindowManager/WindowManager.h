/*
 * WindowManager.h
 *
 *  Created on: Dec 6, 2010
 *      Author: tychi
 *
 * Interface class for window management
 */

#pragma once

#include "Common/Singleton.h"
#include "Common/BasicObject.h"
#include "Common/Math.h"

namespace Flewnit
{

class Config;

class WindowManager
	: public Singleton<WindowManager>, public BasicObject
{
	FLEWNIT_BASIC_OBJECT_DECLARATIONS
public:

	WindowManager(){}
	virtual ~WindowManager(){}



	virtual void init() = 0 ;
	virtual void cleanup() = 0;

	virtual void swapBuffers() = 0;

	virtual void toggleFullScreen() = 0;
	virtual void setMouseGrab(bool value) = 0;

	virtual void setWindowTitle(String title) =0;

	virtual float getLastFrameDuration() = 0;
	virtual double getFPS(bool averaged =false )=0;


protected:


	virtual void createWindow(bool fullScreen, const Vector2Di& position, const Vector2Di& resolution) = 0;






};

}

