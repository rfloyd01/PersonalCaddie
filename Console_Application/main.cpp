#include "pch.h"

#include <iostream>

//#include <Devices/BluetoothLE.h>
#include "Devices/PersonalCaddie.h"
#include <Graphics/graphics.h>
#include <Math/ellipse.h>
#include <Modes/modes.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Devices;
using namespace Bluetooth::GenericAttributeProfile;

auto serviceUUID = Bluetooth::BluetoothUuidHelper::FromShortId(0x2B30BF34); //This is the Sensor Service of the Personal Caddie
uint32_t characteristicUUID = 0x2b30bf35; //This is the Sensor Settings characteristic on the Personal Caddie

int main()
{
    init_apartment();

    PersonalCaddie m_pc;

    //Set up OpenGL and Shaders
    GL GraphicWindow(&BLE_Nano);

    //Add all proper modes to the Graphic Interface
    MainMenu mm(GraphicWindow); GraphicWindow.addMode(&mm);
    FreeSwing fs(GraphicWindow); GraphicWindow.addMode(&fs);
    Calibration cc(GraphicWindow); GraphicWindow.addMode(&cc);
    Training tt(GraphicWindow); GraphicWindow.addMode(&tt);
    Settings ss(GraphicWindow); GraphicWindow.addMode(&ss);

    GraphicWindow.setCurrentMode(ModeType::MAIN_MENU); //start off with the main menu, ultimately want to move this Mode setup into the graphic intialization

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