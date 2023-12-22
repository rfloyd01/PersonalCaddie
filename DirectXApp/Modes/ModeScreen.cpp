#include "pch.h"

#include "ModeScreen.h"
#include "MainMenuMode.h"
#include "SettingsMenuMode.h"
#include "DevelopmentMenuMode.h"
#include "DeviceDiscoveryMode.h"
#include "UITestMode.h"
#include "GraphMode.h"
#include "IMUSettingsMode.h"
#include "MadgwickTestMode.h"
#include "CalibrationMode.h"
#include "FreeSwingMode.h"

#include "Graphics/Rendering/MasterRenderer.h"

#include <functional>

ModeScreen::ModeScreen() :
	m_currentMode(ModeType::MAIN_MENU),
	alert_active(false),
	button_pressed(false),
	personal_caddy_event(PersonalCaddieEventType::NONE)
{
	//Create instances of all the different modes.
	for (int i = 0; i < static_cast<int>(ModeType::END); i++) m_modes.push_back(nullptr);

	m_modes[static_cast<int>(ModeType::MAIN_MENU)] = std::make_shared<MainMenuMode>();
	m_modes[static_cast<int>(ModeType::SETTINGS_MENU)] = std::make_shared<SettingsMenuMode>();
	m_modes[static_cast<int>(ModeType::FREE)] = std::make_shared<FreeSwingMode>();
	m_modes[static_cast<int>(ModeType::DEVICE_DISCOVERY)] = std::make_shared<DeviceDiscoveryMode>();
	m_modes[static_cast<int>(ModeType::DEVELOPER_TOOLS)] = std::make_shared<DevelopmentMenuMode>();
	m_modes[static_cast<int>(ModeType::UI_TEST_MODE)] = std::make_shared<UITestMode>();
	m_modes[static_cast<int>(ModeType::GRAPH_MODE)] = std::make_shared<GraphMode>();
	m_modes[static_cast<int>(ModeType::IMU_SETTINGS)] = std::make_shared<IMUSettingsMode>();
	m_modes[static_cast<int>(ModeType::MADGWICK)] = std::make_shared<MadgwickTestMode>();
	m_modes[static_cast<int>(ModeType::CALIBRATION)] = std::make_shared<CalibrationMode>();

	//After creating the modes, bind the mode handler method to the mode class so that all
	//different mode types can use it
	Mode::setHandlerMethod(std::bind(&ModeScreen::ModeHandler, this, std::placeholders::_1, std::placeholders::_2));

	//Set default times for various timers (in milliseconds)
	alert_timer_duration = 2500;
	button_pressed_duration = 100;
}

winrt::fire_and_forget ModeScreen::Initialize(
	_In_ std::shared_ptr<InputProcessor> const& input,
	_In_ std::shared_ptr<MasterRenderer> const& renderer
)
{
	m_inputProcessor = input;
	m_renderer = renderer;

	//Load the main mode (which is set in the constructor) and pass any resources
	//generated to the master renderer
	m_modeState = m_modes[static_cast<int>(m_currentMode)]->initializeMode(m_renderer->getCurrentScreenSize());
	co_await m_renderer->CreateModeResourcesAsync();
	m_renderer->FinalizeCreateDeviceResources();
}

void ModeScreen::setPersonalCaddie(_In_ std::shared_ptr<PersonalCaddie> const& pc)
{
	m_personalCaddie = pc;
}

void ModeScreen::update()
{
	//check for any input form the mouse/keyboard that needs processing by the current mode
	auto inputUpdate = m_inputProcessor->update();
	
	//Handle Keyboard Input
	processKeyboardInput(inputUpdate);

	//Handle Mouse Input
	processMouseInput(inputUpdate);

	//after processing input, see if there are any event handlers that were triggered
	processEvents();

	//Have the current state make any necessary updates based on the 
	//input and events that have just been processed
	getCurrentMode()->uiUpdate(); //Updates based on interaction with UI Elements on screen
	getCurrentMode()->update();  //Updates based on internal state of the mode
}

void ModeScreen::processKeyboardInput(InputState* inputState)
{
	//Any key presses that are common between modes (such as the escape key being used to exit the  
	//current mode) are processed right here in the mode state class. If the key press isn't processed
	//here then we defer to the current mode to do it.

	if (inputState->currentPressedKey == KeyboardKeys::DeadKey) return; //no button was pressed so there's nothing to process
	getCurrentMode()->handleKeyPress(inputState->currentPressedKey);
	m_inputProcessor->setKeyboardState(KeyboardState::KeyProcessed); //let the input processor know to deactivate this key until it's released
}

void ModeScreen::processMouseInput(InputState* inputState)
{
	//If the mouse hasn't moved, been clicked, or the scroll wheel
	//hasn't rotated then there's nothing to process here.
	if (!inputState->mouseClick && inputState->scrollWheelDirection == 0 && ((inputState->mousePosition.x == m_previousMousePosition.x) && (inputState->mousePosition.y == m_previousMousePosition.y)))
	{
		return;
	}

	//If the mouse HAS moved, clicked or scrolled then the UIElementManager of the currently
	//active mode will check all of the UIElements that are in the same section of the screen 
	//(the screen is partitioned into a grid) as the mouse currently is to see if this mouse 
	//input changes any of their states.
	m_modes[static_cast<int>(m_currentMode)]->getUIElementManager().updateGridSquareElements(inputState);
	m_previousMousePosition = inputState->mousePosition; //update the mouse position variable

	//reset input states if necessary
	if (inputState->mouseClick) m_inputProcessor->setMouseState(MouseState::ButtonProcessed); //let the input processor know that the click has been handled
	if (inputState->scrollWheelDirection != 0) inputState->scrollWheelDirection = 0; //let the input processor know that mouse scroll has been handled
}

void ModeScreen::processEvents()
{
	//There are times when handler methods asynchronously receive information that we'd like to display
	//in screen. With DirectX, we're only allowed to access the main rendering device from the main 
	//thread, so attempting to render from an asynchronous handler method can cause some very confusing
	//crashes. What we do instead is have these handler methods set flags in the ModeScreen class, which 
	//then get processed in this method. This method is only called from the main rendering loop so this 
	//is a safe way to handle asynchronous rendering of things such as alerts.
	m_modes[static_cast<int>(m_currentMode)]->checkAlerts();
}

void ModeScreen::changeCurrentMode(ModeType mt)
{
	//This method unitializes the current mode and initializes the mode selected.
	//Any active alerts will get copied into the text of the new mode
	if (mt == m_currentMode) return; //no need to change anything, already on this mode

	//First, get any active alerts before deleting the text and color map of the 
	//current mode
	auto currentAlerts = m_modes[static_cast<int>(m_currentMode)]->removeAlerts();

	//If we're in sensor active mode, we need to first enter sensor idle mode, and then connected mode
	m_modeState = 0;
	m_modes[static_cast<int>(m_currentMode)]->uninitializeMode();

	//Switch to the new mode and initialize it
	m_currentMode = mt;
	uint32_t startingModeState = 0;

	//Initialize the new mode
	m_modeState = m_modes[static_cast<int>(m_currentMode)]->initializeMode(m_renderer->getCurrentScreenSize(), startingModeState);
	
	//After initializing, add any alerts that were copied over to the text and
	//color maps and then create text and color resources in the renderer
	m_modes[static_cast<int>(m_currentMode)]->overwriteAlerts(currentAlerts);
}

void ModeScreen::getTextRenderPixels(std::vector<UIText*> const& text)
{
	//Sets the size for the text overlay render box of the given text element
	for (int i = 0; i < text.size(); i++) m_renderer->setTextLayoutPixels(text[i]);
}

std::map<UIElementType, std::vector<std::shared_ptr<ManagedUIElement> > > const& ModeScreen::getCurrentModeUIElementMap()
{
	return getCurrentMode()->getUIElementManager().getElementsMap();
}

void ModeScreen::resizeCurrentModeUIElements(winrt::Windows::Foundation::Size windowSize)
{
	///Update the m_screenSize variable of the current modes UIElementManager
	m_modes[static_cast<int>(m_currentMode)]->getUIElementManager().updateScreenSize(windowSize);
}

std::vector<std::shared_ptr<VolumeElement> > const& ModeScreen::getCurrentModeVolumeElements()
{
	//returns a reference to all 3D elements to be rendered on screen
	return m_modes[static_cast<int>(m_currentMode)]->getVolumeElements();
}

const UIColor ModeScreen::getBackgroundColor()
{
	//returns the background color of the current mode
	return m_modes[static_cast<int>(m_currentMode)]->getBackgroundColor();
}

void ModeScreen::createAlert(std::wstring message, UIColor color)
{
	m_modes[static_cast<int>(m_currentMode)]->createAlert(message, color, m_renderer->getCurrentScreenSize());
}

void ModeScreen::PersonalCaddieHandler(PersonalCaddieEventType pcEvent, void* eventArgs)
{
	//This method will get automatically called when certain things happen with the 
	//Personal Caddie device. For example, if we lose the connection to the Personal
	//Caddie a BLE_ALERT will come here so we can render it on the screen. Another 
	//example would be when we're in Device Discovery mode, we can send a list of all
	//the found Bluetooth devices here so the Discovery mode can process them.

	personal_caddy_event = pcEvent;
	switch (pcEvent)
	{
	case PersonalCaddieEventType::CONNECTION_EVENT:
	{
		//This is a special type of BLE event. We've either connected to a new device
		//or disconnected from the current one. This may potentially enable or disable
		//some features depending on the current mode.
		
		m_modes[static_cast<int>(m_currentMode)]->handlePersonalCaddieConnectionEvent(m_personalCaddie->ble_device_connected);
		std::wstring alertText = *((std::wstring*)eventArgs); //cast the eventArgs into a wide string
		createAlert(alertText, UIColor::Blue);

		break;
	}
	case PersonalCaddieEventType::BLE_ALERT:
	{
		std::wstring alertText = *((std::wstring*)eventArgs); //cast the eventArgs into a wide string
		createAlert(alertText, UIColor::Blue);
		
		break;
	}
	case PersonalCaddieEventType::PC_ALERT:
	{
		//TODO: Instead of sending a message of text, send an instance of the PeronsalCaddiePowerMode
		//enum which contains the new mode. Craft an appropriate message depending on the mode obtained.
		std::wstring alertText = *((std::wstring*)eventArgs); //cast the eventArgs into a wide string
		createAlert(alertText, UIColor::Yellow);

		if (alertText == L"The Personal Caddie has been placed into Sensor Idle Mode")
		{
			m_modes[static_cast<int>(m_currentMode)]->pc_ModeChange(PersonalCaddiePowerMode::SENSOR_IDLE_MODE);
		}
		else if (alertText == L"The Personal Caddie has been placed into Sensor Active Mode")
		{
			m_modes[static_cast<int>(m_currentMode)]->pc_ModeChange(PersonalCaddiePowerMode::SENSOR_ACTIVE_MODE);
		}
		else if (alertText == L"The Personal Caddie has been placed into Connected Mode")
		{
			m_modes[static_cast<int>(m_currentMode)]->pc_ModeChange(PersonalCaddiePowerMode::CONNECTED_MODE);

			//When entering connected mode, we make sure that data notifications are turned off
			m_personalCaddie->disableDataNotifications();
		}
		break;
	}
	case PersonalCaddieEventType::IMU_ALERT:
	{
		std::wstring alertText = *((std::wstring*)eventArgs); //cast the eventArgs into a wide string
		createAlert(alertText, UIColor::Green);

		if (alertText == L"Updated Calibration Info" || alertText == L"Updated Axis Orientation Info")
		{
			if (m_currentMode == ModeType::CALIBRATION)
			{
				m_modeState ^= ModeState::Active; //completing the update causes us to leave active mode
			}
		}
		break;
	}
	case PersonalCaddieEventType::DEVICE_WATCHER_UPDATE:
	{
		std::set<DeviceInfoDisplay>* foundDevices = (std::set<DeviceInfoDisplay>*)eventArgs;
		std::wstring devices;
		for (auto it = foundDevices->begin(); it != foundDevices->end(); it++)
		{
			devices += L"Name: " + it->device_name;
			devices += L", Address: " + std::to_wstring(it->device_address.first) + L"\n";
		}

		getCurrentMode()->getString(devices); //pass the list of devices to the current mode
		break;
	}
	case PersonalCaddieEventType::NOTIFICATIONS_TOGGLE:
	{
		//We've either turned data notifications on or off. If we've turned them on, then we need
		//to put the Personal Caddie into the power mode dictated by the m_modeState variable. If we've
		//turned them off then we put the Personal Caddie into sensor idle mode to stop transfering data.

		//Note: Notifactions need to be enabled before data transfer starts, which is why we wait to change into 
		//sensor active mode until this event occurs. The opposite is true when turning notifications off, we need
		//to leave sensor active mode first, and then disable notifications. For that reason we don't leave
		//sensor active mode in this event.

		//Enabling notifications have the ability to a
		std::wstring alertText = L"Data Notifications have been turned " + *((std::wstring*)eventArgs) + L"\n";
		createAlert(alertText, UIColor::Blue);

		if (*((std::wstring*)eventArgs) == L"On")
		{
			//TODO: Due to a timing bug in the firmware we need to wait for notifications to be turned on before
			//entering sensor idle mode, and we must enter sensor idle mode before sensor active mode. This means 
			//that any time notifications are enabled we should default to changing the power mode to sensor idle
			m_personalCaddie->changePowerMode(PersonalCaddiePowerMode::SENSOR_IDLE_MODE);
		}
		else m_modes[static_cast<int>(m_currentMode)]->ble_NotificationsChange(0);
		
		break;
	}
	case PersonalCaddieEventType::DATA_READY:
	{
		//The imu on the personal caddie has finished taking readings and has sent the data over.
		//Send the data to the current mode if it needs it.

		//TODO: Remove the mode specific logic, it should be the same regardless of the mode
		
		if (m_currentMode == ModeType::GRAPH_MODE || m_currentMode == ModeType::CALIBRATION)
		{
			m_modes[static_cast<int>(m_currentMode)]->addData(m_personalCaddie->getSensorData(), m_personalCaddie->getMaxODR(), m_personalCaddie->getDataTimeStamp(), m_personalCaddie->getNumberOfSamples());
		}
		else if (m_currentMode == ModeType::MADGWICK || m_currentMode == ModeType::FREE)
		{
			m_modes[static_cast<int>(m_currentMode)]->addQuaternions(m_personalCaddie->getQuaternions(), m_personalCaddie->getNumberOfSamples(), m_personalCaddie->getCurrentTime(), 1.0f / m_personalCaddie->getMaxODR());
			m_modes[static_cast<int>(m_currentMode)]->addData(m_personalCaddie->getSensorData(), m_personalCaddie->getMaxODR(), m_personalCaddie->getDataTimeStamp(), m_personalCaddie->getNumberOfSamples());
		}
		break;
	}
	case PersonalCaddieEventType::PC_ERROR:
	{
		//Some sort of error occured on the nRF device, whether it be a TWI read error, notification set
		//up error, or something similar. All we do for now is just create an alert in red text which 
		//displays the pertinent error code.

		std::wstring alertText = *((std::wstring*)eventArgs);
		createAlert(alertText, UIColor::Red);
		break;
	}
	}

}

void ModeScreen::ModeHandler(ModeAction action, void* eventArgs)
{
	//Since the individual mode classes don't have direct access to the Personal Caddie class, BLE class, etc.,
	//this handler method is used to forward on actions that the current mode needs to the Personal 
	//Caddie and other classes. As an example, when Discovery mode is active,
	//we need to turn on the BLE class's device watcher to start scanning for Personal Caddie devices.
	//When the scan button is clicked in the mode it forwards the request to this handler method, which 
	//then asks the BLE class to turn on the device watcher. The decision to deny direct access between
	//Personal Caddie and mode classes was deliberate. The mode screen class can almost be thought of as
	//like ther service layer in a three layerd software architecture.
	switch (action)
	{
	case ChangeMode:
	{
		//Pressing certain keys on the keyboard will cause us to change to a new mode. Since the 
		//individual modes aren't aware of the other modes existence, the task of switching between
		//them can only be carried out by the Mode Screen class. An instance of the ModeType enum is 
		//wrapped inside of the eventArgs, which alerts this method where we want to navigate to.
		ModeType newMode = *((ModeType*)eventArgs);
		changeCurrentMode(newMode);
		break;
	}
	case PersonalCaddieChangeMode:
	{
		//We want to change the power mode of the personal caddie. The event args
		//in this case will be an instance of the PersonalCaddiePowerMode enum. Other
		//handler methods will handle getting data to the mode if and when necessary.
		
		//TODO: There's a bug in the firmware that messes up the timing between enabling
		//characteristic notifications and starting to read data. To prevent this, we need to
		//make sure that notifications are enabled BEFORE the sensor is put into sensor idle
		//mode. For now, hardcode it so that when entering idle mode from connected mode we
		//wait for notifications to be enabled first, and when entering connected mode from 
		//idle mode we disbale notifications AFTER.
		PersonalCaddiePowerMode currentMode = m_personalCaddie->getCurrentPowerMode();
		PersonalCaddiePowerMode newMode = *((PersonalCaddiePowerMode*)eventArgs);

		if (newMode == PersonalCaddiePowerMode::SENSOR_IDLE_MODE && currentMode == PersonalCaddiePowerMode::CONNECTED_MODE) m_personalCaddie->enableDataNotifications();
		else m_personalCaddie->changePowerMode(newMode);
		break;
	}
	case RendererGetTextSize:
	{
		//Some UIElements are sized so that they can perfectly fit their text inside of themselves,
		//the DropDownMenu is a good example of this. It isn't possible to know just how large the text
		//will be (in pixels) though until it actually gets rendered. This leads to a strange race scenario
		//where we need to create a UIElement with dimensions, and yet, can't know what those dimensions are
		//until after it's already been rendered. To help out with this, we can send a vector containing any
		//text that we need sized directly to the renderer. This will synchronously create the text while
		//the UIElement is being created, allowing us to take the dimensions that get placed in the UIText
		//objects and extracting them to the UIElement. --Warning-- this method can only be called from 
		//the main render loop or it can lead to unexpected errors.
		std::vector<UIText*> const& text = *((std::vector<UIText*>*)eventArgs);
		getTextRenderPixels(text);
		break;
	}
	case RendererGetMaterial:
	{
		//All 3D materials to be rendered are controlled by the MasterRenderer class. When a particular mode
		//has something to be rendered on screen we need to grab the necessary materials from the renderer
		//and give them to the current mode. Since this is all done with references, and the ModeScreen class
		//can directly access the volume elements and material vectors of the current mode the eventArgs doesn't
		//actually contain anything here.
		std::vector<std::shared_ptr<VolumeElement> > const& volumeElements = m_modes[static_cast<int>(m_currentMode)]->getVolumeElements();
		std::vector<MaterialType> const& materialTypes = m_modes[static_cast<int>(m_currentMode)]->getMaterialTypes();
		for (int i = 0; i < volumeElements.size(); i++)
		{
			m_renderer->setMaterialAndMesh(volumeElements[i], materialTypes[i]);
		}
		break;
	}
	case BLEConnection:
	{
		//There are a few actions we can take regarding the BLE class. We can see the current connection status
		//(or we connected to a device or not), we can attempt to connect to a device, and we can disconnect from 
		//the currently connected device. Wrapped inside of the eventArgs is an instance of the BLEState enum class
		//as well as a 64-bit integer. This integer represents the address of a device that we potentially want to
		//connect to.
		std::pair<BLEState, uint64_t> state = *((std::pair<BLEState, uint64_t>*)eventArgs);
		switch (state.first)
		{
		case BLEState::Connected:
		default:
			//In this case we're simply curious to see if we're currently connected to a BLEDevice
			getCurrentMode()->getBLEConnectionStatus(m_personalCaddie->bleConnectionStatus());
			break;
		case BLEState::Disconnect:
			//Disconnect from the current device
			m_personalCaddie->disconnectFromDevice();
			break;
			//
		case BLEState::Reconnect:
			m_personalCaddie->connectToDevice(state.second);
			break;
		}
		break;
	}
	case BLEDeviceWatcher:
	{
		//For the BLE Device Watcher we can more or less take the same actions as for the BLE Connection. That is,
		//we can confirm if it's currently running, we can turn it on, or we can turn it off. The device watcher status
		//is wrapped inside of the same BLEState enum class as above.
		BLEState state = *((BLEState*)eventArgs);
		switch (state)
		{		
		case BLEState::DeviceWatcherStatus:
		default:
			//In this case we're simply curious to see if the device watcher is already on
			getCurrentMode()->getBLEDeviceWatcherStatus(m_personalCaddie->bleDeviceWatcherStatus());
			break;
		case BLEState::DisableDeviceWatcher:
			//Turn off the device watcher
			m_personalCaddie->stopBLEAdvertisementWatcher();
			break;
		case BLEState::EnableDeviceWatcher:
			//Turn on the device watcher
			m_personalCaddie->startBLEAdvertisementWatcher();
			break;
		}
		break;
	}
	case BLENotifications:
	{
		//We pass in a value of 1 to enable notifications and a value of 0 to disable them.
		//Cast the evenArgs to an integer to see what we need to do here.
		if (*((int*)eventArgs)) m_personalCaddie->enableDataNotifications();
		else m_personalCaddie->disableDataNotifications();

		break;
	}
	case SensorCalibration:
	{
		//There are four main actions we can take here. We can get the current sensor calibration, set the current sensor calibration,
		//get the IMU axis calibration or set the IMU axis calibration. Since there are 3 different sensors that make up the IMU then
		//in reality there are 8 different things we can do here. One of these 8 options is wrapped inside the eventArgs. If we just
		//want to get an existing cal all we need is the action, however, if we want to update an existing cal then we need the new
		//numbers. These are also wrapped inside of the event args. A struct called CalibrationRequest is used to wrap all of this info
		//together.
		auto cal_info = *((CalibrationRequest*)eventArgs);
		switch (cal_info.action)
		{
		case SensorCalibrationAction::GET_SENSOR_CAL:
			getCurrentMode()->getSensorCalibrationNumbers(cal_info.sensor, m_personalCaddie->getSensorCalibrationNumbers(cal_info.sensor));
			break;
		case SensorCalibrationAction::SET_SENSOR_CAL:
			m_personalCaddie->updateSensorCalibrationNumbers(cal_info.sensor, cal_info.cal_numbers);
			break;
		case SensorCalibrationAction::GET_SENSOR_AXIS_CAL:
			getCurrentMode()->getSensorAxisCalibrationNumbers(cal_info.sensor, m_personalCaddie->getSensorAxisCalibrationNumbers(cal_info.sensor));
			break;
		case SensorCalibrationAction::SET_SENSOR_AXIS_CAL:
			m_personalCaddie->updateSensorAxisOrientations(cal_info.sensor, cal_info.axis_numbers);
			break;
		}
		break;
	}
	case SensorSettings:
	{
		//For the sensor settings there are two actions we can take. Either we get the current settings, or, we update the 
		//settings. The eventArgs in this case will be a pointer to an uint8_t type. If the pointer is null it means we want
		//to simply get the current settings. If the pointer isn't null though, it's pointing to an array with new settings that
		//will be used to overwrite the current ones.
		uint8_t* settings = (uint8_t*)eventArgs;
		if (settings == nullptr) ((IMUSettingsMode*)m_modes[static_cast<int>(m_currentMode)].get())->getCurrentSettings(m_personalCaddie->getIMUSettings(), m_personalCaddie->getAvailableSensors());
		else m_personalCaddie->updateIMUSettings(settings);
		break;
	}
	case IMUHeading:
	{
		//Gets or sets the heading offset quaternion from the IMU class. The Madgwick filter uses magnetic north as the reference direction for 
		//Earth's magnetic field, so if the computer monitor isn't aligned perfectly North-South, then anything being rendered on screen
		//will be offset. Rotating all images being rendered on screen by this offset will effectively change the direction of magnetic
		//North to line up with the computer monitor so all images will match their real world counter parts. If a nullptr is passed in as the
		//eventArgs it means we simply want to get the current heading. If we want to set a new heading offset then a qlm::Quat stucture is
		//passed in.
		
		if (eventArgs == nullptr) getCurrentMode()->getIMUHeadingOffset(m_personalCaddie->getHeadingOffset());
		else
		{
			glm::quat heading = *((glm::quat*)eventArgs);
			m_personalCaddie->setHeadingOffset(heading);
		}
		break;
	}
	case MadgwickUpdateFilter:
	{
		//We can alter the beta value for the Madgwick filter to allow for a quicker convergence of the current rotation quaternion.
		//The higher the beta value is, the more heavily biased the filter is towards the accelerometer and magnetometer readings as opposed
		//to the gyroscope. Once the orientation of the image on screen has converged on its real world orientation, we can then lower the 
		//beta value of the filter back to its standard value so the readings once again are more heavily weighted towards the gyro.
		float beta_value = *((float*)eventArgs);
		m_personalCaddie->setMadgwickBeta(beta_value);
		break;
	}
	default: return;
	}
}

bool ModeScreen::needs3DRendering()
{
	//return true if the current mode requires 3D rendering and all 3D resources have
	//been loaded
	return m_modes[static_cast<int>(m_currentMode)]->m_needsCamera;
}