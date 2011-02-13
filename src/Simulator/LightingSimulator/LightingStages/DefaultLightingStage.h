/*
 * DefaultLightingStage.h
 *
 *  Created on: Feb 5, 2011
 *      Author: tychi
 */

#pragma once



#include "Simulator/LightingSimulator/LightingSimStageBase.h"



namespace Flewnit
{


class DefaultLightingStage
: public LightingSimStageBase
{
	FLEWNIT_BASIC_OBJECT_DECLARATIONS;

public:
	DefaultLightingStage(ConfigStructNode* simConfigNode);
	virtual ~DefaultLightingStage();

	virtual bool stepSimulation() throw(SimulatorException);
	virtual void initStage()throw(SimulatorException);
	virtual void validateStage()throw(SimulatorException);

};

}

