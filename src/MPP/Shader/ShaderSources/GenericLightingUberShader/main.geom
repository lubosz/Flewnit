//Third concept of a geometry shader template, this time in grantlee template engine syntax;

{% comment %} 
  when do we need a geometry shader? 
    - for layered rendering, i.e.:
        - cube shadow map generation
        - dynamic cubemap rendering
        - multiple spotlight shadowmap generation
    - for god rays (won't be implemented too soon ;( )
    - we'll see ;)
{% endcomment %}

{% include  "00_Generic_Common_VersionTag.glsl" %}
{% include  "01_Generic_Common_precisionTag.glsl" %}

//like in the fragment template no #defines are necessary anymore, because the "preprocessor-masking" 
//is now done much more conveniently by the template engine
//we also need no special data types for fragment/geometry programs


layout(triangles) in;

{% if RENDER_TARGET_TEXTURE_TYPE_2D_CUBE or RENDER_TARGET_TEXTURE_TYPE_2D_CUBE_DEPTH %}
  //3 vertices per triangle, output 6 triangles (1 for each cubemap-face)
  layout(triangle_strip, max_vertices=18) out;
{% endif %}
{% if RENDERING_TECHNIQUE_SHADOWMAP_GENERATION and LIGHT_SOURCES_SHADOW_FEATURE_ALL_SPOT_LIGHTS %}
  layout(triangle_strip, max_vertices= 3 * {{  numMaxShadowCasters }} ) out;
{% endif %}
{% if RENDERING_TECHNIQUE_PRIMITIVE_ID_RASTERIZATION %}
    layout(triangle_strip, max_vertices= 3 ) out; //just use the geom shader to grab the primitive id;
{% endif %}


{% if RENDERING_TECHNIQUE_SHADOWMAP_GENERATION and LIGHT_SOURCES_SHADOW_FEATURE_ALL_SPOT_LIGHTS %}
  layout(shared) uniform ShadowCameraTransformBuffer
  {
    //we need only the view projection matrices, as no pure view space calculations are performed during shadow map generation
    uniform mat4 shadowCameraviewProjectionMatrices[ {{  numMaxShadowCasters }} ];
  };
{% else %}{% if RENDER_TARGET_TEXTURE_TYPE_2D_CUBE or RENDER_TARGET_TEXTURE_TYPE_2D_CUBE_DEPTH %}

    uniform mat4 cubeMapCameraViewMatrices[ 6 ];
    uniform mat4 cubeMapCameraViewProjectionMatrices[ 6 ];

{% endif %}{% endif %}


{% if RENDERING_TECHNIQUE_SHADOWMAP_GENERATION and LIGHT_SOURCES_SHADOW_FEATURE_ALL_SPOT_LIGHTS %}
  //number of lightsources casting shadows being currently active
  uniform int numCurrentlyActiveShadowCasters = {{ numMaxShadowCasters }} ;
  uniform float invNumCurrentlyActiveShadowShadowCasters =  {{ invNumMaxShadowCasters }} ;
{% endif %}
	uniform float cameraFarClipPlane = {{ cameraFarClipPlane }};
	uniform float invCameraFarClipPlane = {{ invCameraFarClipPlane }};

//---- shader interface ----------------------
//input from vertex/tessEval shader
in InterfaceData
{
  {% include  "09_Generic_InterfaceData.glsl" %}
} input;
//output to fragment shader
out InterfaceData
{
  {% include  "09_Generic_InterfaceData.glsl" %}
} output;
  
void main()
{
  //remember: every input is WORLD space transformed, now view/viewproj transform is performed for every layer
  
  {%comment%} ############################## begin cube map layered rendering stuff #################################### {%endcomment%}
  {% if RENDER_TARGET_TEXTURE_TYPE_2D_CUBE or RENDER_TARGET_TEXTURE_TYPE_2D_CUBE_DEPTH %}
            
    for(gl_Layer = 0; gl_Layer < 6; gl_Layer++ ) 
    {
      for(int vertexID = 0; vertexID < 3; vertexID ++ )
      {
            gl_Position = cubeMapCameraViewProjectionMatrices[gl_Layer] * gl_in[vertexID].input.position;
      
            {% if shaderPerformsColorCalculations %} 
              {% if VISUAL_MATERIAL_TYPE_SKYDOME_RENDERING %}
                //special case: neither world nor view space transform: set the box into centre of view, but don't rotate according to view direction
                output.position = gl_in[vertexID].input.position; // no further transform to view space stuff, kepp pseaudo world space
              {% else %}  
                output.position = cubeMapCameraViewMatrices[gl_Layer] * gl_in[vertexID].input.position;
              {% endif %}             
              output.normal = 	cubeMapCameraViewMatrices[gl_Layer] * gl_in[vertexID].input.normal;
              {% if SHADING_FEATURE_NORMAL_MAPPING %}
                output.tangent = cubeMapCameraViewMatrices[gl_Layer] * gl_in[vertexID].input.tangent;
              {% endif %}
              {% if texCoordsNeeded %}
                output.texCoords =  gl_in[vertexID].input.texCoords; //pass through
              {%endif%}    
              {% if SHADOW_FEATURE_EXPERIMENTAL_SHADOWCOORD_CALC_IN_FRAGMENT_SHADER and LIGHT_SOURCES_SHADOW_FEATURE_ONE_SPOT_LIGHT %}
                output.shadowCoord =  gl_in[vertexID].input.shadowCoord; //pass through
              {% endif %}
            {% endif %} 
           
              {% if depthButNotSpotLight %}
                //VIEW space transform
                //output.depthViewSpaceNORMALIZED =       vec4(cubeMapCameraViewMatrices[gl_Layer]  * gl_in[vertexID].input.position).z * invCameraFarClipPlane;
                //output.depthViewSpaceUNSCALED =         vec4(cubeMapCameraViewMatrices[gl_Layer]  * gl_in[vertexID].input.position).z;
                //output.positionViewSpaceNORMALIZED =        (cubeMapCameraViewMatrices[gl_Layer]  * gl_in[vertexID].input.position) * invCameraFarClipPlane;
                output.positionViewSpaceUNSCALED =        cubeMapCameraViewMatrices * gl_in[vertexID].input.position; //TODO check the optimized data pass variants when stable;
              {% endif %}     
               
            EmitVertex();   
       }     
       EndPrimitive();
     }     
  {% endif %}  
   
  {%comment%} ############################## begin texture array layered shadowmap rendering stuff #################### {%endcomment%}            
  {% if RENDERING_TECHNIQUE_SHADOWMAP_GENERATION and LIGHT_SOURCES_SHADOW_FEATURE_ALL_SPOT_LIGHTS %}

     for(gl_Layer = 0; gl_Layer < {{  numMaxShadowCasters }}; gl_Layer++ ) //compile time fixed length hard code loop; 
    //for(gl_Layer = 0; gl_Layer < numCurrentlyActiveShadowShadowCasters ; gl_Layer++ )  //TODO try out uniform controlled dynamic loop when stable
    {
      for(int vertexID = 0; vertexID < 3; vertexID ++ )
      {            
            gl_Position = cubeMapCameraViewProjectionMatrices[gl_Layer] * gl_in[vertexID].input.position; 
            EmitVertex();   
       }     
       EndPrimitive();
     } 
     
  {% endif %}  
     
  {%comment%} ############################## begin primitive ID grabbing stuff ;) ##################################### {%endcomment%}
  {%comment%} ################################# following ID inputs ##############################################################{%endcomment%}
  {% if RENDERING_TECHNIQUE_PRIMITIVE_ID_RASTERIZATION or instancedRendering %}
      output.genericIndices= input.genericIndices;
      output.genericIndices.x= gl_PrimitiveID;
  {% endif %}
    
} //end main

