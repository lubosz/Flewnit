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

#include <boost/foreach.hpp>

#ifdef FLEWNIT_USE_GLFW
# include <GL/glfw.h>
#else

#endif


namespace Flewnit
{

InputManager::InputManager()
: mInputInterpreter(0)
{
	// TODO Auto-generated constructor stub
#ifdef FLEWNIT_USE_GLFW
	glfwSetKeyCallback(keyPressedCallback);
	glfwSetMouseButtonCallback(mouseButtonChangedCallback);
	glfwSetMousePosCallback(mouseMovedCallback);

	mKeyboard = FLEWNIT_INSTANTIATE( new Keyboard() );
	mMouse =  FLEWNIT_INSTANTIATE ( new Mouse() );
	mWiiMote =  FLEWNIT_INSTANTIATE ( new WiiMote() );
#else
	assert(0);
#endif


}

InputManager::~InputManager()
{
	delete mKeyboard;
	delete mMouse;
	delete mWiiMote;
}




//let die InputInterpreter do his work: set camera, delegate GUI
void InputManager::processInput()
{
	assert(mInputInterpreter);

	// keyboard and mouse handle themselves due to tthe callback deleagation;
	// Wiimote has no functionality yet, but for the lulz, lets' "pull" some stuff, even if only stubs are invoked;
	mWiiMote->pullStatus();

}

void InputManager::setInputInterpreter(InputInterpreter* inputInterpreter)
{
	if(mInputInterpreter)
	{
		LOG<< WARNING_LOG_LEVEL << "InputInterpreter already set; deleting old one ;\n";
	}
	mInputInterpreter = inputInterpreter;
}


//the callback functions to go:
void InputManager::keyPressedCallback(int key, int status)
{
	mKeyboard->keyPressed(key,status);
}

void InputManager::mouseMovedCallback(int newXpos, int newYpos)
{
	mMouse->positionChanged(Vector2Di(newXpos,newYpos));
}

void InputManager::mouseButtonChangedCallback(int button, int status)
{
	mMouse->buttonChanged(button,status);
}


}
