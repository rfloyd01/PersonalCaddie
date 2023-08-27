#pragma once

#include "Graphics/Utilities/DeviceResources.h"
#include "Modes/ModeScreen.h"

#include <string>

//This class is responsible for the rendering of all 2D objects. This includes
//text, as well as menu objects like buttons, drop downs, combo boxes, etc.

class Direct2DRenderer
{
public:


private:
	std::shared_ptr<DX::DeviceResources>                                     m_deviceResources;
};