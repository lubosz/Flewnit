//  material samplers:

//applicable to following stages: fragment

//{%materialSamplers}
//{

#if 	(RENDERING_TECHNIQUE == RENDERING_TECHNIQUE_DIRECT) \
     ||	(RENDERING_TECHNIQUE == RENDERING_TECHNIQUE_DEFERRED_GBUFFER_FILL)

	//following textures helping to determine the "unlit" fragment color to be lit or to be written to the Color buffer of the G-Buffer
	#if (SHADER_LIGHTING_FEATURE & SHADER_LIGHTING_FEATURE_DECAL_TEXTURING)
	uniform sampler2D decalTexture;
	#endif


	#if (SHADER_LIGHTING_FEATURE & SHADER_LIGHTING_FEATURE_NORMAL_MAPPING)
	uniform sampler2D normalMap;
	#endif

	#if (SHADER_LIGHTING_FEATURE & SHADER_LIGHTING_FEATURE_CUBE_MAPPING)
	uniform samplerCube cubeMap;
	#endif

#endif //end "unlit" stuff

//}
