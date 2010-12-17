/*
 * DemoInputInterpreter.cpp
 *
 *  Created on: Dec 17, 2010
 *      Author: tychi
 */

#include "DemoInputInterpreter.h"
#include "Util/Log/Log.h"

#include "URE.h"

#ifdef FLEWNIT_USE_GLFW
#	include <GL/glfw.h>
#else

#endif

namespace Flewnit
{

DemoInputInterpreter::DemoInputInterpreter()
{
	// TODO Auto-generated constructor stub

}

DemoInputInterpreter::~DemoInputInterpreter()
{
	// TODO Auto-generated destructor stub
}

void DemoInputInterpreter::interpretInput(Keyboard* keyboard)
{
	if(keyboard->getRecentKey() == GLFW_KEY_ENTER)
	{
		if(keyboard->getRecentStatus() == GLFW_PRESS)
		{
			LOG<<DEBUG_LOG_LEVEL<<"enter pressed;\n";
		}
		else
		{
			LOG<<DEBUG_LOG_LEVEL<<"enter released;\n";
		}
	}


	if(keyboard->getRecentKey() == GLFW_KEY_ESC)
	{
		LOG<<DEBUG_LOG_LEVEL<<"escape pressed, shutting down;(;\n";
		URE_INSTANCE->requestMainLoopQuit();
	}

}

void DemoInputInterpreter::interpretInput(Mouse* mouse)
{
	static bool hideMouse= false;

	if(mouse->getRecentEvent() == Mouse::MOUSE_EVENT_BUTTON_CHANGED)
	{
		if(mouse->getRecentButton() == GLFW_MOUSE_BUTTON_LEFT )
		{
			if(mouse->getRecentButtonStatus() == GLFW_PRESS)
			{
				LOG<<DEBUG_LOG_LEVEL<<"left mouse button pressed;\n";
			}
			else
			{
				LOG<<DEBUG_LOG_LEVEL<<"left mouse button released;\n";
			}
		}

		if(mouse->getRecentButton() == GLFW_MOUSE_BUTTON_RIGHT )
		{
			if(mouse->getRecentButtonStatus() == GLFW_PRESS)
			{
				LOG<<DEBUG_LOG_LEVEL<<"right mouse button pressed; toggling mouse hide status;\n";
				hideMouse = !hideMouse;
				mouse->setHidden(hideMouse);
			}
			else
			{

			}
		}

	}
	else
	{
		if(mouse->getRecentEvent() == Mouse::MOUSE_EVENT_POSITION_CHANGED)
		{
			LOG<<DEBUG_LOG_LEVEL<<"mouse moved from ("
					<<mouse->getLastPosition().x
					<<","
					<<mouse->getLastPosition().y
					<<") to ("
					<<mouse->getRecentPosition().x
					<<","
					<<mouse->getRecentPosition().y
					<<");\n"
					;
		}
	}
}

void DemoInputInterpreter::interpretInput(WiiMote* wiiMote)
{
	//nothing to do yet;
}

}
