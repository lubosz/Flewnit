//Third concept of a fragment shader template, this time in grantlee template engine syntax;
{% include  "00_Generic_Common_VersionTag.glsl" %}
{% include  "01_Generic_Common_precisionTag.glsl" %}
//data type definition
{% include  "./04_Fragment_appCodeIncludable_dataTypes.glsl" %}
//---- interface ---------------------------------------------------------------------------------
//---- application (uniform) input ----
{% include  "./05_Fragment_materialSamplers.glsl" %}
{% include  "./06_Fragment_shadowMapSamplers.glsl" %}
{% include  "./07_Fragment_GBufferSamplers.glsl" %}
{% include  "./08_Fragment_Uniforms.glsl" %}
//---- shader input --------------------
{% include  "./09_Fragment_input.glsl" %}
//---- shader output -------------------
{% include  "./10_Fragment_output.glsl" %}
//----- subroutines ------------------------------------------------------------------------------
{% include  "./11_Fragment_subroutine_getDistanceAttenuation.glsl" %}
{% include  "./11_Fragment_subroutine_getNormal.glsl" %}
{% include  "./11_Fragment_subroutine_getShadowAttenuation.glsl" %}
{% include  "./11_Fragment_subroutine_getSpotLightAttenuation.glsl" %}



void main()
{

{%comment%} ################################# following "coloring" code: shading and GBuffer fill ######################################### {%endcomment%}

{% if RENDERING_TECHNIQUE_DEFAULT_LIGHTING or RENDERING_TECHNIQUE_TRANSPARENT_OBJECT_LIGHTING  or RENDERING_TECHNIQUE_DEFERRED_GBUFFER_FILL or RENDERING_TECHNIQUE_DEFERRED_LIGHTING %}
  
  {% if VISUAL_MATERIAL_TYPE_DEBUG_DRAW_ONLY %}
    outFFinalLuminance = vec4(1.0,0.0,1.0,0.0); //some funny debug draw color^^
    return;
  {% else %} {% if VISUAL_MATERIAL_TYPE_SKYDOME_RENDERING %}
    {% if not SHADING_FEATURE_CUBE_MAPPING %} what the fuck, add a cube map! {% endif %}
    //one of those exceptions: for cuba mapping: we have to pass WORLD coords and not VIEW coords!11
    outFFinalLuminance = texture(cubeMap, normalize( (-1.0) * inFPosition.xyz));
    //TODO try when stable non normalized lookup:
    // outFFinalLuminance = texture(cubeMap,  (-1.0) * inFPosition.xyz );
    return;
  {% else %} 
  
    {%comment%} now the common stuff begins {%endcomment%}
     
    //codeFragment_initNonDefLightingSampleValues
    //{
    {% if not RENDERING_TECHNIQUE_DEFERRED_LIGHTING  %} 
      {%comment%} get the fragment values in the classical way {%endcomment%}
      vec3 normalVN = getNormal(0); //sampleindex zero, as no multisampling is used
      vec4 fragmentColor =  {% if SHADING_FEATURE_DECAL_TEXTURING %}  texture(decalTexture,inFTexCoords.xy);
                            {% else %} vec4(1.0,1.0,1.0,1.0);
                            {% endif %}  
    {% endif %}
    //} //end codeFragment_initNonDefLightingSampleValues
    
    
    {% if RENDERING_TECHNIQUE_DEFERRED_GBUFFER_FILL  %}
      //we have all relevant values, now flush the GBuffer;
      outFGBufferPosition = inFPosition; //TODO NOT write position out this wasting way: try writing gl_FragDepth or single floating point texture instead when this default way is stable works
      outFGBufferNormal = vec4(normalVN,0.0);   //TODO NOT write normal   out this wasting way: try writing to two-channel normalized 8bit signed int texture instead when this default way is stable works out
      outFGBufferColor = vec4(fragmentColor.xyz, shininess);  //code shininess into alph channel 
    {% else %}
    
      {%comment%} now iterate over all lights and fragment samples and perform lighting calculations {%endcomment%}
    
      outFFinalLuminance = vec4(0.0,0.0,0.0,0.0); //init to zero as it will be accumulated over samples and lightsources
      vec3 fragToCamN = normalize( (-1.0) * inFPosition);   //vec3 fragToCamN = normalize(eyePosition_WS - inFPosition); <--legacy worldspace code
       
               
      {% if RENDERING_TECHNIQUE_DEFERRED_LIGHTING %}
        //{############### begin outer samples loop ####################################################################
        {%comment%} get the fragment values from the (possibly multisampled) G-Buffer {%endcomment%}
        {% include "./12_Fragment_codeSnippet_beginGBufferSampleIteration.glsl" %}
      {% endif %} 
        
                  {%comment%} no matter if we are doing deferred or direct lighting now, the calculations are the same  {%endcomment%}
                  
                  {% if not SHADING_FEATURE_DIRECT_LIGHTING %}
                      outFFinalLuminance += fragmentColor; //only accum fragment colors without shading    
                  {% else %}
                  
                       vec4 incidentLight = vec4(0.0,0.0,0.0,0.0); //variable to accum all light for one sample
                       
                     
                       {% if LIGHT_SOURCES_LIGHTING_FEATURE_ALL_POINT_LIGHTS or LIGHT_SOURCES_LIGHTING_FEATURE_ALL_SPOT_LIGHTS or LIGHT_SOURCES_LIGHTING_FEATURE_ALL_POINT_OR_SPOT_LIGHTS %}
                         //there was once a bug in the driver preventing variable lenght-loops; TODO check out if it works now after the rest has been veryfied:
                         //{############### begin inner lighting loop #######################################################
                         //for(int lightIndex = 0; lightIndex < numCurrentlyActiveLightSources ;lightIndex++)
                         for(int lightIndex = 0; lightIndex < NUM_LIGHTSOURCES ;lightIndex++) //<-- hard coded, precompiled loop termination condition variable
                          {
                              //lets hope that there will be a component wise copy and NOT some C++-f***up about non-existing operator=() ;)
                               LightSource lightSource = lightSources[lightIndex]; //copy from uniform buffer to shared memory and name this variable like the uniform variable in the one-lightsource- context
                      {% endif %}
                      
                              //{~~~~ begin per sample, per lightsource calculations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                              {% include "./12_Fragment_codeSnippet_performLightingCalculationsPerFragmentPerLight.glsl" %}
                              //}~~~~ end   per sample, per lightsource calculations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

                      {% if LIGHT_SOURCES_LIGHTING_FEATURE_ALL_POINT_LIGHTS or LIGHT_SOURCES_LIGHTING_FEATURE_ALL_SPOT_LIGHTS or LIGHT_SOURCES_LIGHTING_FEATURE_ALL_POINT_OR_SPOT_LIGHTS %}
                          } //end of for lightsources-loop
                          //}############### end   inner lighting loop #######################################################
                      {% endif %}  
                      

                      outFFinalLuminance += incidentLight * fragmentColor;  //accum the incident light for all lightsources, multiplicated with th color of the current fragment;
                                                                            //do this for every sample      
                     
                      
                  {% endif %} {%comment%} end distinction between if lighting the samples or not {%endcomment%}
            
      {% if RENDERING_TECHNIQUE_DEFERRED_LIGHTING %}
        {%comment%} get the fragment values from the (possibly multisampled) G-Buffer {%endcomment%}
        } //end of for multisamples-loop; see its beginning in "./12_Fragment_codeSnippet_beginGBufferSampleIteration.glsl"
        //divide accumulated luminance by numSamples:
        if(totalValidMultiSamples >0.0) { outFFinalLuminance /= totalValidMultiSamples};
        //}############### end   outer samples loop ####################################################################   
      {% endif %} 
      
      
      {% if SHADING_FEATURE_CUBE_MAPPING and RENDERING_TECHNIQUE_DEFAULT_LIGHTING %} 
        //lerp between actual color and cubemap color; only possible for default lighting; for deferred lighting, the fact that it is a cube map material
        //cannot be handled in a clean way without wasting much memory; hence, like transparent materials, cube mapped geometry may no be rendered deferred
        fragmentColor = mix(   outFFinalLuminance, texture(cubeMap, normalVN), cubeMapReflectivity        );
      {% endif %}
   
    
    {% endif %}     {%comment%} end distinction between G-buffer fill and lighting  {%endcomment%}
         

  {% endif %}{% endif %}   {%comment%} endifs from debug draw and skydome {%endcomment%}

{% endif %} {%comment%} end "coloring" stuff {%endcomment%}

{%comment%} ################################# following shadow/pos/depth generation code ################################################## {%endcomment%}


{%comment%} ################################# following ID generation code ##############################################################{%endcomment%}

} //end main
