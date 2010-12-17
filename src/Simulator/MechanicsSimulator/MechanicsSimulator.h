/*
 * MechanicsSimulator.h
 *
 *  Created on: Dec 16, 2010
 *      Author: tychi
 */

#pragma once

#include "../SimulatorInterface.h"

namespace Flewnit
{

class MechanicsSimulator: public SimulatorInterface
{
	FLEWNIT_BASIC_OBJECT_DECLARATIONS;
public:
	MechanicsSimulator();
	virtual ~MechanicsSimulator();

	virtual bool stepSimulation() throw(SimulatorException) ;
	//build pipeline according to config;
	virtual bool initPipeLine()throw(SimulatorException) ;
	//check if pipeline stages are compatible to each other (also to those stages form other simulators (they might have to interact!))
	virtual bool validatePipeLine()throw(SimulatorException) ;

};

}
