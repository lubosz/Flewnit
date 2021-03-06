/*
 * ShadowMapGenerator.h
 *
 *  Created on: Feb 5, 2011
 *      Author: tychi
 */

#pragma once


#include "Simulator/LightingSimulator/LightingSimStageBase.h"

namespace Flewnit {

class ShadowMapGenerator
: public LightingSimStageBase
{
	FLEWNIT_BASIC_OBJECT_DECLARATIONS;

public:
	ShadowMapGenerator(ConfigStructNode* simConfigNode);
	virtual ~ShadowMapGenerator();


	virtual bool stepSimulation() throw(SimulatorException);
	virtual bool initStage()throw(SimulatorException);
	virtual bool validateStage()throw(SimulatorException) ;

	void setPolygonOffset(const Vector2D& val);
	inline const Vector2D& getPolygonOffset()const{return mPolygonOffset;}

private:

	int mShadowMapResolution; //shadowmap is alwayas of squared size:
	Vector2D mPolygonOffset;

};

}

