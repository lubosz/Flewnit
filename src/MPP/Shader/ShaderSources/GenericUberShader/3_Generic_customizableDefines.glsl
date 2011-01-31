//GLSL Shader Template: customizable defines;
//have definetely to be set by the application shaders (via string concatenation):

//applicable to following stages: all


/*
MUST be defined to values standing in file "2_Generic_C_includable_persistentDefines.glsl":
- RENDERING_TECHNIQUE 	
- LIGHT_SOURCES_SHADOW_FEATURE 	
- SHADOW_TECHNIQUE 	
- LIGHT_SOURCES_LIGHTING_FEATURE
- SHADER_LIGHTING_FEATURE 		

//MUST be defined to a positive integral number:
- NUM_INSTANCES
- NUM_LIGHT_SOURCES
- INV_NUM_LIGHT_SOURCES
- NUM_MULTISAMPLES
- INV_NUM_MULTISAMPLES

if the (RENDERING_TECHNIQUE == RENDERING_TECHNIQUE_DEFERRED_LIGHTING), then we
maybe want to access the G-buffer in several ways, depending on personal preferences and sophistication of the rendering technique
//if you render for a certain reason not to rectangle textures:
#define GBUFFER_TYPE 	TEXTURE_TYPE_2D_DEFAULT
//should be default for the G-buffer
#define GBUFFER_TYPE 	TEXTURE_TYPE_2D_RECT
//for global illumination effects via stencil routed k-buffering to produce kinda "layered depth images" ;) won't be implemented too soon ;(
#define GBUFFER_TYPE 	TEXTURE_TYPE_2D_MULTISAMPLE

//if rendering deferred, we have to specifiy if we want to render generic indices 
	(material index, gl_PrimitiveID, gl_InstanceID or what you might need) to an integer texture;
#define GBUFFER_INDEX_RENDERING
*/

//customizable defines have definetely to be set by the shaders (via string concatenation):
//{%customizabledefines}
//{
	//these are only examples:
	
	/*
	//the absolute minimal, for initial debugging:
	#define RENDERING_TECHNIQUE 				RENDERING_TECHNIQUE_DIRECT
	#define LIGHT_SOURCES_SHADOW_FEATURE 			LIGHT_SOURCES_SHADOW_FEATURE_NONE
	#define SHADOW_TECHNIQUE 				SHADOW_TECHNIQUE_NONE
	#define LIGHT_SOURCES_LIGHTING_FEATURE			LIGHT_SOURCES_LIGHTING_FEATURE_NONE
	#define SHADER_LIGHTING_FEATURE 			SHADER_LIGHTING_FEATURE_NONE


	//rather minimalistic phong: direct lighting without texturing or shadowing with one point light
	#define RENDERING_TECHNIQUE 				RENDERING_TECHNIQUE_DIRECT
	#define LIGHT_SOURCES_SHADOW_FEATURE 			LIGHT_SOURCES_SHADOW_FEATURE_NONE
	#define SHADOW_TECHNIQUE 				SHADOW_TECHNIQUE_NONE
	#define LIGHT_SOURCES_LIGHTING_FEATURE			LIGHT_SOURCES_LIGHTING_FEATURE_ONE_POINT_LIGHT
	#define SHADER_LIGHTING_FEATURE 			SHADER_LIGHTING_FEATURE_DIRECT_LIGHTING

	//amend the before with decal textureing
	#define RENDERING_TECHNIQUE 				RENDERING_TECHNIQUE_DIRECT
	#define LIGHT_SOURCES_SHADOW_FEATURE 			LIGHT_SOURCES_SHADOW_FEATURE_NONE
	#define SHADOW_TECHNIQUE 				SHADOW_TECHNIQUE_NONE
	#define LIGHT_SOURCES_LIGHTING_FEATURE			LIGHT_SOURCES_LIGHTING_FEATURE_ONE_POINT_LIGHT
	#define SHADER_LIGHTING_FEATURE 			SHADER_LIGHTING_FEATURE_DIRECT_LIGHTING | SHADER_LIGHTING_FEATURE_DECAL_TEXTURING

	
	//default classic phong rendering: nondeferred, one shadowmap, decal tex, one lightsource, quite conservative ;)	
	#define RENDERING_TECHNIQUE 				RENDERING_TECHNIQUE_DIRECT
	#define LIGHT_SOURCES_SHADOW_FEATURE 			LIGHT_SOURCES_SHADOW_FEATURE_ONE_SPOTLIGHT
	#define SHADOW_TECHNIQUE 				SHADOW_TECHNIQUE_DEFAULT
	#define LIGHT_SOURCES_LIGHTING_FEATURE			LIGHT_SOURCES_LIGHTING_FEATURE_ONE_SPOT_LIGHT
	#define SHADER_LIGHTING_FEATURE 			SHADER_LIGHTING_FEATURE_DIRECT_LIGHTING | SHADER_LIGHTING_FEATURE_DECAL_TEXTURING

	//normal mapping and all the rest to go when those initial stuff runs;
	*/


	//#define SHADOW_FEATURE_EXPERIMENTAL_SHADOWCOORD_CALC_IN_FRAGMENT_SHADER

	//#define SHADER_FEATURE_INSTANCING
	//#define NUM_INSTANCES 4
	//#define NUM_LIGHTSOURCES 4
//}
