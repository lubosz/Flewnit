/*
 * Loader.cpp
 *
 *  Created on: Dec 6, 2010
 *      Author: tychi
 */

#include "Loader.h"

#include "LoaderHelper.h"

#include <tinyxml.h>

#include "Buffer/Texture.h"
#include <FreeImagePlus.h>


#include <boost/lexical_cast.hpp>
#include "Buffer/BufferHelperUtils.h"
//#include "WorldObject/Box.h"
#include "Simulator/SimulationResourceManager.h"
#include "Scene/Scene.h"
#include "Simulator/LightingSimulator/Light/LightSourceManager.h"
#include "Buffer/BufferSharedDefinitions.h"
#include "Util/Loader/Loader.h"
#include "MPP/Shader/ShaderManager.h"
#include "WorldObject/PureVisualObject.h"
#include "WorldObject/SubObject.h"
#include "URE.h"
#include "Geometry/Procedural/BoxGeometry.h"
#include "Util/HelperFunctions.h"
#include "WorldObject/InstanceManager.h"
#include "WorldObject/SkyDome.h"
#include "UserInterface/WindowManager/WindowManager.h"



namespace Flewnit
{

Loader::Loader()
{
	// TODO Auto-generated constructor stub

}

Loader::~Loader()
{
	// TODO Auto-generated destructor stub
}

void Loader::loadScene()
{
	createHardCodedSceneStuff();

}


//DEBUG stuff:
void Loader::createHardCodedSceneStuff()
{
	bool createTesselationStuffIfTechnicallyPossible = true;

	SceneNode& rootNode = SimulationResourceManager::getInstance().getScene()->root();
	rootNode.addChild(
		new SkyDome( Path("./assets/textures/cubeMaps/"),"cloudy_noon","jpg")
	);

	//create the first rendering, to see anything and to test the camera, the buffers, the shares and to overall architectural frame:
	//TODO
	rootNode.addChild(
		new PureVisualObject("MyBox1",AmendedTransform(Vector3D(0,-10,-30), Vector3D(0.0,0.1,-1.0)))
	);
	rootNode.addChild(
		new PureVisualObject("myTessBox1",AmendedTransform(Vector3D(90,-10,50), Vector3D(0.0f,0.9f,0.1f),Vector3D(0,0,1),0.5))
	);

	rootNode.addChild(
		new PureVisualObject("myBlackNWhiteBox",AmendedTransform(Vector3D(-90,-10,50), Vector3D(0.0f,0.9f,0.1f),Vector3D(0,0,1),1.0))
	);

	rootNode.addChild(
		new PureVisualObject("MyBoxAsPlane",AmendedTransform(Vector3D(0,-40,0), Vector3D(0,0,-1),Vector3D(0,1,0),3.0f))
	);
	rootNode.addChild(
		new PureVisualObject("myEnvmapBox",AmendedTransform(Vector3D(100,20,-80) )) //, Vector3D(0.0f,0.9f,0.1f),Vector3D(0,0,1)))
	);

	Geometry* geo1 = SimulationResourceManager::getInstance().getGeometry("MyBox1");
	if(! geo1)
	{
		geo1 = new BoxGeometry("MyBox1",Vector3D(30.0f,7.0f,150.0f),true,
				//patch stuff only for GL versions 4.0 or higher
				false
				//(WindowManager::getInstance().getAvailableOpenGLVersion().x >= 4)
		);
	}
	Geometry* tessGeo = SimulationResourceManager::getInstance().getGeometry("myTessBox1");
	if(! tessGeo)
	{
		tessGeo = new BoxGeometry("myTessBox1",Vector3D(10.0f,10.0f,10.0f),true,
				//patch stuff only for GL versions 4.0 or higher
				createTesselationStuffIfTechnicallyPossible
				 ? (WindowManager::getInstance().getAvailableOpenGLVersion().x >= 4)
			     : false
		);
	}


	Geometry* blackNWhiteGeo = SimulationResourceManager::getInstance().getGeometry("blackNWhiteGeo");
	if(! blackNWhiteGeo)
	{
		blackNWhiteGeo = new BoxGeometry("blackNWhiteGeo",Vector3D(20.0f,20.0f,20.0f),true, false);
	}


	Geometry* geo3 = SimulationResourceManager::getInstance().getGeometry("MyBoxAsPlane");
	if(! geo3)
	{
		geo3 = new BoxGeometry("MyBoxAsPlane",Vector3D(100.0f,2.0f,100.0f),true,
				//patch stuff only for GL versions 4.0 or higher
				false,
				//(WindowManager::getInstance().getAvailableOpenGLVersion().x >= 4)
				Vector4D(20,20,1,1)
		);
	}



	Material* mat1 = SimulationResourceManager::getInstance().getMaterial("BunnyDecalMaterial");
	if(! mat1)
	{
		Texture* decalTex= URE_INSTANCE->getLoader()->loadTexture(
				String("bunnyDecalTex"),
				DECAL_COLOR_SEMANTICS,
				Path("./assets/textures/bunny.png"),
				BufferElementInfo(4,GPU_DATA_TYPE_UINT,8,true),
				true,
				false,
				true
		);

		std::map<BufferSemantics,Texture*> myMap;
		myMap[DECAL_COLOR_SEMANTICS] = decalTex;

		mat1 = new VisualMaterial("BunnyDecalMaterial",
			//VISUAL_MATERIAL_TYPE_DEBUG_DRAW_ONLY, SHADING_FEATURE_NONE,
			VISUAL_MATERIAL_TYPE_DEFAULT_LIGHTING,
			ShadingFeatures(
					SHADING_FEATURE_DIRECT_LIGHTING
					| SHADING_FEATURE_DECAL_TEXTURING
			),
			myMap,
			//SHADING_FEATURE_DIRECT_LIGHTING,
			//std::map<BufferSemantics,Texture*>(),
			VisualMaterialFlags(true,false,true,true,false,false),
			1000.0f,
			0.5f
		);
	}//endif !mat1


	Material* mat2 = SimulationResourceManager::getInstance().getMaterial("StoneBumpMaterial");
	if(! mat2)
	{
		Texture* decalTex= URE_INSTANCE->getLoader()->loadTexture(
				String("rockbumpDecal"),
				DECAL_COLOR_SEMANTICS,
				Path("./assets/textures/rockbumpDecal.jpg"),
				BufferElementInfo(4,GPU_DATA_TYPE_UINT,8,true),
				true,
				false,
				true
		);
		Texture* normalMap= URE_INSTANCE->getLoader()->loadTexture(
				String("rockbumpNormalDisp"),
				//DECAL_COLOR_SEMANTICS,
				NORMAL_SEMANTICS,
				Path("./assets/textures/rockbumpNormalDisp.png"),
				BufferElementInfo(4,GPU_DATA_TYPE_UINT,8,true),
				true,
				false,
				true
		);
		std::map<BufferSemantics,Texture*> myMap;
		myMap[DECAL_COLOR_SEMANTICS] = decalTex;
		myMap[NORMAL_SEMANTICS] = normalMap;
		//myMap[DECAL_COLOR_SEMANTICS] = normalMap;
		//myMap[DECAL_COLOR_SEMANTICS] = SimulationResourceManager::getInstance().getTexture("bunnyDecalTex");
		mat2 = new VisualMaterial("StoneBumpMaterial",
			//VISUAL_MATERIAL_TYPE_DEBUG_DRAW_ONLY, SHADING_FEATURE_NONE,
			VISUAL_MATERIAL_TYPE_DEFAULT_LIGHTING,
			ShadingFeatures(
					SHADING_FEATURE_DIRECT_LIGHTING
					| SHADING_FEATURE_DECAL_TEXTURING
					| SHADING_FEATURE_NORMAL_MAPPING
			),
			myMap,
			//SHADING_FEATURE_DIRECT_LIGHTING,
			//std::map<BufferSemantics,Texture*>(),
			VisualMaterialFlags(true,false,true,true,false,false),
			5.0f,
			0.1f
		);
	}//endif !mat2

	Material* stoneBumpTessMat = SimulationResourceManager::getInstance().getMaterial("StoneBumpTessMaterial");
	if(! stoneBumpTessMat)
	{
		std::map<BufferSemantics,Texture*> myMap;
		myMap[DECAL_COLOR_SEMANTICS] = SimulationResourceManager::getInstance().getTexture("rockbumpDecal");
		myMap[NORMAL_SEMANTICS] = SimulationResourceManager::getInstance().getTexture("rockbumpNormalDisp");
		Texture* dispTex =  SimulationResourceManager::getInstance().getTexture("rockbumpDisp.png");
		if(!dispTex)
		{
			dispTex= URE_INSTANCE->getLoader()->loadTexture(
				String("rockbumpDisp.png"),
				DECAL_COLOR_SEMANTICS,
				Path("./assets/textures/rockbumpDisp.png"),
				BufferElementInfo(1,GPU_DATA_TYPE_UINT,8,true),
				true,
				false,
				true
		 	 );
		}
		myMap[DISPLACEMENT_SEMANTICS] = dispTex;
		//myMap[DECAL_COLOR_SEMANTICS] = normalMap;
		//myMap[DECAL_COLOR_SEMANTICS] = SimulationResourceManager::getInstance().getTexture("bunnyDecalTex");
		stoneBumpTessMat = new VisualMaterial("StoneBumpTessMaterial",
			//VISUAL_MATERIAL_TYPE_DEBUG_DRAW_ONLY, SHADING_FEATURE_NONE,
			VISUAL_MATERIAL_TYPE_DEFAULT_LIGHTING,
			ShadingFeatures(
					SHADING_FEATURE_DIRECT_LIGHTING
					| SHADING_FEATURE_DECAL_TEXTURING
					| SHADING_FEATURE_NORMAL_MAPPING
					| (createTesselationStuffIfTechnicallyPossible ? SHADING_FEATURE_TESSELATION :0)
			),
			myMap,
			VisualMaterialFlags(true,false,true,true,false,false),
			100.0f,
			0.4f
		);
	}//endif !stoneBumpTessMat


	Material* blackNWhiteMat = SimulationResourceManager::getInstance().getMaterial("blackNWhiteMat");
	if(! blackNWhiteMat)
	{
		Texture* decalTex =  SimulationResourceManager::getInstance().getTexture("rockwallDispDecalTest");
		if(!decalTex)
		{
		 	 decalTex= URE_INSTANCE->getLoader()->loadTexture(
				String("rockwallDispDecalTest"),
				DECAL_COLOR_SEMANTICS,
//				Path("./assets/textures/rockbumpDisp.png"),
//				BufferElementInfo(1,GPU_DATA_TYPE_UINT,8,true),
				Path("./assets/blendfiles/raptor/textures/raptorDisp_32f.exr"),
				BufferElementInfo(1,GPU_DATA_TYPE_FLOAT,32,false),
				true,
				false,
				true
		 	 );
		}
		std::map<BufferSemantics,Texture*> myMap;
		myMap[DECAL_COLOR_SEMANTICS] = decalTex;
		blackNWhiteMat = new VisualMaterial("blackNWhiteMat",
			//VISUAL_MATERIAL_TYPE_DEBUG_DRAW_ONLY, SHADING_FEATURE_NONE,
			VISUAL_MATERIAL_TYPE_DEFAULT_LIGHTING,
			ShadingFeatures(
					SHADING_FEATURE_DIRECT_LIGHTING
					| SHADING_FEATURE_DECAL_TEXTURING
			),
			myMap,
			VisualMaterialFlags(true,false,true,true,false,false),
			1000.0f,
			0.5f
		);
	}//endif !blackNWhiteMat


	Material* matEnvMap = SimulationResourceManager::getInstance().getMaterial("EnvMapCloudyNoonMaterial");
	if(! matEnvMap)
	{
		Texture* envMapTex =
				SimulationResourceManager::getInstance().getTexture("cloudy_noon");
		if(!envMapTex)
		{
			envMapTex= URE_INSTANCE->getLoader()->loadCubeTexture(
				String("cloudy_noon"),
				ENVMAP_SEMANTICS,
				Path("./assets/textures/cubeMaps/cloudy_noon"),
				String("jpg"),
				BufferElementInfo(4,GPU_DATA_TYPE_UINT,8,true),
				true,
				false
			);
		}
//		Texture* decalTex= URE_INSTANCE->getLoader()->loadTexture(
//				String("rockwallDecal"),
//				DECAL_COLOR_SEMANTICS,
//				Path("./assets/textures/rockwallDecal.jpg"),
//				BufferElementInfo(4,GPU_DATA_TYPE_UINT,8,true),
//				true,
//				false,
//				true
//		);
//		Texture* normalMap= URE_INSTANCE->getLoader()->loadTexture(
//				String("rockwallNormalDisp"),
//				//DECAL_COLOR_SEMANTICS,
//				NORMAL_SEMANTICS,
//				Path("./assets/textures/rockwallNormalDisp.png"),
//				BufferElementInfo(4,GPU_DATA_TYPE_UINT,8,true),
//				true,
//				false,
//				true
//		);

		std::map<BufferSemantics,Texture*> myMap;
		myMap[ENVMAP_SEMANTICS] = envMapTex;
		myMap[DECAL_COLOR_SEMANTICS] = SimulationResourceManager::getInstance().getTexture("rockbumpDecal");
		myMap[NORMAL_SEMANTICS] = SimulationResourceManager::getInstance().getTexture("rockbumpNormalDisp");

		matEnvMap = new VisualMaterial("EnvMapCloudyNoonMaterial",
			//VISUAL_MATERIAL_TYPE_DEBUG_DRAW_ONLY, SHADING_FEATURE_NONE,
			VISUAL_MATERIAL_TYPE_DEFAULT_LIGHTING,
			ShadingFeatures(
					SHADING_FEATURE_DIRECT_LIGHTING
					| SHADING_FEATURE_DECAL_TEXTURING
					| SHADING_FEATURE_NORMAL_MAPPING
					| SHADING_FEATURE_CUBE_MAPPING
			),
			myMap,
			//not dyn. cubemap renderable
			VisualMaterialFlags(true,false,true,false,false,false));
	}//endif !matEnvMap



	dynamic_cast<WorldObject*>(rootNode.getChild("MyBox1"))->addSubObject(
		new SubObject(
			"BoxSubObject1",
			VISUAL_SIM_DOMAIN,
			geo1,
			mat1
		)
	);
	dynamic_cast<WorldObject*>(rootNode.getChild("myTessBox1"))->addSubObject(
		new SubObject(
			"myTessBox1SO",
			VISUAL_SIM_DOMAIN,
			tessGeo,
			stoneBumpTessMat
		)
	);

	dynamic_cast<WorldObject*>(rootNode.getChild("myBlackNWhiteBox"))->addSubObject(
		new SubObject(
			"myBlackNWhiteBoxSO",
			VISUAL_SIM_DOMAIN,
			blackNWhiteGeo,
			blackNWhiteMat
		)
	);


	dynamic_cast<WorldObject*>(rootNode.getChild("MyBoxAsPlane"))->addSubObject(
		new SubObject(
			"MyBoxAsPlaneSubObject1",
			VISUAL_SIM_DOMAIN,
			geo3,
			mat2
		)
	);
	dynamic_cast<WorldObject*>(rootNode.getChild("myEnvmapBox"))->addSubObject(
		new SubObject(
			"MyEnvmapBoxSubObject1",
			VISUAL_SIM_DOMAIN,
			new BoxGeometry("myEnvMapBox",Vector3D(20,50,30),true,false),
			matEnvMap
		)
	);

	//---------------------- begin instanced geometry -----------------------------------
	Geometry* boxForInstancingGeom = SimulationResourceManager::getInstance().getGeometry("boxForInstancingGeom");
	if(! boxForInstancingGeom)
	{
		boxForInstancingGeom = new BoxGeometry("boxForInstancingGeom",Vector3D(5.0f,25.0f,25.0f),true,
				//patch stuff only for GL versions 4.0 or higher
				createTesselationStuffIfTechnicallyPossible
				 ? (WindowManager::getInstance().getAvailableOpenGLVersion().x >= 4)
			     : false
		);
	}



	Material* matInstanced = SimulationResourceManager::getInstance().getMaterial("instancedRockWallMaterial");
	if(! matInstanced)
	{
		Texture* decalTex= URE_INSTANCE->getLoader()->loadTexture(
				String("rockWallDecal"),
				DECAL_COLOR_SEMANTICS,
				Path("./assets/textures/rockwallDecal.jpg"),
				BufferElementInfo(4,GPU_DATA_TYPE_UINT,8,true),
				true,
				false,
				true
		);
		Texture* normalMap= URE_INSTANCE->getLoader()->loadTexture(
				String("rockWallNormalDisp"),
				//DECAL_COLOR_SEMANTICS,
				NORMAL_SEMANTICS,
				Path("./assets/textures/rockwallNormalDisp.png"),
				BufferElementInfo(4,GPU_DATA_TYPE_UINT,8,true),
				true,
				false,
				true
		);

		Texture* dispTex= URE_INSTANCE->getLoader()->loadTexture(
			String("rockwallDisp.png"),
			DISPLACEMENT_SEMANTICS,
			Path("./assets/textures/rockwallDisp.png"),
			BufferElementInfo(1,GPU_DATA_TYPE_UINT,8,true),
			true,
			false,
			true
	 	 );


		std::map<BufferSemantics,Texture*> myMap;
		myMap[DECAL_COLOR_SEMANTICS] = decalTex;
		myMap[NORMAL_SEMANTICS] = normalMap;
		myMap[DISPLACEMENT_SEMANTICS] = dispTex;
		matInstanced = new VisualMaterial("instancedRockWallMaterial",
			//VISUAL_MATERIAL_TYPE_DEBUG_DRAW_ONLY, SHADING_FEATURE_NONE,
			VISUAL_MATERIAL_TYPE_DEFAULT_LIGHTING,
			ShadingFeatures(
					SHADING_FEATURE_DIRECT_LIGHTING
					| SHADING_FEATURE_DECAL_TEXTURING
					| SHADING_FEATURE_NORMAL_MAPPING
					| (createTesselationStuffIfTechnicallyPossible ? SHADING_FEATURE_TESSELATION :0)
			),
			myMap,
			VisualMaterialFlags(true,false,true,true,
					true,//YES, is instanced!
					false),
			5.0f,
			0.1f
		);
	}//endif !matInstanced

	SubObject* drawableBoxSOforInstancing = new SubObject(
				"drawableBoxSOforInstancing",
				VISUAL_SIM_DOMAIN,
				boxForInstancingGeom,
				matInstanced
			);

	int numInstancesPerDimension = 6;
	InstanceManager* boxInstanceManager = new InstanceManager(
			"boxInstanceManager",
			drawableBoxSOforInstancing
	);


	SceneNode* instancesParentNode = rootNode.addChild(
			new SceneNode(
				"boxInstanceArmyParent",
				PURE_NODE,
				AmendedTransform(
					Vector3D(-80,0,-200),
					Vector3D(0.2,-0.6,-0.8),
					Vector3D(0,1,0),
					0.5
				)
			)
	);
	//Vector3D instanceArmyPosOffset(100,100,-200);
	Vector3D instanceArmyStride(50,50,50);

	for(int x= -numInstancesPerDimension/2; x< numInstancesPerDimension/2; x++)
	{
		for(int y= -numInstancesPerDimension/2; y< numInstancesPerDimension/2; y++)
		{
			for(int z= -numInstancesPerDimension/2; z< numInstancesPerDimension/2; z++)
			{
				PureVisualObject* pvo = new PureVisualObject(
					String("myInstancingTestBoxWO")
						+ String("x") + HelperFunctions::toString(x)
						+ String("y") + HelperFunctions::toString(y)
						+ String("z") + HelperFunctions::toString(z),
					AmendedTransform(
						/*instanceArmyPosOffset +*/ (instanceArmyStride * Vector3D(x,y,z))
						//Vector3D(0.3f,0.0f,-1.0f)
					)
				);
				pvo->addSubObject(boxInstanceManager->createInstance());
				instancesParentNode->addChild(pvo);
			}
		}
	}




	//---------------------- begin light sources -----------------------------------

	if(
		(ShaderManager::getInstance().getGlobalShaderFeatures().lightSourcesLightingFeature
				== LIGHT_SOURCES_LIGHTING_FEATURE_ONE_POINT_LIGHT )
		||
		(ShaderManager::getInstance().getGlobalShaderFeatures().lightSourcesLightingFeature
				== LIGHT_SOURCES_LIGHTING_FEATURE_ALL_POINT_LIGHTS )
	)
	{
		LightSourceManager::getInstance().createPointLight(
				Vector4D(70.0f,45.0f,10.0f,1.0f),
				false,
				Vector4D(1.0f,1.0f,1.0f,1.0f),
				Vector4D(1.0f,1.0f,1.0f,1.0f)
		);

		if(
			(ShaderManager::getInstance().getGlobalShaderFeatures().lightSourcesLightingFeature
					== LIGHT_SOURCES_LIGHTING_FEATURE_ALL_POINT_LIGHTS )
		)
		{
			if(ShaderManager::getInstance().getGlobalShaderFeatures().numMaxLightSources >= 2)
			{
				LightSourceManager::getInstance().createPointLight(
					Vector4D(-70.0f,25.0f,10.0f,1.0f),
					false,
					Vector4D(0.0f,0.0f,1.0f,1.0f),
					Vector4D(0.0f,0.0f,1.0f,1.0f)
				);
			}

			//two lights already created, hence the -2
			int numTotalLightSourcesToCreate = ShaderManager::getInstance().getGlobalShaderFeatures().numMaxLightSources -2;
			for(int i = 1; i <= numTotalLightSourcesToCreate; i++)
			{
				float fraction = (float)(i)/(float)(numTotalLightSourcesToCreate);
				LightSourceManager::getInstance().createPointLight(
					Vector4D(0.0f,15.0f,240.0f + (-480.0f) * fraction ,1.0f) ,
					false,
					Vector4D(0.0f,0.2f,0.0f,1.0f)/ numTotalLightSourcesToCreate,
					Vector4D((1.0f-fraction)*10.0f,fraction*50, fraction*50 ,1.0f)/ numTotalLightSourcesToCreate
				);
			}
		}
	}

	if(
		(ShaderManager::getInstance().getGlobalShaderFeatures().lightSourcesLightingFeature
				== LIGHT_SOURCES_LIGHTING_FEATURE_ONE_SPOT_LIGHT )
		||
		(ShaderManager::getInstance().getGlobalShaderFeatures().lightSourcesLightingFeature
				== LIGHT_SOURCES_LIGHTING_FEATURE_ALL_SPOT_LIGHTS )
	)
	{
		LightSourceManager::getInstance().createSpotLight(
				Vector4D(70.0f,45.0f,10.0f,1.0f),
				Vector4D(0.0f,-1.0f,-0.2f,0.0f),
				false,
				45.0f,
				47.5f,
				50.0f,
				Vector4D(1.0f,1.0f,1.0f,1.0f),
				Vector4D(1.0f,1.0f,1.0f,1.0f)
		);

		if(
			(ShaderManager::getInstance().getGlobalShaderFeatures().lightSourcesLightingFeature
				 == LIGHT_SOURCES_LIGHTING_FEATURE_ALL_SPOT_LIGHTS )
		)
		{
			if(ShaderManager::getInstance().getGlobalShaderFeatures().numMaxLightSources >= 2)
			{
				LightSourceManager::getInstance().createSpotLight(
						Vector4D(-70.0f,25.0f,10.0f,1.0f),
						Vector4D(1.0f,-1.0f,-1.0f,0.0f),
						false,
						45.0f,
						60.5f,
						10.0f,
						Vector4D(0.0f,0.0f,1.0f,1.0f),
						Vector4D(0.0f,0.0f,1.0f,1.0f)
				);
			}

			//two lights already created, hence the -2
			int numTotalLightSourcesToCreate = ShaderManager::getInstance().getGlobalShaderFeatures().numMaxLightSources -2;
			for(int i = 1; i <= numTotalLightSourcesToCreate; i++)
			{
				float fraction = (float)(i)/(float)(numTotalLightSourcesToCreate);
				LightSourceManager::getInstance().createSpotLight(
					Vector4D(0.0f,15.0f,240.0f + (-480.0f) * fraction ,1.0f) ,
					Vector4D(0.0f,-1.0f,0.0f,0.0f),
					false,
					45.0f,
					47.5f,
					10.0f,
					Vector4D(0.0f,0.2f,0.0f,1.0f)/ numTotalLightSourcesToCreate,
					Vector4D((1.0f-fraction)*10.0f,fraction*50, fraction*50 ,1.0f)/ numTotalLightSourcesToCreate
				);
			}
		}
	}




	if(
			(ShaderManager::getInstance().getGlobalShaderFeatures().lightSourcesLightingFeature
								 == LIGHT_SOURCES_LIGHTING_FEATURE_ALL_POINT_OR_SPOT_LIGHTS )
	)
	{
		LightSourceManager::getInstance().createPointLight(
				Vector4D(70.0f,90.0f,10.0f,1.0f),
				false,
				Vector4D(1.0f,1.0f,1.0f,1.0f)*0.25,
				Vector4D(1.0f,1.0f,1.0f,1.0f)*0.25
		);
		LightSourceManager::getInstance().createSpotLight(
				Vector4D(-70.0f,25.0f,10.0f,1.0f),
				Vector4D(1.0f,-1.0f,-1.0f,0.0f),
				false,
				45.0f,
				60.5f,
				10.0f,
				Vector4D(0.0f,0.0f,1.0f,1.0f),
				Vector4D(0.0f,0.0f,1.0f,1.0f)
		);

		//two lights already created, hence the -2
		int numTotalLightSourcesToCreate = ShaderManager::getInstance().getGlobalShaderFeatures().numMaxLightSources -2;
		for(int i = 1; i <= numTotalLightSourcesToCreate; i++)
		{
			float fraction = (float)(i)/(float)(numTotalLightSourcesToCreate);
			if(i%2)
			{
				LightSourceManager::getInstance().createPointLight(
					Vector4D(0.0f,30.0f,240.0f + (-480.0f) * fraction ,1.0f) ,
					false,
					Vector4D(1.0f,1.2f,0.0f,1.0f)/ numTotalLightSourcesToCreate,
					Vector4D((1.0f-fraction)*10.0f,fraction*50, fraction*50 ,1.0f)/ numTotalLightSourcesToCreate
				);
			}
			else
			{
				LightSourceManager::getInstance().createSpotLight(
						Vector4D(0.0f,30.0f,240.0f + (-480.0f) * fraction ,1.0f) ,
						Vector4D(0.0f,-1.0f,0.0f,0.0f),
						false,
						15.0f,
						25.5f,
						10.0f,
						Vector4D(1.0f,1.2f,0.0f,1.0f)/ numTotalLightSourcesToCreate,
						Vector4D((1.0f-fraction)*10.0f,fraction*50, fraction*50 ,1.0f)/ numTotalLightSourcesToCreate
					);
			}

		}
	}


}



void convertBGRAtoRGBA(BYTE* vec4uInt8Image, int numElements)
{
	BYTE tmp;
	for(int i = 0 ; i< numElements; i++ )
	{
		//exchange component 0 and 2 (R and B)
		tmp = vec4uInt8Image[4*i];
		vec4uInt8Image[4*i]= vec4uInt8Image[4*i + 2];
		vec4uInt8Image[4*i + 2] = tmp;
	}
}

void shiftUnsignedByteToSignedByteForNormalMapping(BYTE* unsignedArray, int sizeInByte)
{
	signed char* signedPtrToUnsignedArray = reinterpret_cast<signed char*>(unsignedArray);

	for(int i = 0 ; i< sizeInByte; i++ )
	{
		//haxx: work on the same array with different type-interpreters
		//cast unsigned byte to signed int, because we need "place" for the sign, subtract 128, AND it with an 8-bit-flag(redundant, but to be sure..),
		//then cast back the int to signed byte, as now, the value is in the valid range [-128..127]
		signedPtrToUnsignedArray[i] =  static_cast<signed char>( (static_cast<int>(unsignedArray[i]) - 128) & 0xFFFF );
	}
}

void  addAlphaChannelToVec3FImage(const Vector3D* vec3Image, Vector4D* newVec4Image, int numElements)
{
	for(int i = 0 ; i< numElements; i++ )
	{
		newVec4Image[i] = Vector4D( vec3Image[i], 1.0f);
	}
}

Texture* Loader::loadTexture(String name,  BufferSemantics bufferSemantics, Path fileName,
		const BufferElementInfo& texelPreferredLayout,
		bool allocHostMemory, bool shareWithOpenCL,  bool genMipmaps
) throw(BufferException)
{
	 fipImage * image = new fipImage();
	 image->load(fileName.string().c_str());
	 LOG<<INFO_LOG_LEVEL<< "Loading image with path "
	          << fileName.string()
	          << "; Bits Per Pixel: " << image->getBitsPerPixel()
	          << "; width: " << image->getWidth()
	          << "; height" << image->getHeight()<<";\n";



	 assert(sizeof(Vector4D) == 4* sizeof(float) && "Vector types must be tightly packed");
	 //alloc buffer of maximum size; maybe only a fourth will be needed, but that does'nt matter,
	 //as it's only a temorary buffer
	 void* buffer = malloc( sizeof(Vector4D) * image->getWidth()*image->getHeight());

	 BufferElementInfo newTexeli(texelPreferredLayout);
	 //fill buffer according to a rater sophisticated conversion:
	 transformPixelData(bufferSemantics,buffer,newTexeli,image);

	  int dimensionality = (image->getHeight() >1) ? 2 : 1;

	  Texture* returnTex = 0;

	  if(dimensionality == 1)
	  {
		  returnTex = new Texture1D(
				  name,
				  bufferSemantics,
				  image->getWidth(),
				  newTexeli, //texelPreferredLayout,
				  allocHostMemory,
				  buffer,
				  	// buffer
				  	//  ?	reinterpret_cast<void*>(buffer)
					//  : reinterpret_cast<void*>(image->accessPixels()),
				  genMipmaps);

	  }
	  else
	  {
		  returnTex = new Texture2D(
				  name,
				  bufferSemantics,
				  image->getWidth(),
				  image->getHeight(),
				  newTexeli,//texelPreferredLayout,
				  allocHostMemory,
				  shareWithOpenCL,
				  //dont't make rectangle; Rectangle is good for deferred rendering, not for decal textureing
				  false,
				  buffer,
				  	// buffer
				  	//  ?	reinterpret_cast<void*>(buffer)
					//  : reinterpret_cast<void*>(image->accessPixels())
				  genMipmaps);
	  }


		//	  if(buffer)
		//	  {
		//		  delete buffer;
		//	  }
	  free(buffer);
	  delete image;

	  return returnTex;
}


Texture2DCube* Loader::loadCubeTexture(
		String name,  BufferSemantics bufferSemantics, Path fileName, String fileEndingWithoutDot,
					const BufferElementInfo& texelPreferredLayout,
					bool allocHostMemory,  bool genMipmaps
)throw(BufferException)//may be changed by the loading routine!
{
	//sequence.. maybe  becaus of left handed system and wtf... ;(
	const String suffixes[] = {"_RT", "_LF", "_DN", "_UP", "_FR", "_BK"};
	//const String suffixes[] = {"_LF", "_RT", "_UP", "_DN", "_BK", "_FR"};
	//const String suffixes[] = {"_RT", "_LF", "_UP", "_DN", "_FR", "_BK"};

	BufferElementInfo newTexeli(texelPreferredLayout);

	fipImage * image = new fipImage();


	assert( (sizeof(Vector4D) == 4* sizeof(float)) && "Vector types must be tightly packed");
	//alloc buffer of maximum size; maybe only a fourth will be needed, but that does'nt matter,
	//as it's only a temorary buffer
	void* buffer = 0;


	unsigned int dimensions;
	for(int runner=0;runner<6;runner++)
	{
		String currentFileName = fileName.string();
		currentFileName.append(suffixes[runner]);
		currentFileName.append(".");
		currentFileName.append(fileEndingWithoutDot);

		image->load(currentFileName.c_str());
		 LOG<<INFO_LOG_LEVEL<< "Loading image with path "
		          << currentFileName
		          << "; Bits Per Pixel: "
		          << image->getBitsPerPixel()
		          << "; width: " << image->getWidth()
		          << "; height" << image->getHeight()<<";\n";

		int bufferOffset;

		if(runner==0)
		{
			buffer = malloc( 6*  sizeof(Vector4D) * image->getWidth()*image->getHeight());
			bufferOffset =0;
			dimensions = image->getHeight();
		}
		else
		{
			//newTexeli is only valid until transformPixelData() has been called once;
			//luckily, its values aren't needed before
			bufferOffset =
					runner *
					(newTexeli.bitsPerChannel/ 8) / sizeof(BYTE) *
					newTexeli.numChannels *
					image->getWidth()*image->getHeight();
		}


		//fill buffer according to a rater sophisticated conversion:
		transformPixelData(bufferSemantics,
				//get adress of buffer at the current offset
				&(reinterpret_cast<BYTE*>(buffer)[bufferOffset]),
				newTexeli,
				image);

		if(runner==0)
		{dimensions = image->getHeight();}


		if(
			(image->getHeight() != image->getWidth())
			||
			! BufferHelper::isPowerOfTwo(image->getHeight())
			|| (image->getHeight() != dimensions)
		)
		{
			throw(BufferException("cubetex should be power of two,squared and all images should have the same size!"));
		}
		image->clear();
	}

	Texture2DCube* returnTex =
			new Texture2DCube(
					name, dimensions, newTexeli,allocHostMemory,
					buffer,genMipmaps
			);

	delete image;
	free(buffer);

	return returnTex;
}



void Loader::transformPixelData(BufferSemantics bufferSemantics,
		//inout buffers
		void* buffer, BufferElementInfo& texelLayout, fipImage* image)
{
	texelLayout.validate();
	assert(sizeof(Vector4D) == 4* sizeof(float) && "Vector types must be tightly packed");

	switch(image->getImageType())
	{
	case FIT_BITMAP:
		if(texelLayout.numChannels==1)
		{
			fipImage* greyImage = new fipImage();
			BOOL sucess = image->getChannel(*greyImage, FICC_RED);
			assert("got gray image" && sucess);
			assert("8 bits per pixel" && greyImage->getBitsPerPixel() == 8);
			memcpy( buffer,
					  reinterpret_cast<void*>(greyImage->accessPixels()),
					  texelLayout.numChannels * sizeof(BYTE) * greyImage->getWidth()* greyImage->getHeight()
			);
			delete greyImage;
		}
		else
		{
			if(image->getBitsPerPixel() != 32)
			{
				if ( ! (image->convertTo32Bits()) )
				{
					throw(BufferException("conversion of image to 32 bit per texel failed"));
				}
			}

			#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR
				convertBGRAtoRGBA(image->accessPixels(), image->getWidth()*image->getHeight());
			#endif

			//if(bufferSemantics == DISPLACEMENT_SEMANTICS)
			//{
			//	LOG<<WARNING_LOG_LEVEL<<"Trying the shiftUnsignedByteToSignedByteForNormalMapping; "
			//		<< "This is experimental; in case of bugs, check  Loader::loadTexture()";
			//	//normalmapping adoption :)
			//	shiftUnsignedByteToSignedByteForNormalMapping(image->accessPixels(), image->getImageSize());
			//	texelLayout = BufferElementInfo(4,GPU_DATA_TYPE_INT,8,true);
			//}
			//else{
			//override to default as i don't have time for sophisticated adoption atm
			texelLayout = BufferElementInfo(4,GPU_DATA_TYPE_UINT,8,true);
			//}

			  //copy altered image contents to the designated buffer
			  memcpy( buffer,
					  reinterpret_cast<void*>(image->accessPixels()),
					  texelLayout.numChannels * sizeof(BYTE) * image->getWidth()* image->getHeight()
			  );
		}
		break;

	case FIT_FLOAT:
		assert("we want load a gray level image" && texelLayout.numChannels==1);

		//TODO
		assert(0 && "TODO implement gray float image loading for high res displ mapping!");
		break;

	case FIT_RGBAF:
		if(texelLayout.numChannels==1)
		{
			assert("float image really wanted" && (texelLayout.internalGPU_DataType==GPU_DATA_TYPE_FLOAT)
					&& (texelLayout.bitsPerChannel ==32));
			fipImage* greyImage = new fipImage();
			BOOL sucess = image->getChannel(*greyImage, FICC_RED);
			assert("got gray image" && sucess);
			assert("32 bits per pixel" && greyImage->getBitsPerPixel() == 32);
			 memcpy( buffer,
					  reinterpret_cast<void*>(greyImage->accessPixels()),
					  texelLayout.numChannels * sizeof(float) * greyImage->getWidth()* greyImage->getHeight()
			 );
			 delete greyImage;
		}
		else
		{
		  texelLayout = BufferElementInfo(4,GPU_DATA_TYPE_FLOAT,32,false);

		  //copy altered image contents to the designated buffer
		  memcpy( buffer,
				  reinterpret_cast<void*>(image->accessPixels()),
				  texelLayout.numChannels * sizeof(float) * image->getWidth()* image->getHeight()
		  );
		}

		  break;

	case FIT_RGBF:
		  //add alpha channel for alignment purposes; freeimange doesn't support this conversion,
		  //so let's hack it for ourselves:
		  //buffer	= new Vector4D[image->getWidth()*image->getHeight()];

		  assert((image->getBitsPerPixel()==96) && "it is really 32bit three component image" );

		  addAlphaChannelToVec3FImage(
				reinterpret_cast<Vector3D*>(image->accessPixels()),
		  		reinterpret_cast<Vector4D*> (buffer),
		  		image->getWidth()*image->getHeight());

		  texelLayout = BufferElementInfo(4,GPU_DATA_TYPE_FLOAT,32,false);

		  //copy altered image contents to the designated buffer
		  memcpy( buffer,
				  reinterpret_cast<void*>(image->accessPixels()),
				  texelLayout.numChannels * sizeof(float) * image->getWidth()* image->getHeight()
			);
		  break;

	default:
		  throw(BufferException("sorry, there is no other image type but "
				  "floating point or Bitmap(i.e. 8 bit unsigned normalized int) RGB(A) supported yet "));

	}
}


//--------------------------------------------------------------------------------



void Loader::loadGlobalConfig(Config& config)
{
	Path dummy(FLEWNIT_DEFAULT_CONFIG_PATH);
	loadGlobalConfig(config, dummy);
}

void Loader::loadGlobalConfig(Config& config, const Path & pathToGlobalConfigFile)
{
	LOG<<INFO_LOG_LEVEL<< "Loading global Config;\n";

	TiXmlDocument XMLDoc ("flewnitGlobalConfig");

	try {
		if(! XMLDoc.LoadFile(pathToGlobalConfigFile.string())
			|| XMLDoc.Error())
		{
			throw std::exception();
		}

		LOG<<INFO_LOG_LEVEL<<"Config file has root node named "<<
				XMLDoc.RootElement()->ValueStr()<<" ;\n";


		//config.rootPtr() = new ConfigStructNode();
		//config.root() = parseElement();

		//check for children:
		TiXmlNode* child = 0;
		TiXmlNode* rootXmlNode =  XMLDoc.RootElement();
		while( ( child = rootXmlNode->IterateChildren( child ) ) != NULL)
		{
			//parse only "elements":
			TiXmlElement* childElement = child->ToElement();
			if(childElement)
			{
				//returnNode->get(child->ValueStr())= parseElement(childElement);
				config.root()[child->ValueStr()].push_back( parseElement(childElement) );
			}
		}


	} catch (...) {
		LOG<<ERROR_LOG_LEVEL<< "[Loader::loadGlobalConfig] Error loading file \""<< pathToGlobalConfigFile.string() <<"\";"
				<< XMLDoc.ErrorDesc()
				<<"; No default config initialization yet exists;\n";
		assert(0);
	}
}




ConfigStructNode* Loader::parseElement(TiXmlElement* xmlElementNode)
{
	ConfigStructNode* returnNode = 0;

	const String* typeOfNode = xmlElementNode->Attribute(String("type"));
	const String nodeName = xmlElementNode->ValueStr();


	if(typeOfNode)
	{
		//ok, this is a "value" node which could be interesting for GUI manipulation:
		GUIParams guiParams;
		getGUIParams(xmlElementNode,guiParams);

		if( *typeOfNode == String("STRING") )
		{
			returnNode = new ConfigValueNode<String>(
					nodeName,
					//dereference String pointer
					*( xmlElementNode->Attribute(String("value")) )
					, guiParams);
		}

		if( *typeOfNode == String("BOOL") )
		{
			returnNode = new ConfigValueNode<bool>(
					nodeName,
					 *(xmlElementNode->Attribute(String("value"))) == String("true")
					 ? true : false
					,guiParams);
		}

		if( *typeOfNode == String("INT") )
		{
			returnNode = new ConfigValueNode<int>(
					nodeName,
					 boost::lexical_cast<int>( *(xmlElementNode->Attribute( String("value") )) )
					,guiParams);
		}

		if( *typeOfNode == String("FLOAT") )
		{
			returnNode = new ConfigValueNode<float>(
					nodeName,
					boost::lexical_cast<float>( *(xmlElementNode->Attribute( String("value") )) )
					,guiParams);
		}




		if( *typeOfNode == String("VEC2I") )
		{
			returnNode = new ConfigValueNode<Vector2Di>(
					nodeName,
					Vector2Di(
							boost::lexical_cast<int>( *(xmlElementNode->Attribute( String("x") )) ),
							boost::lexical_cast<int>( *(xmlElementNode->Attribute( String("y") )) )
					)
					,guiParams);
		}

		if( *typeOfNode == String("VEC3I") )
		{
			returnNode = new ConfigValueNode<Vector3Di>(
					nodeName,
					Vector3Di(
							boost::lexical_cast<int>( *(xmlElementNode->Attribute( String("x") )) ),
							boost::lexical_cast<int>( *(xmlElementNode->Attribute( String("y") )) ),
							boost::lexical_cast<int>( *(xmlElementNode->Attribute( String("z") )) )
					)
					,guiParams);
		}


		if( *typeOfNode == String("VEC4I") )
		{
			returnNode = new ConfigValueNode<Vector4Di>(
					nodeName,
					Vector4Di(
							boost::lexical_cast<int>( *(xmlElementNode->Attribute( String("x") )) ),
							boost::lexical_cast<int>( *(xmlElementNode->Attribute( String("y") )) ),
							boost::lexical_cast<int>( *(xmlElementNode->Attribute( String("z") )) ),
							boost::lexical_cast<int>( *(xmlElementNode->Attribute( String("w") )) )
					)
					,guiParams);
		}




		if( *typeOfNode == String("VEC2") )
		{
			returnNode = new ConfigValueNode<Vector2D>(
					nodeName,
					Vector2D(
							boost::lexical_cast<float>( *(xmlElementNode->Attribute( String("x") )) ),
							boost::lexical_cast<float>( *(xmlElementNode->Attribute( String("y") )) )
					)
					,guiParams);
		}

		if( *typeOfNode == String("VEC3") )
		{
			returnNode = new ConfigValueNode<Vector3D>(
					nodeName,
					Vector3D(
							boost::lexical_cast<float>( *(xmlElementNode->Attribute( String("x") )) ),
							boost::lexical_cast<float>( *(xmlElementNode->Attribute( String("y") )) ),
							boost::lexical_cast<float>( *(xmlElementNode->Attribute( String("z") )) )
					)
					,guiParams);
		}


		if( *typeOfNode == String("VEC4") )
		{
			returnNode = new ConfigValueNode<Vector4D>(
					nodeName,
					Vector4D(
							boost::lexical_cast<float>( *(xmlElementNode->Attribute( String("x") )) ),
							boost::lexical_cast<float>( *(xmlElementNode->Attribute( String("y") )) ),
							boost::lexical_cast<float>( *(xmlElementNode->Attribute( String("z") )) ),
							boost::lexical_cast<float>( *(xmlElementNode->Attribute( String("w") )) )
					)
					,guiParams);
		}

		//TODO rest
		if(! returnNode)
		{
			LOG<<ERROR_LOG_LEVEL<<"Loader::parseElement: unknown \"type\" string in XML config element: "<<
					(*typeOfNode) <<";\n";

		}
	}
	else
	{
		//it's no "value" node but a container node:
		returnNode = new ConfigStructNode(nodeName);
	}

	//check for children:
	TiXmlNode* child = 0;
	while( ( child = xmlElementNode->IterateChildren( child ) ) != NULL )
	{
		//parse only "elements":
		TiXmlElement* childElement = child->ToElement();
		if(childElement)
		{
			(*returnNode) [ child->ValueStr() ] . push_back( parseElement(childElement) );
		}
	}


	return returnNode;
}

void Loader::getGUIParams(TiXmlElement* xmlElementNode, GUIParams& guiParams)
{
	const String* guiVisibilityOfNode = xmlElementNode->Attribute(String("GUIVisibility"));

	if(guiVisibilityOfNode)
	{
		if(*(guiVisibilityOfNode) == String("read"))
		{
			guiParams.setGUIVisibility( ACCESS_READ );
		}
		else
		{
			if(*(guiVisibilityOfNode) == String("read/write"))
			{
				guiParams.setGUIVisibility( ACCESS_READWRITE );
			}
			else
			{
				if(*(guiVisibilityOfNode) == String("none"))
				{
					guiParams.setGUIVisibility( ACCESS_NONE );
				}
				else
				{
					LOG<<ERROR_LOG_LEVEL<<"GUIVisibility String \""<< *guiVisibilityOfNode << "\" invalid; \n";
				}
			}
		}

		//get the tweakConfigString
		const String* tweakConfigString = xmlElementNode->Attribute(String("tweakConfigString"));
		if(tweakConfigString)
		{
			guiParams.setGUIPropertyString(*tweakConfigString);
		}
	}
	//else do nothing, as the defaul constructor of GUIParams allready has initialized its values;


}



}



