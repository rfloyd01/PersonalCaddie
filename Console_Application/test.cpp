#include "pch.h"

#include <iostream>
#include <test.h>

#include <Devices/PersonalCaddie.h>
#include <Graphics/graphics.h>
#include <Graphics/model.h>
#include <Modes/mode.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Devices;
using namespace Bluetooth::GenericAttributeProfile;

//auto serviceUUID = Bluetooth::BluetoothUuidHelper::FromShortId(0x180C);
//auto characteristicUUID = Bluetooth::BluetoothUuidHelper::FromShortId(0x2A56);

//This file is for testing out a new main() function

int test_main()
{
    //Create BLE device and wait for it to connect to actual BLE device
    //float sensor_refresh_rate = 400; //chip is set to gather all forms of data at 400Hz
    //BLEDevice BLE_Nano(Bluetooth::BluetoothUuidHelper::FromShortId(0x180C), 0x2A56, sensor_refresh_rate);
    //IMU Arduino_Nano_BLE(Accelerometer::BLE_SENSE_33, Gyroscope::BLE_SENSE_33, Magnetometer::BLE_SENSE_33);
    //BLEDevice BLE_Nano(Bluetooth::BluetoothUuidHelper::FromShortId(0x180C), 0x2A56, &Arduino_Nano_BLE);
    //BLE_Nano.connect();
    //while (BLE_Nano.data_available == false) //wait until chip has connected and started to gather data before moving onto the next step
    //{
    //}
    //BLE_Nano.setMagField();

    //Set up OpenGL and Shaders
    //GL GraphicWindow(&BLE_Nano);
    //GraphicWindow.LoadTexture("FXOS8700.jpg");
    //GraphicWindow.AddText(MessageType::FOOT_NOTE, { "Press Space to enter calibration mode.", 520.0f, 10.0f, 0.33f, glm::vec3(1.0f, 1.0f, 1.0f), GraphicWindow.getScreenWidth() });

    //Create vector of Modes within the Graphic Interface

    //while (!GraphicWindow.ShouldClose())
    //{
    //    //GraphicWindow.Swap();
    //    //if (glfwGetTime() - GraphicWindow.GetKeyTimer() >= GraphicWindow.GetKeyTime()) GraphicWindow.setCanPressKey(true);
    //    //std::cout << GraphicWindow.getCurrentMode()->mode_name << std::endl;
    //}

	return 0;
}