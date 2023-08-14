#include "pch.h"

#include "Devices/PersonalCaddie.h"
#include "Graphics/graphics.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Devices;
using namespace Bluetooth::GenericAttributeProfile;

int main()
{
    init_apartment();

    //Create an instance of the Personal Caddie class
    PersonalCaddie m_pc;

    //Set up OpenGL and Shaders and link it to the Personal Caddie via a pointer
    GL GraphicWindow(&m_pc);

    //Main rendering loop
    while (!GraphicWindow.ShouldClose())
    {
        //All updates for sensor data are taking place behing the scenes, values are updated when new data comes in from a concurrent function
        //That's why (for now) the only thing in this loop has to do with the graphic window

        GraphicWindow.masterUpdate();
        GraphicWindow.masterRender();
    }

    GraphicWindow.Terminate();

    return 0;
}