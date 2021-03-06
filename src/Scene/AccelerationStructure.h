/*
 * AccelerationStructure.h
 *
 *  Created on: Apr 21, 2011
 *      Author: tychi
 *
 *  Base class for all kinds of data Acceleration structures;
 *  Has no functionality; Just for structuring concepts;
 */


#pragma once


#include "WorldObject/WorldObject.h"


#include "Simulator/SimulatorMetaInfo.h"



namespace Flewnit
{
	enum AccelerationStructureType
	{
		UNIFORM_GRID,
		//rest not used in this thesis;
		OCT_TREE,
		BOUNDING_VOULUME_HIERARCHY,
		KD_TREE
	};


	class AccelerationStructure
	 : public WorldObject //derive from WorldObject to be debug-drawable!
	{
		 FLEWNIT_BASIC_OBJECT_DECLARATIONS

	public:
		AccelerationStructure(String name, AccelerationStructureType type);
		virtual ~AccelerationStructure();

		//throws exception if scene representation is not compatible
		//virtual void update(SceneRepresentation* sceneRep) throw(SimulatorException) = 0;

	private:

		AccelerationStructureType mAccelerationStructureType;
	};


}
