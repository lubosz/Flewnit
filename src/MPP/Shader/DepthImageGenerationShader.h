/*
 * DepthImageGenerationShader.h
 *
 *  Created on: Feb 6, 2011
 *      Author: tychi
 */

#pragma once


#include "Shader.h"

namespace Flewnit {



class DepthImageGenerationShader
: public Shader
{
	FLEWNIT_BASIC_OBJECT_DECLARATIONS;

public:
	DepthImageGenerationShader(Path codeDirectory,
			RenderingTechnique renderingTechnique,
			TextureType renderTargetTextureType,
			bool instancedRendering);

	virtual ~DepthImageGenerationShader();

	virtual void use(SubObject* so)throw(SimulatorException);

protected:
	virtual void generateCustomDefines();
	virtual void bindFragDataLocations(RenderTarget* rt) throw(BufferException);
};
}

