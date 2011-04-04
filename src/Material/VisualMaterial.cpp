/*
 * VisualMaterial
 *
 *  Created on: Dec 22, 2010
 *      Author: tychi
 */

#include "VisualMaterial.h"
#include "MPP/Shader/ShaderManager.h"
#include "Buffer/Texture.h"
#include "MPP/Shader/Shader.h"
#include "Simulator/LightingSimulator/LightingSimStageBase.h"

#include <boost/foreach.hpp>
#include "Simulator/LightingSimulator/LightingStages/ShadowMapGenerator.h"
#include "WorldObject/InstanceManager.h"
#include "Simulator/ParallelComputeManager.h"
#include "UserInterface/WindowManager/WindowManager.h"
#include "Util/Log/Log.h"


namespace Flewnit
{

VisualMaterialFlags::VisualMaterialFlags(
		bool castsShadows,bool isTransparent,bool isShadable, bool isDynamicCubeMapRenderable,
		bool isInstanced,bool isCustomMaterial)
:		castsShadows(castsShadows),
 		isTransparent(isTransparent),
		isShadable(isShadable),
		isDynamicCubeMapRenderable(isDynamicCubeMapRenderable),
		isInstanced(isInstanced),
		isCustomMaterial(isCustomMaterial)
{}

VisualMaterialFlags::VisualMaterialFlags(const VisualMaterialFlags& rhs)
:		castsShadows(rhs.castsShadows),
 		isTransparent(rhs.isTransparent),
		isShadable(rhs.isShadable),
		isDynamicCubeMapRenderable(rhs.isDynamicCubeMapRenderable),
		isInstanced(rhs.isInstanced),
		isCustomMaterial(rhs.isCustomMaterial)
{}

//checker to compare a meterial's flags with those of a lighting stage in order to check if they are compatible
bool VisualMaterialFlags::areCompatibleTo(const VisualMaterialFlags& lightingStageMask)const
{
	if( (! castsShadows ) && lightingStageMask.castsShadows ) return false;
	if( (! isTransparent )&& lightingStageMask.isTransparent ) return false;
	if( (! isShadable )   &&  lightingStageMask.isShadable) return false;
	if( (! isDynamicCubeMapRenderable ) && lightingStageMask.isDynamicCubeMapRenderable) return false;

	//Instancing is not tested, as it should not play any role, the shader generation
	//and the geometry implementation should completely hide the special treatment of instanced geometry...


	//mask any kind of custom material, is handled by another control flow of special lighting stages;
	if( isCustomMaterial || lightingStageMask.isCustomMaterial) return false;

	return true;
}

bool VisualMaterialFlags::operator==(const VisualMaterialFlags& rhs) const
{
	return
		castsShadows ==rhs.castsShadows &&
		isTransparent ==rhs.isTransparent &&
		isShadable  ==rhs.isShadable &&
		isDynamicCubeMapRenderable ==rhs.isDynamicCubeMapRenderable &&
		isInstanced ==rhs.isInstanced &&
		isCustomMaterial  ==rhs.isCustomMaterial
		;
}

//--------------------------------------------------------------------------------------

VisualMaterial::VisualMaterial(
			String name,
			VisualMaterialType type,
			//ShaderFeatures shaderFeatures,
			ShadingFeatures shadingFeatures,
			//must contain at least the textures used in the shader as samplers
			const std::map<BufferSemantics, Texture*>& textures,
			const VisualMaterialFlags& visualMaterialFlags,
			float shininess,
			float reflectivity
			)
: 	Material(name, VISUAL_SIM_DOMAIN),
	mType(type),
	mShadingFeatures(shadingFeatures),
	mVisMatFlags(visualMaterialFlags),

	mCurrentlyUsedShader(0),
	mShininess(shininess),
	mReflectivity(reflectivity)
{
	mTextures = textures;

    if((mShadingFeatures & SHADING_FEATURE_TESSELATION ) != 0 )
    {
    	if(WindowManager::getInstance().getAvailableOpenGLVersion().x < 4)
    	{
    		LOG<<WARNING_LOG_LEVEL<<"You requested a tessellated material, but your active OpenGL version "
    				<<"doesn't support this; Hence, Tesselation will be deactivated;\n";
    		reinterpret_cast<unsigned int&>(mShadingFeatures) &=  (~SHADING_FEATURE_TESSELATION);
    	}
    }

	validateTextures();

	ShaderManager::getInstance().registerVisualMaterial(this);
}

VisualMaterial::~VisualMaterial()
{
	if(ShaderManager::isInitialised())
	{
		ShaderManager::getInstance().unregisterVisualMaterial(this);
	}
	//nothing to delete, every used stuff is "owned by other manager/container classes
}

//check for equality in order to check if a material with the desired properties
//(shader feature set and textures) already exists in the ResourceManager;
bool VisualMaterial::operator==(const Material& rhs) const
{
	const VisualMaterial* castedMat = dynamic_cast<const VisualMaterial*>(&rhs);

	if(castedMat)
	{
		//DON't compare name, as the name plays no role for logical equality

		//first, compare the logical features:
		if(
				mType == castedMat->getType() &&
				mShadingFeatures == castedMat->getShadingFeatures() &&
				mVisMatFlags == castedMat->getFlags()
		)
		{
			//compare textures; that the texture state is valid, has already been asserted,
			//hence we don't need to check for existance
			BOOST_FOREACH(SemanticsToTextureMap::value_type semTexPair, mTextures)
			{
				assert(castedMat->getTexture(semTexPair.first));
				if(
					!(
						*(castedMat->getTexture(semTexPair.first)) ==
						*(semTexPair.second)
					)
				)
				{
					return false;
				}
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	return true;
}

void VisualMaterial::activate(
			SimulationPipelineStage* currentStage,
			SubObject* currentUsingSuboject) throw(SimulatorException)
{
	//following assertiopns on incompatible rendering states to detect bad masking by the sim-stage
	LightingSimStageBase* castedStage= reinterpret_cast<LightingSimStageBase*>(currentStage);
	assert(castedStage);
	assert(castedStage->getRenderingTechnique() != RENDERING_TECHNIQUE_DEFERRED_LIGHTING);
	if( castedStage->getRenderingTechnique() == RENDERING_TECHNIQUE_SHADOWMAP_GENERATION )
		{assert(castsShadows());}
	if( castedStage->getRenderingTechnique() == RENDERING_TECHNIQUE_DEFERRED_GBUFFER_FILL )
		{assert(! isTransparent());}

	// starting real functionality from here on:
	if(isInstanced())
	{
		//activate shader only if we are in a real draw call issued by an InstanceManager;
		if(InstanceManager::instancedRenderingIsCurrentlyActive())
		{
			//we have a "real" geometry bound, i.e. there shall be drawn something
			mCurrentlyUsedShader->use(currentUsingSuboject);
		}
	}
	else
	{
		mCurrentlyUsedShader->use(currentUsingSuboject);
	}

}

void VisualMaterial::deactivate(
		SimulationPipelineStage* currentStage,
		SubObject* currentUsingSuboject) throw(SimulatorException)
{
	//do nothing in this base class;
}


//called by ShaderManager when a new shader becomes necessary as the rendering scenario has changed;
//calls validate()
void VisualMaterial::setShader(Shader* shader)
{
	mCurrentlyUsedShader = shader;
	validateShader();

}


//calls "validate"
void VisualMaterial::setTexture(Texture* tex)
{
	mTextures[tex->getBufferInfo().bufferSemantics] = tex;
	validateTextures();
}

void VisualMaterial::setTexture(BufferSemantics explicitSemantics, Texture* tex)
{
	mTextures[explicitSemantics] = tex;
	validateTextures();
}


//returns NULL if doesn't exist;
Texture* VisualMaterial::getTexture(BufferSemantics bs)const
{
	if(mTextures.find(bs)== mTextures.end())
	{
		return 0;
	}
	return mTextures.find(bs)->second;
	//return mTextures[bs];
}


//assert that every material- (and hence shader-)used texture is available and that it is of the correct type;
//called by constructor
void VisualMaterial::validateTextures()throw(SimulatorException)
{
	if(mType == VISUAL_MATERIAL_TYPE_DEFAULT_LIGHTING)
	{
		if( (mShadingFeatures &  SHADING_FEATURE_DIFFUSE_TEXTURING) != 0)
		{
			assert(mTextures.find(DIFFUSE_COLOR_SEMANTICS) != mTextures.end());
		}
		if( (mShadingFeatures &  SHADING_FEATURE_DETAIL_TEXTURING) != 0)
		{
			assert(mTextures.find(DETAIL_TEXTURE_SEMANTICS) != mTextures.end());
		}
		if( (mShadingFeatures &  SHADING_FEATURE_NORMAL_MAPPING) != 0)
		{
			assert(mTextures.find(NORMAL_SEMANTICS) != mTextures.end());
		}
		if( (mShadingFeatures &  SHADING_FEATURE_CUBE_MAPPING) != 0)
		{
			assert(mTextures.find(ENVMAP_SEMANTICS) != mTextures.end());
		}
		if( (mShadingFeatures & SHADING_FEATURE_AMBIENT_OCCLUSION ) != 0)
		{
			assert(mTextures.find(POSITION_SEMANTICS) != mTextures.end()
			        && "for ambient occlusion calc, at the moment, "
					"there is no other way then binding a position texture, "
					"as in the shader generation, the depth-optimazation is not included yet");
		}

		//shadow map check: <-- shadow maps CANNOT exist so far, because materials are created before
		//any simulators and their according render targets;
		//the shadowmaps must be set globally
//		if( (ShaderManager::getInstance().getGlobalShaderFeatures().lightSourcesShadowFeature
//				!= LIGHT_SOURCES_SHADOW_FEATURE_NONE)
//			&&
//			( (mShadingFeatures & SHADING_FEATURE_DIRECT_LIGHTING  ) != 0)
//		)
//		{
//			assert(mTextures.find(SHADOW_MAP_SEMANTICS) != mTextures.end());
//		}

		return;

	}

	if(mType == VISUAL_MATERIAL_TYPE_SKYDOME_RENDERING)
	{
		assert(mTextures.find(ENVMAP_SEMANTICS) != mTextures.end());
		return;
	}

	if(mType == VISUAL_MATERIAL_TYPE_DEBUG_DRAW_ONLY)
	{
		return;
	}


	throw(SimulatorException("no other visual material type supported yet"));

	//tex->getTextureType()
}
//called by setShader();
void VisualMaterial::validateShader()throw(SimulatorException)
{
	if(mCurrentlyUsedShader)
	{
		assert( (mCurrentlyUsedShader->getLocalShaderFeatures().instancedRendering )
			== isInstanced() );
	}

	//TODO further validation when needed
}


//-------------------------------------------------------------------------------------------


SkyDomeMaterial::SkyDomeMaterial(String name, Texture2DCube* cubeTex)
: VisualMaterial(
		name,
		VISUAL_MATERIAL_TYPE_SKYDOME_RENDERING,
		SHADING_FEATURE_CUBE_MAPPING,
		//must contain at least the textures used in the shader as samplers
		std::map<BufferSemantics, Texture*>{std::pair<BufferSemantics, Texture*>(ENVMAP_SEMANTICS, cubeTex)},
		VisualMaterialFlags(
			//NO shadow casting, NOT transparent,
			false,false,
			//YES shadable (actually not, but as functionality is integrated in ubershader and defaultlighting stage,
			//we have to set this flag in order to not mask out the sky dome),
			true,
			//NOT instanced,
			//YES dyncubemaprenderable,NOT custom (is special, but can still
			//be handled by generic lighting stages)
			true,false)
		)
{

}

SkyDomeMaterial::~SkyDomeMaterial()
{

}

	//check for equality in order to check if a material with the desired properties
	//(shader feature set and textures) already exists in the ResourceManager;
bool SkyDomeMaterial::operator==(const Material& rhs) const
{
	return (
		dynamic_cast<const SkyDomeMaterial*>(&rhs)
		&&
		VisualMaterial::operator==(rhs)
	);
}

void SkyDomeMaterial::activate(
			SimulationPipelineStage* currentStage,
			SubObject* currentUsingSuboject) throw(SimulatorException)
{
	//GUARD(glDisable(GL_DEPTH_TEST));
	GUARD(glCullFace(GL_FRONT));

	getCurrentlyUsedShader()->use(currentUsingSuboject);
}
void SkyDomeMaterial::deactivate(SimulationPipelineStage* currentStage,
			SubObject* currentUsingSuboject) throw(SimulatorException)
{
	//GUARD(glEnable(GL_DEPTH_TEST));
	GUARD(glCullFace(GL_BACK));
}

}
