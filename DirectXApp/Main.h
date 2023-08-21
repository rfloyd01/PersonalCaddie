#pragma once

#include "Graphics/Rendering/MasterRenderer.h"

class Main : public winrt::implements<Main, winrt::Windows::Foundation::IInspectable>, DX::IDeviceNotify
{

};