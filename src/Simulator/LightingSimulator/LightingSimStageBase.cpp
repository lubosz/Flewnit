/*
 * LightingSimStageBase.cpp
 *
 *  Created on: Feb 5, 2011
 *      Author: tychi
 */

#include "LightingSimStageBase.h"

#include <boost/foreach.hpp>

#include "WorldObject/WorldObject.h"
#include "WorldObject/SubObject.h"
#include "Material/VisualMaterial.h"
#include "Geometry/Geometry.h"

#include "URE.h"

namespace Flewnit
{

LightingSimStageBase::LightingSimStageBase(String name,
		RenderingTechnique renderingTechnique,
		const VisualMaterialFlags& materialFlagMask,
		ConfigStructNode* simConfigNode)
:
	SimulationPipelineStage(name, VISUAL_SIM_DOMAIN, simConfigNode),
	mUsedRenderTarget(0),
	mRenderingTechnique(renderingTechnique),
	mMaterialFlagMask(materialFlagMask)
{

}

LightingSimStageBase::~LightingSimStageBase() {
	//nothing to do
}


void LightingSimStageBase::visitSceneNode(SceneNode* node)
{
	WorldObject* wo = dynamic_cast<WorldObject*>(node);

	if(wo)
	{
		BOOST_FOREACH(SubObject* so, wo->getSubObjects(VISUAL_SIM_DOMAIN) )
		{
			VisualMaterial* visMat = dynamic_cast<VisualMaterial*>(so->getMaterial());
			assert("subobject in visual domain has a visual material" &&
					dynamic_cast<VisualMaterial*>(so->getMaterial()) );

			if(
				visMat->getFlags().areCompatibleTo(mMaterialFlagMask)
				&&
				(
					( visMat->getType() != VISUAL_MATERIAL_TYPE_DEBUG_DRAW_ONLY )
					||
					URE_INSTANCE->doDebugDraw()
				)
			)
			{
				visMat->activate(this,so);

				so->getGeometry()->draw();

				visMat->deactivate(this, so);
			}

		}
	}
}


////iterates over the scenegraph and issues material activations and geometry draw calls on materials
////compatible to the specific rendering technique;
//void LightingSimStageBase::drawAllCompliantGeometry(const VisualMaterialFlags& materialFlagMask)
//{
//	SceneNode* rootNode = SimulationResourceManager::getInstance().getScene()->root();
//
//	//TODO
//	assert(0 && "still to implement");
//}

}
