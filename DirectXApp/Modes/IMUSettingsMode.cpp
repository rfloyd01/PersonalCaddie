#include "pch.h"
#include "IMUSettingsMode.h"

//Below includes are for string to wstring conversion
#include <locale>
#include <codecvt>

IMUSettingsMode::IMUSettingsMode()
{
	//set a light gray background color for the mode
	m_backgroundColor = UIColor::LightBlue;
}

uint32_t IMUSettingsMode::initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState)
{
	//Create UI Elements on the page
	std::wstring buttonText = L"Get Current Settings";
	TextButton updateButton(windowSize, { 0.5, 0.225 }, { 0.12, 0.1 }, buttonText);

	m_uiElements.push_back(std::make_shared<TextButton>(updateButton));

	initializeTextOverlay(windowSize);

	m_state = initialState;

	//initialize the vector which will hold drop down text
	m_dropDownText = { {}, {}, {} };
	m_dropDownCategories = {};
	for (int i = 0; i < 3; i++)
	{
		//m_dropDownText.push_back({});
		for (int j = SENSOR_MODEL; j <= EXTRA_2; j++) m_dropDownText[i].push_back(L"");
	}
	dropDownsSet = false; //This won't get set to true until all drop downs are sized and placed
	m_state |= IMUSettingsState::GET_SETTINGS;

	//When this mode is initialzed we go into a state of CanTransfer and Active.
	//Can Transfer allows us to use the esc. key to go back to the settings menu
	//while active diverts state control to this mode
	return (ModeState::CanTransfer);
}

void IMUSettingsMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	for (int i = 0; i < m_uiElements.size(); i++) m_uiElements[i] = nullptr;
	m_uiElements.clear();
}

void IMUSettingsMode::initializeTextOverlay(winrt::Windows::Foundation::Size windowSize)
{
	//Title information
	std::wstring title_message = L"IMU Settings";
	TextOverlay title(windowSize, { UIConstants::TitleTextLocationX, UIConstants::TitleTextLocationY }, { UIConstants::TitleTextSizeX, UIConstants::TitleTextSizeY },
		title_message, UIConstants::TitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)title_message.length() }, UITextJustification::CenterCenter);
	m_uiElements.push_back(std::make_shared<TextOverlay>(title));

	//Body information
	std::wstring body_message =
		L"The IMU settings page allows us to change the settings on the individual sensors of the Personal Caddie. As examples, you could change the "
		L"full scale range of of the Accelerometer from +/- 2g to +/- 8g, you could turn off the Gyroscope high pass filter, etc. Clicking the button "
		L"above will populate drop down menus with the current settings. Change the settings as desired and click the button again to apply the changes.";
	TextOverlay body(windowSize, { UIConstants::BodyTextLocationX, UIConstants::BodyTextLocationY }, { UIConstants::BodyTextSizeX, UIConstants::BodyTextSizeY },
		body_message, 0.045, { UIColor::White }, { 0,  (unsigned int)title_message.length() }, UITextJustification::UpperLeft);
	m_uiElements.push_back(std::make_shared<TextOverlay>(body));

	//Footnote information
	std::wstring footnote_message = L"Press Esc. to return to settings menu.";
	TextOverlay footnote(windowSize, { UIConstants::FootNoteTextLocationX, UIConstants::FootNoteTextLocationY }, { UIConstants::FootNoteTextSizeX, UIConstants::FootNoteTextSizeY },
		footnote_message, UIConstants::FootNoteTextPointSize, { UIColor::White }, { 0,  (unsigned int)footnote_message.length() }, UITextJustification::LowerRight);
	m_uiElements.push_back(std::make_shared<TextOverlay>(footnote));
}

void IMUSettingsMode::getCurrentSettings(winrt::Windows::Foundation::Size windowSize, std::vector<uint8_t*> settings, std::vector<uint8_t> const& availableSensors, bool use_current)
{
	if (!use_current)
	{
		//The vector passed into this method holds three pointers. One each that
	//points to the settings in the accelerometer, gyroscope and magnetometer
	//classes. We copy these into the m_currentSettings array
		for (int i = 0; i < 3; i++)
		{
			//TODO: shouldn't be using the number 10 here, should be getting this
			//value from the sensor_settings.h file somewhere to allow future updates
			for (int j = 0; j < 10; j++)
			{
				//the additional 1 is because the very first setting in the array
				//holds the current Personal Caddie power mode and nothing about
				//the sensors themselves.
				m_currentSettings[10 * i + j + 1] = *(settings[i] + j);
				m_newSettings[10 * i + j + 1] = *(settings[i] + j); //the new settings array starts as a copy of the current settings
			}
		}

		//Take the given list of available sensors and populate the appropriate vectors and drop down boxes
		for (int i = 0; i < 10; i++)
		{
			if (availableSensors[i] == 0xFF) break; //signals the end of available sensors on the bus
			m_internalSensors.push_back(availableSensors[i]);
		}

		for (int i = 10; i < 20; i++)
		{
			if (availableSensors[i] == 0xFF) break; //signals the end of available sensors on the bus
			m_externalSensors.push_back(availableSensors[i]);
		}

		std::wstring acc_sensors = L"", gyr_sensors = L"", mag_sensors = L"";
		for (int i = 0; i < m_internalSensors.size(); i++)
		{
			std::wstring acc_sensor = get_sensor_model_string_from_address(ACC_SENSOR, m_internalSensors[i]);
			std::wstring gyr_sensor = get_sensor_model_string_from_address(GYR_SENSOR, m_internalSensors[i]);
			std::wstring mag_sensor = get_sensor_model_string_from_address(MAG_SENSOR, m_internalSensors[i]);

			if (acc_sensor != L"") acc_sensors += (acc_sensor + L"\n");
			if (gyr_sensor != L"") gyr_sensors += (gyr_sensor + L"\n");
			if (mag_sensor != L"") mag_sensors += (mag_sensor + L"\n");
		}

		for (int i = 0; i < m_externalSensors.size(); i++)
		{
			std::wstring acc_sensor = get_sensor_model_string_from_address(ACC_SENSOR, m_externalSensors[i]);
			std::wstring gyr_sensor = get_sensor_model_string_from_address(GYR_SENSOR, m_externalSensors[i]);
			std::wstring mag_sensor = get_sensor_model_string_from_address(MAG_SENSOR, m_externalSensors[i]);

			if (acc_sensor != L"") acc_sensors += (acc_sensor + L"\n");
			if (gyr_sensor != L"") gyr_sensors += (gyr_sensor + L"\n");
			if (mag_sensor != L"") mag_sensors += (mag_sensor + L"\n");
		}

		//Remove any trailing new line characters from the sensor dropdown text
		if (acc_sensors != L"") acc_sensors = acc_sensors.substr(0, acc_sensors.length() - 1);
		if (gyr_sensors != L"") gyr_sensors = gyr_sensors.substr(0, gyr_sensors.length() - 1);
		if (mag_sensors != L"") mag_sensors = mag_sensors.substr(0, mag_sensors.length() - 1);

		m_dropDownText[ACC_SENSOR][SENSOR_MODEL] = acc_sensors;
		m_dropDownText[GYR_SENSOR][SENSOR_MODEL] = gyr_sensors;
		m_dropDownText[MAG_SENSOR][SENSOR_MODEL] = mag_sensors;
	}

	//After getting the current settings we need to add some text, as well as
	//all of the drop down menus on screen
	createDropDownMenus(windowSize, use_current);
	m_state ^= IMUSettingsState::GET_SETTINGS; //once the drop downs are populated we remove the GetSettings state
}

void IMUSettingsMode::createDropDownMenus(winrt::Windows::Foundation::Size windowSize, bool use_current)
{
	//The screen is split into three columns, one each for the acc, gyr and mag sensors.
	//Each of these columns features 7 drop down menus.

	if (!use_current)
	{
		//First split the screen into three columns, each with its own sub-title
		std::wstring sub_title = L"Accelerometer Settings";
		TextOverlay acc(windowSize, { 0.15, 0.35 }, { 0.33, 0.1 },
			sub_title, UIConstants::SubTitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)sub_title.length() }, UITextJustification::UpperCenter);

		sub_title = L"Gyroscope Settings";
		TextOverlay gyr(windowSize, { 0.5, 0.35 }, { 0.33, 0.1 },
			sub_title, UIConstants::SubTitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)sub_title.length() }, UITextJustification::UpperCenter);

		sub_title = L"Magnetometer Settings";
		TextOverlay mag(windowSize, { 0.85, 0.35 }, { 0.33, 0.1 },
			sub_title, UIConstants::SubTitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)sub_title.length() }, UITextJustification::UpperCenter);

		sub_title = L"Model";
		TextOverlay acc_mod(windowSize, { 0.15, 0.42 }, { 0.33, 0.1 },
			sub_title, 0.025, { UIColor::White }, { 0,  (unsigned int)sub_title.length() }, UITextJustification::UpperCenter);

		//sub_title = L"Gyr. Model";
		TextOverlay gyr_mod(windowSize, { 0.5, 0.42 }, { 0.33, 0.1 },
			sub_title, 0.025, { UIColor::White }, { 0,  (unsigned int)sub_title.length() }, UITextJustification::UpperCenter);

		//sub_title = L"Mag. Model";
		TextOverlay mag_mod(windowSize, { 0.85, 0.42 }, { 0.33, 0.1 },
			sub_title, 0.025, { UIColor::White }, { 0,  (unsigned int)sub_title.length() }, UITextJustification::UpperCenter);

		Line line1(windowSize, { 0.33, 0.3 }, { 0.33, 0.92 }, UIColor::White, 2.0f);
		Line line2(windowSize, { 0.67, 0.3 }, { 0.67, 0.92 }, UIColor::White, 2.0f);

		m_uiElements.push_back(std::make_shared<TextOverlay>(acc));
		m_uiElements.push_back(std::make_shared<TextOverlay>(gyr));
		m_uiElements.push_back(std::make_shared<TextOverlay>(mag));
		m_uiElements.push_back(std::make_shared<TextOverlay>(acc_mod));
		m_uiElements.push_back(std::make_shared<TextOverlay>(gyr_mod));
		m_uiElements.push_back(std::make_shared<TextOverlay>(mag_mod));
		m_uiElements.push_back(std::make_shared<Line>(line1));
		m_uiElements.push_back(std::make_shared<Line>(line2));

		//Then, add the drop down menus. The width of the drop down menus is
		//dependent on the length of the text inside them, so final placements
		//will be calculated separately.

		//The first drop downs added hold the names of the sensors that we can switch to
		DropDownMenu acc_menu(windowSize, { 0.15, 0.43 }, { 0.15, 0.1 }, m_dropDownText[ACC_SENSOR][SENSOR_MODEL], 0.0225); //the locations will get set by a separate method
		DropDownMenu gyr_menu(windowSize, { 0.5, 0.43 }, { 0.15, 0.1 }, m_dropDownText[GYR_SENSOR][SENSOR_MODEL], 0.0225); //the locations will get set by a separate method
		DropDownMenu mag_menu(windowSize, { 0.85, 0.43 }, { 0.15, 0.1 }, m_dropDownText[MAG_SENSOR][SENSOR_MODEL], 0.0225); //the locations will get set by a separate method

		m_uiElements.push_back(std::make_shared<DropDownMenu>(acc_menu));
		m_uiElements.push_back(std::make_shared<DropDownMenu>(gyr_menu));
		m_uiElements.push_back(std::make_shared<DropDownMenu>(mag_menu));
	}

	//the text for each drop down menu is specific to the sensors on the chip so we call a separate method to get the strings
	populateDropDownText();

	m_accFirstDropDown = m_uiElements.size(); //save the location of the first drop down menu for later reference
	m_gyrFirstDropDown = m_accFirstDropDown + 9;
	m_magFirstDropDown = m_gyrFirstDropDown + 9;

	for (int i = 0; i < 3; i++)
	{
		for (int j = FS_RANGE; j <= EXTRA_2; j++)
		{
			DropDownMenu menu(windowSize, { 0, 0 }, { 0.15, 0.1 }, m_dropDownText[i][j], 0.0225); //the locations will get set by a separate method
			m_uiElements.push_back(std::make_shared<DropDownMenu>(menu));
		}
	}
}

void IMUSettingsMode::populateDropDownText()
{
	//create a converter from string to wstring
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

	if (m_newSettings[ACC_START] == LSM9DS1_ACC)
		for (int i = FS_RANGE; i <= EXTRA_2; i++) m_dropDownText[ACC_SENSOR][i] = lsm9ds1_get_complete_settings_string(ACC_SENSOR, static_cast<sensor_settings_t>(i));
	else if (m_newSettings[ACC_START] == FXOS8700_ACC)
		for (int i = FS_RANGE; i <= EXTRA_2; i++) m_dropDownText[ACC_SENSOR][i] = fxas_fxos_get_complete_settings_string(ACC_SENSOR, static_cast<sensor_settings_t>(i));

	if (m_newSettings[GYR_START] == LSM9DS1_GYR)
		for (int i = FS_RANGE; i <= EXTRA_2; i++) m_dropDownText[GYR_SENSOR][i] = lsm9ds1_get_complete_settings_string(GYR_SENSOR, static_cast<sensor_settings_t>(i));
	else if (m_newSettings[GYR_START] == FXAS21002_GYR)
		for (int i = FS_RANGE; i <= EXTRA_2; i++) m_dropDownText[GYR_SENSOR][i] = fxas_fxos_get_complete_settings_string(GYR_SENSOR, static_cast<sensor_settings_t>(i));

	if (m_newSettings[MAG_START] == LSM9DS1_MAG)
		for (int i = FS_RANGE; i <= EXTRA_2; i++) m_dropDownText[MAG_SENSOR][i] = lsm9ds1_get_complete_settings_string(MAG_SENSOR, static_cast<sensor_settings_t>(i));
	else if (m_newSettings[MAG_START] == FXOS8700_MAG)
		for (int i = FS_RANGE; i <= EXTRA_2; i++) m_dropDownText[MAG_SENSOR][i] = fxas_fxos_get_complete_settings_string(MAG_SENSOR, static_cast<sensor_settings_t>(i));
}

uint32_t IMUSettingsMode::handleUIElementStateChange(int i)
{
	if (i == 0)
	{
		//This is the get/set settings button
		if (!(m_state & IMUSettingsState::DISPLAY_SETTINGS))
		{
			//this will put us into the active state, causing the
			//mode screen class to give the current sensor settings.
			m_state |= IMUSettingsState::DISPLAY_SETTINGS;

			//Change the text of button and disable it. The button
			//only becomes enabled if any settings have actually changed.
			m_uiElements[0]->getText()->message = L"Update Settings";
			m_uiElements[0]->setState(m_uiElements[0]->getState() | UIElementState::Disabled);

			//We also erase the body text so that we can actually
			//display the option dropdowns.
			m_uiElements.erase(m_uiElements.begin() + 2);
		}
		else
		{
			//clicking the update settings button while it's active will cause
			//the settings to be updated on the actual device.
			m_state |= IMUSettingsState::UPDATE_SETTINGS;
		}
	}
	else if (i >= m_accFirstDropDown - 3)
	{
		//One of the drop down menus was clicked, see if an option was selected. If so, update the m_newSettings
		//array and compare it to the m_currentSettings array. If the arrays are different we can enable to 
		//update settings button at the top of the page. If they're the same, the button is disabled.
		if (m_uiElements[i]->getChildren()[2]->getState() & UIElementState::Invisible)
		{
			//The drop down menu scroll box becomes invisible as soon as we select a new option
			std::wstring selectedOption = ((FullScrollingTextBox*)m_uiElements[i]->getChildren()[2].get())->getLastSelectedText();
			selectedOption = selectedOption.substr(selectedOption.find(L"0x") + 2); //extract the hexadecimal at the end of the option
			
			//Update the new settings array
			int sensor = 0;
			if (i >= m_gyrFirstDropDown || i == m_accFirstDropDown - 2) sensor = 1;
			if (i >= m_magFirstDropDown || i == m_accFirstDropDown - 1) sensor = 2;

			uint8_t newSetting = convertStringToHex(selectedOption);

			//Before applying the new setting we need to see if it's a compound setting that will potentially alter the
			//values in other text boxes. In most cases we'll only change other options on the same sensor, however, there
			//are instances where setting on other sensors will be affected as well. For example, if both the LSM9DS1 
			//accelerometer and gyrocsope are used then their ODR and Power levels are tied together. We call a separate
			//method to help us update all settings appropriately.
			sensor_settings_t cat = SENSOR_MODEL;
			if (i >= m_accFirstDropDown) cat = static_cast<sensor_settings_t>(m_dropDownCategories[i - m_accFirstDropDown]);
			updateSetting(static_cast<sensor_type_t>(sensor), cat, newSetting);

			bool different = false;

			for (int j = 0; j < SENSOR_SETTINGS_LENGTH; j++)
			{
				if (m_currentSettings[j] != m_newSettings[j])
				{
					//the settings have now been changed, this means we enable the Update button
					m_uiElements[0]->removeState(UIElementState::Disabled);
					different = true;
					break; //only need one byte to be different to enabled updating
				}
			}

			if (!different)
			{
				//The settings haven't been altered from their original form, so disable the update button
				m_uiElements[0]->setState(m_uiElements[0]->getState() | UIElementState::Disabled);
			}

			if (i < m_accFirstDropDown && different)
			{
				//If we've selected a different sensor it means we need to load a new set of drop down menus.
				//The easiest way to do this is to just remove all current drop downs and load everything
				//from scratch using the current settings array.

				//First erase the labels for each settings drop down (these are located before the sensor options menus)
				int stop = m_uiElements.size() - m_accFirstDropDown;

				for (int j = m_uiElements.size() - 1; j >= m_accFirstDropDown; j--) m_uiElements[j] = nullptr;
				m_uiElements.erase(m_uiElements.begin() + m_accFirstDropDown, m_uiElements.end());

				for (int j = m_accFirstDropDown - 4; j > (m_accFirstDropDown - 4 - stop); j--) m_uiElements[j] = nullptr;
				m_uiElements.erase(m_uiElements.begin() + (m_accFirstDropDown - 3 - stop), m_uiElements.begin() + (m_accFirstDropDown - 3));

				m_dropDownText = { {}, {}, {} };
				m_dropDownCategories = {};
				for (int i = 0; i < 3; i++)
				{
					//m_dropDownText.push_back({});
					for (int j = SENSOR_MODEL; j <= EXTRA_2; j++) m_dropDownText[i].push_back(L"");
				}

				//After removing the current drop down menus, update the m_newSettings array with the default values
				//of the newly selected sensor
				uint8_t sensor_starts[3] = { ACC_START, GYR_START, MAG_START };
				get_sensor_default_settings(sensor, newSetting, &m_newSettings[sensor_starts[sensor]]);

				dropDownsSet = false; //This won't get set to true until all drop downs are sized and placed
				m_state |= IMUSettingsState::GET_SETTINGS; //signals the mode screen that we need to recreate drop downs
			}
		}
	}
	return m_state;
}

void IMUSettingsMode::updateSetting(sensor_type_t sensor_type, sensor_settings_t setting_type, uint8_t setting)
{
	//This method will look at the setting and update it for the given sensor and setting type. This function
	//will also figure out if any other settings should be updated as a result of this setting being updated.
	//For example, turning off a sensor will also change its ODR to 0 Hz. As another example, if we have an 
	//LSM9DS1 acc and LSM9DS1 gyro activated, their ODR's need to be equal, so changing one will cascade to
	//the other
	int sensor_start_locations[3] = { ACC_START, GYR_START, MAG_START }; //useful to know where each sensor's settings begins

	//Before updating anything, check to see if we're updating a sensor model type. This has the effect
	//of changing all the drop downs for a certain sensor as opposed to just changing a few of them.
	//This will be handled elsewhere so just update the settings array and return from this function for now
	if (setting_type == SENSOR_MODEL)
	{
		m_newSettings[sensor_start_locations[sensor_type] + setting_type] = setting;
		return;
	}

	//Before making any updates, save the locations of some key drop down menus in the uiElement array.
	int accOdrIndex = 0, gyrOdrIndex = 0, accPowerIndex = 0, gyrPowerIndex = 0, magOdrIndex = 0, magPowerIndex = 0; //figure out which dropdown menu represents the acc ODR
	for (int i = 0; i < (m_gyrFirstDropDown - m_accFirstDropDown); i++)
	{
		if (m_dropDownCategories[i] == ODR) accOdrIndex = i;
		else if (m_dropDownCategories[i] == POWER) accPowerIndex = i;
	}

	for (int i = m_gyrFirstDropDown - m_accFirstDropDown; i < (m_magFirstDropDown - m_accFirstDropDown); i++)
	{
		if (m_dropDownCategories[i] == ODR) gyrOdrIndex = i;
		else if (m_dropDownCategories[i] == POWER) gyrPowerIndex = i;
	}

	for (int i = m_magFirstDropDown - m_accFirstDropDown; i < m_dropDownCategories.size(); i++)
	{
		if (m_dropDownCategories[i] == ODR) magOdrIndex = i;
		else if (m_dropDownCategories[i] == POWER) magPowerIndex = i;
	}

	uint8_t newSetting = m_newSettings[sensor_start_locations[sensor_type] + setting_type];

	if (m_newSettings[ACC_START] == LSM9DS1_ACC && m_newSettings[GYR_START] == LSM9DS1_GYR)
	{
		//We have both an LSM9DS1 acc and gyro, which means that the ODR and Power settings of both sensors
		//are tied together.
		std::wstring acc_odr_text = m_uiElements[m_accFirstDropDown + accOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->message;
		lsm9ds1_update_acc_gyr_setting(m_newSettings, sensor_type, setting_type, &newSetting, setting, &acc_odr_text[0]);

		//Chain drop-down text changes as necessary if the power or odr setting was altered
		if (setting_type == ODR || setting_type == POWER)
		{
			m_newSettings[ACC_START + ODR] = newSetting;
			m_newSettings[ACC_START + POWER] = newSetting;
			m_newSettings[GYR_START + ODR] = newSetting;
			m_newSettings[GYR_START + POWER] = newSetting;

			//Make necessary changes to gyroscope drop downs
			m_uiElements[m_accFirstDropDown + gyrOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->message = lsm9ds1_get_settings_string(GYR_SENSOR, ODR, newSetting);
			m_uiElements[m_accFirstDropDown + gyrOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->colorLocations.back() = m_uiElements[m_accFirstDropDown + gyrOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->message.length();

			m_uiElements[m_accFirstDropDown + gyrPowerIndex]->getChildren()[0]->getChildren()[1]->getText()->message = lsm9ds1_get_settings_string(GYR_SENSOR, POWER, newSetting);
			m_uiElements[m_accFirstDropDown + gyrPowerIndex]->getChildren()[0]->getChildren()[1]->getText()->colorLocations.back() = m_uiElements[m_accFirstDropDown + gyrPowerIndex]->getChildren()[0]->getChildren()[1]->getText()->message.length();

			m_uiElements[m_accFirstDropDown + accOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->message = lsm9ds1_get_settings_string(ACC_SENSOR, ODR, newSetting);
			m_uiElements[m_accFirstDropDown + accOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->colorLocations.back() = m_uiElements[m_accFirstDropDown + accOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->message.length();

			m_uiElements[m_accFirstDropDown + accPowerIndex]->getChildren()[0]->getChildren()[1]->getText()->message = lsm9ds1_get_settings_string(ACC_SENSOR, POWER, newSetting);
			m_uiElements[m_accFirstDropDown + accPowerIndex]->getChildren()[0]->getChildren()[1]->getText()->colorLocations.back() = m_uiElements[m_accFirstDropDown + accPowerIndex]->getChildren()[0]->getChildren()[1]->getText()->message.length();
		}
	}
	else if (m_newSettings[ACC_START] == LSM9DS1_ACC)
	{
		//We have an LSM9DS1 acc that isn't tied to a gyro. Basically we just need to make sure that the power setting
		//and ODR match up.
		std::wstring acc_odr_text = m_uiElements[m_accFirstDropDown + accOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->message;
		lsm9ds1_update_acc_setting(m_newSettings, setting_type, &newSetting, setting);

		//Chain drop-down text changes as necessary if the power or odr setting was altered
		if (setting_type == ODR || setting_type == POWER)
		{
			m_newSettings[ACC_START + ODR] = newSetting;
			m_newSettings[ACC_START + POWER] = newSetting;

			m_uiElements[m_accFirstDropDown + accOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->message = lsm9ds1_get_settings_string(ACC_SENSOR, ODR, newSetting);
			m_uiElements[m_accFirstDropDown + accOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->colorLocations.back() = m_uiElements[m_accFirstDropDown + accOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->message.length();

			m_uiElements[m_accFirstDropDown + accPowerIndex]->getChildren()[0]->getChildren()[1]->getText()->message = lsm9ds1_get_settings_string(ACC_SENSOR, POWER, newSetting);
			m_uiElements[m_accFirstDropDown + accPowerIndex]->getChildren()[0]->getChildren()[1]->getText()->colorLocations.back() = m_uiElements[m_accFirstDropDown + accPowerIndex]->getChildren()[0]->getChildren()[1]->getText()->message.length();
		}
	}
	else if (m_newSettings[GYR_START] == LSM9DS1_GYR)
	{
		//We have an LSM9DS1 acc that isn't tied to a gyro. Basically we just need to make sure that the power setting
		//and ODR match up.
		lsm9ds1_update_gyr_setting(m_newSettings, setting_type, &newSetting, setting);

		//Chain drop-down text changes as necessary if the power or odr setting was altered
		if (setting_type == ODR || setting_type == POWER)
		{
			m_newSettings[GYR_START + ODR] = newSetting;
			m_newSettings[GYR_START + POWER] = newSetting;

			m_uiElements[m_accFirstDropDown + gyrOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->message = lsm9ds1_get_settings_string(GYR_SENSOR, ODR, newSetting);
			m_uiElements[m_accFirstDropDown + gyrOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->colorLocations.back() = m_uiElements[m_accFirstDropDown + gyrOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->message.length();

			m_uiElements[m_accFirstDropDown + gyrPowerIndex]->getChildren()[0]->getChildren()[1]->getText()->message = lsm9ds1_get_settings_string(GYR_SENSOR, POWER, newSetting);
			m_uiElements[m_accFirstDropDown + gyrPowerIndex]->getChildren()[0]->getChildren()[1]->getText()->colorLocations.back() = m_uiElements[m_accFirstDropDown + gyrPowerIndex]->getChildren()[0]->getChildren()[1]->getText()->message.length();
		}
	}
	else if (m_newSettings[MAG_START] == LSM9DS1_MAG)
	{
		std::wstring mag_odr_text = m_uiElements[m_accFirstDropDown + magOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->message;
		lsm9ds1_update_mag_setting(m_newSettings, setting_type, &newSetting, setting, &mag_odr_text[0]);

		//Update the odr and power drop down menu text as neceessary
		m_uiElements[m_accFirstDropDown + magOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->message = lsm9ds1_get_settings_string(MAG_SENSOR, ODR, m_newSettings[MAG_START + ODR]);
		m_uiElements[m_accFirstDropDown + magOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->colorLocations.back() = m_uiElements[m_accFirstDropDown + magOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->message.length();

		m_uiElements[m_accFirstDropDown + magPowerIndex]->getChildren()[0]->getChildren()[1]->getText()->message = lsm9ds1_get_settings_string(MAG_SENSOR, POWER, m_newSettings[MAG_START + POWER]);
		m_uiElements[m_accFirstDropDown + magPowerIndex]->getChildren()[0]->getChildren()[1]->getText()->colorLocations.back() = m_uiElements[m_accFirstDropDown + magPowerIndex]->getChildren()[0]->getChildren()[1]->getText()->message.length();
	}
	else
	{
		//We've selected a setting without any special effects. We just need to update the
		//m_new settings array accordingly.
		m_newSettings[sensor_start_locations[sensor_type] + setting_type] = setting;
	}
}

uint8_t IMUSettingsMode::convertStringToHex(std::wstring hexString)
{
	//This method takes a string which represents a hexadecimal number (without the leading 0x)
	//and converts it into an actual number. The longest hexString should be 2 digits.
	uint8_t number = 0;
	
	for (int i = 0; i < hexString.length(); i++)
	{
		wchar_t character = hexString[i];
		number <<= 4;

		if (character >= L'0' && character <= L'9')
		{
			//this is a normal number
			number += (uint8_t)(character - '0');
		}
		else if (character >= L'a' && character <= L'f')
		{
			//this is a lower case hex letter
			number += (uint8_t)(character - 'a') + 10;
		}
		else if (character >= L'A' && character <= L'F')
		{
			//this is an upper case hex letter
			number += (uint8_t)(character - 'A') + 10;
		}
		else
		{
			OutputDebugString(L"IMUSettingsMode: convertStringToHex: Invalid hex number supplied.\n");
			return 0;
		}
	}
	
	return number;
}

void IMUSettingsMode::update()
{
	//We use this method to help position all of the drop down menus. We don't know the width of the 
	//menus until after they've been created. Once this happens we placed them based on their width
	if (!dropDownsSet)
	{
		//Before attempting to move anything, make sure that all the text boxes have been resized.
		//Due to the timing of when the drop downs get createded in the main render loop, a full
		//render cycle will pass before they get resized.
		if (m_uiElements[m_accFirstDropDown]->getChildren()[0]->getAbsoluteSize().x == m_uiElements[m_accFirstDropDown]->getChildren()[1]->getAbsoluteSize().x) return;

		//We remove any drop down menus that don't actually have any text, so we need to use
		//a while loop to iterate over all the drop down menus instead of a for loop.
		int i = m_accFirstDropDown, index = 0, row = 0, sensor = 0, type = 0;
		std::pair<int, int> indices = { 0, 0 };
		float location = 1.0f / 6.0f, marginWidth; //the margin size is the width of two buttons (all drop down menus will have the same size button)

		//all drop down menu buttons should be the same size, find the first none-empty
		//drop box and take its button size.
		for (int j = i; j < m_uiElements.size(); j++)
		{
			if (m_uiElements[j]->getChildren()[0]->getChildren()[1]->getText()->message != L"")
			{
				marginWidth = m_uiElements[j]->getChildren()[1]->getAbsoluteSize().x * 1.0f;
				break;
			}
		}

		//We place two drop downs next to each other, so we need to know the width of both 
		//before figuring out their locations. The goal is to have the arrow buttons of all
		//the drop downs line up.
		int sensor_start_locations[3] = { ACC_START, GYR_START, MAG_START };
		while (i < m_uiElements.size())
		{
			type++; //this variable is used for poulating the drop downs with the current settings from the settings array

			if (m_uiElements[i]->getChildren()[0]->getChildren()[1]->getText()->message == L"")
			{
				m_uiElements.erase(m_uiElements.begin() + i);

				//if we're looking at the accelerometer then both gyroscope and magnetometer
				//starting indices need to be decremented. If we're looking at the gyroscope
				//only the magnetometer start needs to be decremented. If we're looking at the
				//magnetometer then no need to decrement anything.
				if (sensor < 1) m_gyrFirstDropDown--;
				if (sensor < 2)	m_magFirstDropDown--;
			}
			else
			{
				//First, choose the correct setting for this drop down menu
				std::wstring initialText;
				switch (m_newSettings[sensor_start_locations[sensor] + SENSOR_MODEL])
				{
				case LSM9DS1_ACC:
					initialText = lsm9ds1_get_settings_string(static_cast<sensor_type_t>(sensor), static_cast<sensor_settings_t>(type), m_newSettings[sensor_start_locations[sensor] + type]);
					break;
				case FXOS8700_ACC:
					initialText = fxas_fxos_get_settings_string(static_cast<sensor_type_t>(sensor), static_cast<sensor_settings_t>(type), m_newSettings[sensor_start_locations[sensor] + type]);
					break;
				}
				
				m_uiElements[i]->getChildren()[0]->getChildren()[1]->getText()->message = initialText;
				m_uiElements[i]->getChildren()[0]->getChildren()[1]->getText()->colorLocations.back() = initialText.length();
				m_dropDownCategories.push_back(type);

				//Then work on placing it
				if (index == 0)
				{
					indices.first = i;
					index++;
				}
				else if (index == 1)
				{
					indices.second = i;
					//This is the second option for this row. Since we now have
					//the lengths of both boxes we can set their locations
					m_uiElements[indices.first]->setAbsoluteLocation({ location - marginWidth - m_uiElements[indices.first]->getAbsoluteSize().x / 2.0f, 0.1f * (row + 3.0f) + 0.23f });
					m_uiElements[indices.second]->setAbsoluteLocation({ (1.0f / 6.0f + location) - marginWidth - m_uiElements[indices.second]->getAbsoluteSize().x / 2.0f, 0.1f * (row + 3.0f) + 0.23f });

					//after setting the location for each drop down it can be resized
					m_uiElements[indices.first]->resize(m_uiElements[indices.first]->getCurrentWindowSize()); //since only the location has changed, not the size, this is ok
					m_uiElements[indices.second]->resize(m_uiElements[indices.second]->getCurrentWindowSize()); //since only the location has changed, not the size, this is ok
					
					//update the row and reset the index
					row++;
					index = 0;
				}
				i++; //go to the next drop down box
			}

			if ((i == m_gyrFirstDropDown && sensor == 0) || (i == m_magFirstDropDown && sensor == 1) || (i == m_uiElements.size()))
			{
				//move onto the gyroscope drop downs
				if (index == 1)
				{
					//we ended a row with only a single drop down so put it in the middle of the column
					m_uiElements[indices.first]->setAbsoluteLocation({ location, 0.10f * (row + 3.0f) + 0.23f });
					m_uiElements[indices.first]->resize(m_uiElements[indices.first]->getCurrentWindowSize()); //since only the location has changed, not the size, this is ok
					index = 0;
				}

				sensor++;
				row = 0;
				type = 0;
				location += 1.0f / 3.0f;
			}
		}

		//int x = 10;

		//Once all of the drop down menus have been placed, we put labels over each of them
		for (int i = 0; i < m_dropDownCategories.size(); i++)
		{
			std::wstring category;
			switch (m_dropDownCategories[i])
			{
			case FS_RANGE:
				category = L"Full Scale Range";
				break;
			case ODR:
				category = L"Octal Data Rate";
				break;
			case POWER:
				category = L"Power Level";
				break;
			case FILTER_SELECTION:
				category = L"Filter Selection";
				break;
			case LOW_PASS_FILTER:
				category = L"Low Pass Filter";
				break;
			case HIGH_PASS_FILTER:
				category = L"High Pass Filter";
				break;
			case EXTRA_FILTER:
				category = L"Extra Filter";
				break;
			case EXTRA_1:
				category = L"Extra Settings";
				break;
			case EXTRA_2:
				category = L"Extra Settings";
				break;
			}

			auto dropDown = m_uiElements[i + m_accFirstDropDown].get();
			auto dropDownLocation = dropDown->getAbsoluteLocation();
			auto dropDownSize = dropDown->getAbsoluteSize();
			auto currentWindowSize = dropDown->getCurrentWindowSize();
			TextOverlay dropDownTitle(currentWindowSize, { dropDownLocation.x, dropDownLocation.y - dropDownSize.y }, { 0.25f, dropDownSize.y * 2.0f }, category, 0.025,
				{ UIColor::White }, { 0, (unsigned int)category.length() }, UITextJustification::UpperCenter); //any width will be ok as the title is centered over its respective drop down

			//Insert the titles before the actual drop down menus so that when the drop down scroll box pops out they
			//will cover the titles. The first drop down variables need to be increased accordingly
			m_uiElements.insert(m_uiElements.begin() + m_accFirstDropDown - 3, std::make_shared<TextOverlay>(dropDownTitle));

			m_accFirstDropDown++;
			m_gyrFirstDropDown++;
			m_magFirstDropDown++;
		}

		//The last thing we do before returning is to pick the approrpiate text
		//for the currently selected sensor drop down menus. Since theirs only one
		//of these boxes per column they're automatically placed in the center.
		std::wstring sensor_model = get_sensor_model_string(ACC_SENSOR, m_newSettings[ACC_START + SENSOR_MODEL]);
		m_uiElements[m_accFirstDropDown - 3]->getChildren()[0]->getChildren()[1]->getText()->message = sensor_model;
		m_uiElements[m_accFirstDropDown - 3]->getChildren()[0]->getChildren()[1]->getText()->colorLocations.back() = sensor_model.length();

		sensor_model = get_sensor_model_string(GYR_SENSOR, m_newSettings[GYR_START + SENSOR_MODEL]);
		m_uiElements[m_accFirstDropDown - 2]->getChildren()[0]->getChildren()[1]->getText()->message = sensor_model;
		m_uiElements[m_accFirstDropDown - 2]->getChildren()[0]->getChildren()[1]->getText()->colorLocations.back() = sensor_model.length();

		sensor_model = get_sensor_model_string(MAG_SENSOR, m_newSettings[MAG_START + SENSOR_MODEL]);
		m_uiElements[m_accFirstDropDown - 1]->getChildren()[0]->getChildren()[1]->getText()->message = sensor_model;
		m_uiElements[m_accFirstDropDown - 1]->getChildren()[0]->getChildren()[1]->getText()->colorLocations.back() = sensor_model.length();

		dropDownsSet = true;
	}
}

TextOverlay IMUSettingsMode::removeAlerts()
{
	//If an alert is removed from in front of the drop down boxes then we need to updated the starting locations
	//for each of the sensors. If not though then nothing needs to be changed
	TextOverlay alert({ 0, 0 }, { 0, 0 }, { 0, 0 }, L"", 0, { UIColor::Black }, { 0, 0 }, UITextJustification::CenterCenter);
	for (int i = m_uiElements.size() - 1; i >= 0; i--)
	{
		if (m_uiElements[i]->isAlert())
		{
			alert = *((TextOverlay*)m_uiElements[i].get());
			m_uiElements[i] = nullptr;
			m_uiElements.erase(m_uiElements.begin() + i);

			if (i < m_accFirstDropDown)
			{
				m_accFirstDropDown--;
				m_gyrFirstDropDown--;
				m_magFirstDropDown--;
			}
			break;
		}
	}

	return alert;
}

void IMUSettingsMode::handlePersonalCaddieConnectionEvent(bool connectionStatus)
{
	//TODO::
	//If the connection is lost while on this page then disable all buttons and 
	//drop down menus, prompting the user to attempt to reconnect.
}