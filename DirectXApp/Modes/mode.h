#pragma once

#include "Graphics/Objects/2D/UIElements.h"
#include "Graphics/Utilities/UIElementManager.h"
#include "Graphics/Objects/3D/Elements/VolumeElement.h"
#include "Graphics/Rendering/Material.h"
#include "Input/InputProcessor.h"
#include "Math/glm.h"
#include "Devices/PersonalCaddie.h" //needed for PersonalCaddiePowerMode enum

#include <string>
#include <functional>


//Classes, structs and enums that are helpful for this class
enum class ModeType
{
	//This enum represents all of the different types of modes there are for the program
	//when adding a new mode it should also be added to this list

	MAIN_MENU = 0,
	FREE = 1,
	CALIBRATION = 2,
	TRAINING = 3,
	SETTINGS_MENU = 4,
	DEVICE_DISCOVERY = 5,
	DEVELOPER_TOOLS = 6,
	UI_TEST_MODE = 7,
	GRAPH_MODE = 8,
	IMU_SETTINGS = 9,
	MADGWICK = 10,
	END = 11 //allows for looping through of ModeType enum class. This number should be one more than previous value
};

//The ModeState enum keeps track of what the current state of the app is. Each of these
//enums act as binary flags that go into a larger number
enum ModeState
{
	Active = 1,
	Recording = 2,
	CanTransfer = 4,
	NeedTextUpdate = 8,
	PersonalCaddieSensorIdleMode = 16,
	PersonalCaddieSensorActiveMode = 32,
	NeedMaterial = 64,
	Enter_Active = 128,
	State_Update = 256,
	Leave_Active = 512
};

//Sometimes there are certain actions that we need to take inside the given mode, however, the mode doesn't have
//the power to actually take the action. For example, in calibration mode we need to turn on the Personal Caddie
//so that it starts taking data measurements, but this can only be done from the Mode Screen class. This enum
//holds different actions that the active mode can ask the mode screen class to take on its behalf.
enum ModeAction
{
	PersonalCaddieChangeMode,
	RendererGetMaterial,
	MadgwickUpdateFilter,
	SensorSettings,
	SensorCalibration,
	BLEDeviceWatcher,
	BLEConnection,
	BLENotifications
};

//Forward Declerations
//enum PersonalCaddiePowerMode;

//Class definition
class Mode
{
public:
	//PUBLIC FUNCTIONS
	virtual uint32_t initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState = 0) = 0;
	virtual void uninitializeMode() = 0;

	virtual void update() {}; //not a pure virtual method as not all modes require this method

	virtual void handlePersonalCaddieConnectionEvent(bool connectionStatus) {}; //Some modes need the ability to enable or disable features if the Personal Caddie gets disconnected

	const UIColor getBackgroundColor();

	std::vector<std::shared_ptr<UIElement> > const& getUIElements() { return m_uiManager.getRenderElements(); }
	UIElementManager & getUIElementManager() { return m_uiManager; }
	std::vector<std::shared_ptr<VolumeElement> > const& getVolumeElements() { return m_volumeElements; }

	uint32_t getModeState() { return m_state; }

	std::vector<MaterialType> const& getMaterialTypes() { return m_materialTypes; }; //modes with 3d rendering need materials generated from the Direct3D context

	template <typename T>
	void addUIElement(T const& element) { m_uiElements.push_back(std::make_shared<T>(element)); }

	//Alert Methods
	void createAlert(std::wstring message, UIColor color, winrt::Windows::Foundation::Size windowSize, long long duration = 2500); //default to 2.5 second alerts
	//void createAlert(TextOverlay& alert);
	//virtual TextOverlay removeAlerts(); //some modes need to maintain a specific order for elements and removing alerts can change this. This method is virtual so those modes can override this one.
	std::vector<std::shared_ptr<ManagedUIElement>> removeAlerts();
	void overwriteAlerts(std::vector<std::shared_ptr<ManagedUIElement>> const& alerts);
	void checkAlerts();

	virtual void getSensorCalibrationNumbers(sensor_type_t sensor, std::pair<const float*, const float**> cal_numbers) {};
	virtual void getSensorAxisCalibrationNumbers(sensor_type_t sensor, std::pair<const int*, const int*> cal_numbers) {};

	virtual uint32_t handleUIElementStateChange(int i) = 0;

	virtual void addData(std::vector<std::vector<std::vector<float> > > const& sensorData, float sensorODR, float timeStamp, int totalSamples) {} //A method that modes can overwrite when they need data from the Personal Caddie
	virtual void addQuaternions(std::vector<glm::quat> const& quaternions, int quaternion_number, float time_stamp, float delta_t) {} //A method that modes can overwrite when they need data from the Personal Caddie

	bool m_needsCamera = false; //Modes that require 3D rendering will set this variable to true to let the ModeScreen know that its camera is needed

	//Methods for handling Personal Caddie, BLE and Sensor Events
	virtual void pc_ModeChange(PersonalCaddiePowerMode newMode) {};
	virtual void ble_NotificationsChange(int state) {};
	static void setHandlerMethod(std::function<void(ModeAction, void*)> func) { m_mode_screen_handler = func; } //static method for setting the function pointer in the mode state class

protected:
	UIColor m_backgroundColor; //represents the background color when this mode is being rendered

	//std::vector<std::shared_ptr<UIElement> >  m_uiElements; //2d objects like Drop downs, combo boxes, buttons and text
	std::vector<std::shared_ptr<VolumeElement> >  m_volumeElements; //3d objects to be rendered on screen (not all modes feature 3D objects)
	std::vector<MaterialType> m_materialTypes; //A vector of material types to be applied to 3d rendered objects
	UIElementManager m_uiManager; //a class that helps us manage the UI Elements in the mode

	uint32_t m_state; //the state of the current mode

	//static function pointer to Mode Screen class goes here. The handler method is the same for all
	//modes which is why the function pointer is static
	static std::function<void(ModeAction, void*)> m_mode_screen_handler; //pointer to an event handler in the Personal Caddie class
};