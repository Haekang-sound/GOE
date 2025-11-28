#pragma once
#include "../GOE_Core/Core_pch.h"
#include "../GOE_AssetLoader/AssetCore.h"
#include "../GOE_Editor/EditorCore.h"


namespace GOE
{
	struct EngineContext
	{
		ID3DRenderer* renderer = nullptr;
		AssetCore* assetCore = nullptr;
	};
}
