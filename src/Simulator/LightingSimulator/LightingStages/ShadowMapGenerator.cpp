/*
 * ShadowMapGenerator.cpp
 *
 *  Created on: Feb 5, 2011
 *      Author: tychi
 */

#include "ShadowMapGenerator.h"

#include "MPP/Shader/ShaderManager.h"
#include "Simulator/LightingSimulator/RenderTarget/RenderTarget.h"
#include "Buffer/BufferHelperUtils.h"
#include "Buffer/Texture.h"
#include "Util/Log/Log.h"

namespace Flewnit
{

ShadowMapGenerator::ShadowMapGenerator(ConfigStructNode* simConfigNode)
: LightingSimStageBase("ShadowMapGenerationStage",
		RENDERING_TECHNIQUE_SHADOWMAP_GENERATION,
		//mask every material not castnig shadows:
		VisualMaterialFlags(true,false,false,false,false),
		simConfigNode),
	mShadowMapResolution(0) //set in init function
{


}

ShadowMapGenerator::~ShadowMapGenerator()
{
	//this stage owns this target, so it will destroy it
	delete mUsedRenderTarget;
}

bool ShadowMapGenerator::stepSimulation() throw(SimulatorException)
{
	//LOG<<DEBUG_LOG_LEVEL<<"Shadowmap generator in action! ;(;\n";

	return true;
}

void ShadowMapGenerator::initStage()throw(SimulatorException)
{
	//TODO remove hardCode
	mShadowMapResolution = 512;
	assert(BufferHelper::isPowerOfTwo(mShadowMapResolution));


	if(ShaderManager::getInstance().getGlobalShaderFeatures().lightSourcesShadowFeature
			!= LIGHT_SOURCES_SHADOW_FEATURE_NONE)
	{
		TextureType texType = TEXTURE_TYPE_2D;

		switch(ShaderManager::getInstance().getGlobalShaderFeatures().lightSourcesShadowFeature)
		{
		case LIGHT_SOURCES_SHADOW_FEATURE_ONE_SPOT_LIGHT:
			texType = TEXTURE_TYPE_2D;
			break;
		case LIGHT_SOURCES_SHADOW_FEATURE_ONE_POINT_LIGHT:
			texType = TEXTURE_TYPE_2D_CUBE;
			break;
		case LIGHT_SOURCES_SHADOW_FEATURE_ALL_SPOT_LIGHTS:
			texType = TEXTURE_TYPE_2D_ARRAY;
			//keep ini shadowcaster value
			break;
		default: break; //omit waning that the NONE value wasn't checked
		}

		mUsedRenderTarget= new RenderTarget(
				String("ShadowMapGenerationStageRT"),
				Vector2Dui(mShadowMapResolution,mShadowMapResolution),
				texType,
				DEPTH_TEXTURE,
				BufferElementInfo(1,GPU_DATA_TYPE_FLOAT,32,false),
				1,
				ShaderManager::getInstance().getGlobalShaderFeatures().numMaxShadowCasters
				);
	}

	//make shadowmap(s) accessible from other stages
	mRenderingResults[SHADOW_MAP_SEMANTICS] = mUsedRenderTarget->getStoredDepthTexture();

	//make even the depth generation shader material-associated;
	//like this, there is no special treatment of "global shaders" in the logic,
	//and some special materials like those of voxel fluids can contribute with their own shaders
	//generated for them;
	//cause shader generation now:
	ShaderManager::getInstance().setRenderingScenario(this);

//	mShadowMapGenerationShader = new DepthImageGenerationShader(
//			ShaderManager::getInstance().getShaderCodeDirectory(),
//			RENDERING_TECHNIQUE_SHADOWMAP_GENERATION,
//			mUsedRenderTarget->getTextureType,
//			false TODO HOWTO HANDLE GLOBAL SHADERS FOR INSTANCING
//	);
}

void ShadowMapGenerator::validateStage()throw(SimulatorException)
{
	//no dependencies;
}

}
