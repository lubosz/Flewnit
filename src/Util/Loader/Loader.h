
/*
 * Loader.h
 *
 *  Created on: Dec 6, 2010
 *      Author: tychi
 */

#pragma once


#include "Common/BasicObject.h"

#include "Buffer/BufferSharedDefinitions.h"

class TiXmlElement;
struct fipImage;


namespace Flewnit
{

class Config;
class ConfigStructNode;
class GUIParams;
class SceneGraph;

class Loader : public BasicObject
{
	FLEWNIT_BASIC_OBJECT_DECLARATIONS

public:
	Loader();
	virtual ~Loader();

	void loadGlobalConfig( Config& config, const Path& pathToGlobalConfigFile	);
	void loadGlobalConfig( Config& config);

	void loadScene();

	Texture* loadTexture(String name,  BufferSemantics bufferSemantics, Path fileName,
			//default layout for non-HDR color images unsigned byte to be normalize to float in [0..1]
			//loading routine doesn't guaranty that the preferred layout will be done!
			const BufferElementInfo&  texelPreferredLayout = BufferElementInfo(4,GPU_DATA_TYPE_UINT,8,true),
			//for normal maps, one could try the following, i.e. normalized SIGNED int; this way, a bias from [0..1] to [-1..+1] within a pixel shader would become obsolete ;)
			//BufferElementInfo(4,GPU_DATA_TYPE_INT,8,true),
			bool allocHostMemory=true, bool shareWithOpenCL= false, bool genMipmaps = false
	) throw(BufferException);

	Texture2DCube* loadCubeTexture(
			Path cubeMapFilePath,  BufferSemantics bufferSemantics,
			//loading routine doesn't guaranty that the preferred layout will be done!
			//default layout for non-HDR color images unsigned byte to be normalize to float in [0..1]
			const BufferElementInfo& texelPreferredLayout = BufferElementInfo(4,GPU_DATA_TYPE_UINT,8,true),
			bool allocHostMemory=true,  bool genMipmaps = false
	)throw(BufferException);

private:

	void loadUISettings(TiXmlElement* xmlElementNode, Config& config);

	ConfigStructNode* parseElement(TiXmlElement* xmlElementNode);

	void getGUIParams(TiXmlElement* xmlElementNode, GUIParams& guiParams);

	//DEBUG stuff:
	void createHardCodedSceneStuff();


	void transformPixelData(BufferSemantics bufferSemantics,
			//inout buffers
			void* buffer, BufferElementInfo& texelLayout, fipImage* image);


};


}

