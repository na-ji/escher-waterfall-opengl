#pragma once

#include "myMaterial.h"
#include "myTexture.h"

class mySubObject3D
{
public:
	myMaterial *material = nullptr;
	int start_index, end_index;
};
