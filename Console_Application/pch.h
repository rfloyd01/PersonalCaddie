#pragma once
#include "winrt/Windows.Foundation.h"
#include "winrt/Windows.Foundation.Collections.h"
#include "winrt/Windows.Devices.Bluetooth.h"
#include "winrt/Windows.Devices.Bluetooth.Advertisement.h"
#include "winrt/Windows.Storage.Streams.h"
#include "winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h"
#include "winrt/Windows.Security.Cryptography.h"
#include <experimental/resumable>
#include <stdio.h>
#include <tchar.h>
#include <Math/eigen.h> //This header needs to be included before "mode.h" in main code, not sure why though