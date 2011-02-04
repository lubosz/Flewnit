/*
 * InputManager.cpp
 *
 *  Created on: Dec 6, 2010
 *      Author: tychi
 */

#include "InputManager.h"
#include "Util/Log/Log.h"

#include "Keyboard.h"
#include "Mouse.h"
#include "WiiMote.h"

#include "InputInterpreter.h"

#include <boost/foreach.hpp>

//#ifdef FLEWNIT_USE_GLFW
//# include <GL/glfw.h>
//#else
//#endif
#include "Common/CL_GL_Common.h"





namespace Flewnit
{

InputManager::InputManager()
: mInputInterpreter(0)
{
	// TODO Auto-generated constructor stub
#ifdef FLEWNIT_USE_GLFW

	mKeyboard =  new Keyboard();
	mMouse =    new Mouse();
	mWiiMote =   new WiiMote();

	glfwSetKeyCallback(keyPressedCallback);

	glfwSetMouseButtonCallback(mouseButtonChangedCallback);
	glfwSetMousePosCallback(mouseMovedCallback);
	glfwSetMouseWheelCallback(mouseWheelCallback);


//	mKeyboard = 0;//FLEWNIT_INSTANTIATE( new Keyboard() );
//		mMouse = 0; //FLEWNIT_INSTANTIATE ( new Mouse() );
//		mWiiMote = 0;// FLEWNIT_INSTANTIATE ( new WiiMote() );
#else
	assert(0);
#endif


}

InputManager::~InputManager()
{
	//there is a bug when shutting down and mouse is still hidden;
	mMouse->setHidden(false);

	delete mKeyboard;
	delete mMouse;
	delete mWiiMote;

	delete mInputInterpreter;
}




//let die InputInterpreter do his work: set camera, delegate GUI
bool InputManager::processInput()
{
	assert(mInputInterpreter);

	// keyboard and mouse handle themselves due to tthe callback deleagation;
	// Wiimote has no functionality yet, but for the lulz, lets' "pull" some stuff, even if only stubs are invoked;
	mWiiMote->pullStatus();

	return true;

}

void InputManager::setInputInterpreter(InputInterpreter* inputInterpreter)
{
	if(mInputInterpreter)
	{
		LOG<< WARNING_LOG_LEVEL << "InputInterpreter already set; deleting old one ;\n";
		delete mInputInterpreter;
	}
	mInputInterpreter = inputInterpreter;
}


//the callback functions to go:
void InputManager::keyPressedCallback(int key, int status)
{
	InputManager::getInstance().mKeyboard->keyPressed(key,status);
}

void InputManager::mouseMovedCallback(int newXpos, int newYpos)
{
	InputManager::getInstance().mMouse->positionChanged(newXpos,newYpos);
}

void InputManager::mouseButtonChangedCallback(int button, int status)
{
	InputManager::getInstance().mMouse->buttonChanged(button,status);
}


void InputManager::mouseWheelCallback(int wheelValue)
{
	InputManager::getInstance().mMouse->mouseWheelChanged(wheelValue);
}

void InputManager::setMouseHidden(bool value)
{
	mMouse->setHidden(value);
}


}
