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
	//Take the current screen size and pass it to the UIElementManager, this is so that the manager knows
	//how large to make each element.
	m_uiManager.updateScreenSize(windowSize);

	//Create UI Elements on the page
	std::wstring buttonText = L"Update Settings";
	TextButton updateButton(windowSize, { 0.5, 0.225 }, { 0.12, 0.1 }, buttonText);
	Line line1(windowSize, { 0.33, 0.3 }, { 0.33, 0.92 }, UIColor::White, 2.0f);
	Line line2(windowSize, { 0.67, 0.3 }, { 0.67, 0.92 }, UIColor::White, 2.0f);

	m_uiManager.addElement<TextButton>(updateButton, L"Update Button");
	m_uiManager.addElement<Line>(line1, L"Line 1"); //separates sensor options into columns
	m_uiManager.addElement<Line>(line2, L"Line 2"); //separates sensor options into columns

	m_uiManager.getElement<TextButton>(L"Update Button")->updateState(UIElementState::Disabled);

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
	
	//Get the current sensor settings from the IMU class. Once we get these settings the 
	//getCurrentSettings() method will be called which will automatically create drop down menus
	//and populate them with text, based on the settings obtained from the IMU
	m_mode_screen_handler(ModeAction::SensorSettings, nullptr);

	//The above call to the m_mode_screen_handler() method will cause a multitude of drop down menus to 
	//be created, however, none of them will be in their proper locations. We call the update method
	//to not only place the drop downs on the page, but also to delete any empty drop down boxes.
	//update();

	//When this mode is initialzed we go into a state of CanTransfer and Active.
	//Can Transfer allows us to use the esc. key to go back to the settings menu
	//while active diverts state control to this mode
	return (ModeState::CanTransfer);
}

void IMUSettingsMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	m_uiManager.removeAllElements();

	//also, clear out the available sensor vectors
	m_internalSensors.clear();
	m_externalSensors.clear();
}

void IMUSettingsMode::initializeTextOverlay(winrt::Windows::Foundation::Size windowSize)
{
	//Title information
	std::wstring title_message = L"IMU Settings";
	TextOverlay title(windowSize, { UIConstants::TitleTextLocationX, UIConstants::TitleTextLocationY }, { UIConstants::TitleTextSizeX, UIConstants::TitleTextSizeY },
		title_message, UIConstants::TitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)title_message.length() }, UITextJustification::CenterCenter);
	m_uiManager.addElement<TextOverlay>(title, L"Title Text");

	//Footnote information
	std::wstring footnote_message = L"Press Esc. to return to settings menu.";
	TextOverlay footnote(windowSize, { UIConstants::FootNoteTextLocationX, UIConstants::FootNoteTextLocationY }, { UIConstants::FootNoteTextSizeX, UIConstants::FootNoteTextSizeY },
		footnote_message, UIConstants::FootNoteTextPointSize, { UIColor::White }, { 0,  (unsigned int)footnote_message.length() }, UITextJustification::LowerRight);
	m_uiManager.addElement<TextOverlay>(footnote, L"Footnote Text");
}

void IMUSettingsMode::getCurrentSettings(std::vector<uint8_t*> settings, std::vector<uint8_t> const& availableSensors, bool use_current)
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

		use_current = false; //this is needed in the instance that a new sensor is selected
	}

	getAvailableSensors();

	//After getting the current settings we need to add some text, as well as
	//all of the drop down menus on screen
	createDropDownMenus(m_uiManager.getScreenSize(), use_current);
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

		m_uiManager.addElement<TextOverlay>(acc, L"Acc Setting Text");
		m_uiManager.addElement<TextOverlay>(gyr, L"Gyr Setting Text");
		m_uiManager.addElement<TextOverlay>(mag, L"Mag Setting Text");
		m_uiManager.addElement<TextOverlay>(acc_mod, L"Acc Model Text");
		m_uiManager.addElement<TextOverlay>(gyr_mod, L"Gyr Model Text");
		m_uiManager.addElement<TextOverlay>(mag_mod, L"Mag Model Text");

		//Then, add the drop down menus. The width of the drop down menus is
		//dependent on the length of the text inside them, so final placements
		//will be calculated separately.

		//The first drop downs added hold the names of the sensors that we can switch to
		DropDownMenu acc_menu(windowSize, { 0.15, 0.43 }, { 0.15, 0.1 }, m_dropDownText[ACC_SENSOR][SENSOR_MODEL], 0.0225); //the locations will get set by a separate method
		DropDownMenu gyr_menu(windowSize, { 0.5, 0.43 }, { 0.15, 0.1 }, m_dropDownText[GYR_SENSOR][SENSOR_MODEL], 0.0225); //the locations will get set by a separate method
		DropDownMenu mag_menu(windowSize, { 0.85, 0.43 }, { 0.15, 0.1 }, m_dropDownText[MAG_SENSOR][SENSOR_MODEL], 0.0225); //the locations will get set by a separate method

		m_uiManager.addElement<DropDownMenu>(acc_menu, L"Acc Model Drop Down Menu");
		m_uiManager.addElement<DropDownMenu>(gyr_menu, L"Gyr Model Drop Down Menu");
		m_uiManager.addElement<DropDownMenu>(mag_menu, L"Mag Model Drop Down Menu");
	}

	//the text for each drop down menu is specific to the sensors on the chip so we call a separate method to get the strings
	populateDropDownText();

	//After getting the drop down text, create the physical drop down menus
	for (int i = 0; i < 3; i++)
	{
		std::wstring sensorType;
		if (i == 0) sensorType = L"Acc ";
		else if (i == 1) sensorType = L"Gyr ";
		else sensorType = L"Mag ";

		for (int j = FS_RANGE; j <= EXTRA_2; j++)
		{
			DropDownMenu menu(windowSize, { 0, 0 }, { 0.15, 0.1 }, m_dropDownText[i][j], 0.0225); //the locations will get set by a separate method
			m_uiManager.addElement<DropDownMenu>(menu, sensorType + L"Setting Drop Down Menu " + std::to_wstring(j));
		}
	}
}

void IMUSettingsMode::getAvailableSensors()
{
	//This method reads the values from the available sensors characteristic and populates the appropriate
	//element of the m_dropDownText vector
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

void IMUSettingsMode::populateDropDownText()
{
	//create a converter from string to wstring
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

	//ACC Text
	if (m_newSettings[ACC_START] == LSM9DS1_ACC)
	{
		for (int i = FS_RANGE; i <= EXTRA_2; i++) m_dropDownText[ACC_SENSOR][i] = lsm9ds1_get_complete_settings_string(ACC_SENSOR, static_cast<sensor_settings_t>(i));
	}
	else if (m_newSettings[ACC_START] == BMI270_ACC)
	{
		for (int i = FS_RANGE; i <= EXTRA_2; i++) m_dropDownText[ACC_SENSOR][i] = bmi_bmm_get_complete_settings_string(ACC_SENSOR, static_cast<sensor_settings_t>(i));
	}
	else if (m_newSettings[ACC_START] == FXOS8700_ACC)
	{
		for (int i = FS_RANGE; i <= EXTRA_2; i++)
		{
			if (i == ODR && m_newSettings[MAG_START + SENSOR_MODEL] == FXOS8700_MAG && m_newSettings[MAG_START + POWER] != 0xFF) m_dropDownText[ACC_SENSOR][i] = fxas_fxos_get_complete_settings_string(ACC_SENSOR, static_cast<sensor_settings_t>(100));
			else m_dropDownText[ACC_SENSOR][i] = fxas_fxos_get_complete_settings_string(ACC_SENSOR, static_cast<sensor_settings_t>(i));
		}
	}

	//GYR Text
	if (m_newSettings[GYR_START] == LSM9DS1_GYR)
	{
		for (int i = FS_RANGE; i <= EXTRA_2; i++) m_dropDownText[GYR_SENSOR][i] = lsm9ds1_get_complete_settings_string(GYR_SENSOR, static_cast<sensor_settings_t>(i));
	}
	else if (m_newSettings[GYR_START] == BMI270_GYR)
	{
		for (int i = FS_RANGE; i <= EXTRA_2; i++) m_dropDownText[GYR_SENSOR][i] = bmi_bmm_get_complete_settings_string(GYR_SENSOR, static_cast<sensor_settings_t>(i));
	}
	else if (m_newSettings[GYR_START] == FXAS21002_GYR)
	{
		for (int i = FS_RANGE; i <= EXTRA_2; i++) m_dropDownText[GYR_SENSOR][i] = fxas_fxos_get_complete_settings_string(GYR_SENSOR, static_cast<sensor_settings_t>(i));
	}

	//MAG Text
	if (m_newSettings[MAG_START] == LSM9DS1_MAG)
	{
		for (int i = FS_RANGE; i <= EXTRA_2; i++) m_dropDownText[MAG_SENSOR][i] = lsm9ds1_get_complete_settings_string(MAG_SENSOR, static_cast<sensor_settings_t>(i));
	}
	else if (m_newSettings[MAG_START] == BMM150_MAG)
	{
		for (int i = FS_RANGE; i <= EXTRA_2; i++) m_dropDownText[MAG_SENSOR][i] = bmi_bmm_get_complete_settings_string(MAG_SENSOR, static_cast<sensor_settings_t>(i));
	}
	else if (m_newSettings[MAG_START] == FXOS8700_MAG)
	{
		for (int i = FS_RANGE; i <= EXTRA_2; i++)
		{
			if (i == ODR && m_newSettings[ACC_START + SENSOR_MODEL] == FXOS8700_ACC && m_newSettings[ACC_START + POWER] != 0xFF) m_dropDownText[MAG_SENSOR][i] = fxas_fxos_get_complete_settings_string(MAG_SENSOR, static_cast<sensor_settings_t>(100));
			else m_dropDownText[MAG_SENSOR][i] = fxas_fxos_get_complete_settings_string(MAG_SENSOR, static_cast<sensor_settings_t>(i));
		}
	}
}

void IMUSettingsMode::uiElementStateChangeHandler(std::shared_ptr<ManagedUIElement> element)
{
	if (element->name == L"Update Button")
	{
		//clicking the update settings button while it's active will cause
		//the settings to be updated on the actual device.
		m_mode_screen_handler(ModeAction::SensorSettings, (void*)m_newSettings);
	}
	else if (element->type == UIElementType::DROP_DOWN_MENU)
	{
		//One of the drop down menus was clicked, see if an option was selected. If so, update the m_newSettings
		//array and compare it to the m_currentSettings array. If the arrays are different we can enable to 
		//update settings button at the top of the page. If they're the same, the button is disabled.
		if (element->element->getChildren()[2]->getState() & UIElementState::Invisible)
		{
			//The drop down menu scroll box becomes invisible as soon as we select a new option
			std::wstring selectedOption = ((DropDownMenu*)element->element.get())->getSelectedOption();
			selectedOption = selectedOption.substr(selectedOption.find(L"0x") + 2); //extract the hexadecimal at the end of the option

			//Update the new settings array
			int sensor = 0;
			if (element->name.substr(0, 3) == L"Gyr") sensor = 1;
			else if (element->name.substr(0, 3) == L"Mag") sensor = 2;

			uint8_t newSetting = convertStringToHex(selectedOption);

			//Before applying the new setting we need to see if it's a compound setting that will potentially alter the
			//values in other text boxes. In most cases we'll only change other options on the same sensor, however, there
			//are instances where settings on other sensors will be affected as well. For example, if both the LSM9DS1 
			//accelerometer and gyrocsope are used then their ODR and Power levels are tied together. We call a separate
			//method to help us update all settings appropriately.
			if (element->name.back() != L'u')
			{
				//We're looking at one of the standard setting dropdown menus
				int settingType = (int)(element->name.back() - L'0'); //get the setting type from the end drop down menu's name
				sensor_settings_t cat = static_cast<sensor_settings_t>(settingType);
				updateSetting(static_cast<sensor_type_t>(sensor), cat, newSetting);

				bool different = false;

				for (int j = 0; j < SENSOR_SETTINGS_LENGTH; j++)
				{
					if (m_currentSettings[j] != m_newSettings[j])
					{
						//the settings have now been changed, this means we enable the Update button
						m_uiManager.getElement<TextButton>(L"Update Button")->removeState(UIElementState::Disabled);
						different = true;
						break; //only need one byte to be different to enabled updating
					}
				}

				if (!different)
				{
					//The settings haven't been altered from their original form, so disable the update button
					m_uiManager.getElement<TextButton>(L"Update Button")->updateState(UIElementState::Disabled);
				}
			}
			else
			{
				//We've slected a different sensor from the available sensor drop down menus which means we need to load a
				//new set of drop down menus. The easiest way to do this is to just remove all current drop downs and text
				//overlays and just load everything from scratch using the current settings array.
				m_uiManager.removeElementType(UIElementType::DROP_DOWN_MENU); //TODO: Calling this causes a crash due to vector sub-script out of range
				m_uiManager.removeElementType(UIElementType::TEXT_OVERLAY);

				//By Removing all text overlays we've also removed the title and foot note so add those back. Calling the 
				//intitialzeTextOverlay() method will also put the body text back which we don't want so remove that again
				initializeTextOverlay(m_uiManager.getScreenSize());

				m_dropDownText = { {}, {}, {} };
				m_dropDownCategories = {};
				for (int i = 0; i < 3; i++)
				{
					for (int j = SENSOR_MODEL; j <= EXTRA_2; j++) m_dropDownText[i].push_back(L"");
				}

				//After removing the current drop down menus, update the m_newSettings array with the default values
				//of the newly selected sensor
				uint8_t sensor_starts[3] = { ACC_START, GYR_START, MAG_START };
				get_sensor_default_settings(sensor, newSetting, &m_newSettings[sensor_starts[sensor]]);

				//Recreate and populate all drop down menus and their titles
				dropDownsSet = false;
				getAvailableSensors();
				createDropDownMenus(m_uiManager.getScreenSize());
			}
		}
	}
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

	//Before making any updates, save the locations of some key drop down menus from the uiElement manager.
	auto accODRDropdown   = m_uiManager.getElement<DropDownMenu>(L"Acc Setting Drop Down Menu " + std::to_wstring(static_cast<int>(ODR)));
	auto accPowerDropdown = m_uiManager.getElement<DropDownMenu>(L"Acc Setting Drop Down Menu " + std::to_wstring(static_cast<int>(POWER)));
	auto gyrODRDropdown   = m_uiManager.getElement<DropDownMenu>(L"Gyr Setting Drop Down Menu " + std::to_wstring(static_cast<int>(ODR)));
	auto gyrPowerDropdown = m_uiManager.getElement<DropDownMenu>(L"Gyr Setting Drop Down Menu " + std::to_wstring(static_cast<int>(POWER)));
	auto magODRDropdown   = m_uiManager.getElement<DropDownMenu>(L"Mag Setting Drop Down Menu " + std::to_wstring(static_cast<int>(ODR)));
	auto magPowerDropdown = m_uiManager.getElement<DropDownMenu>(L"Mag Setting Drop Down Menu " + std::to_wstring(static_cast<int>(POWER)));

	uint8_t newSetting = m_newSettings[sensor_start_locations[sensor_type] + setting_type];

	if ((sensor_type == ACC_SENSOR || sensor_type == GYR_SENSOR) && m_newSettings[ACC_START] == LSM9DS1_ACC && m_newSettings[GYR_START] == LSM9DS1_GYR)
	{
		//We have both an LSM9DS1 acc and gyro, which means that the ODR and Power settings of both sensors
		//are tied together.
		std::wstring acc_odr_text = accODRDropdown->getSelectedOption();
		lsm9ds1_update_acc_gyr_setting(m_newSettings, sensor_type, setting_type, &newSetting, setting, &acc_odr_text[0]);

		//Chain drop-down text changes as necessary if the power or odr setting was altered
		if (setting_type == ODR || setting_type == POWER)
		{
			m_newSettings[ACC_START + ODR] = newSetting;
			m_newSettings[ACC_START + POWER] = newSetting;
			m_newSettings[GYR_START + ODR] = newSetting;
			m_newSettings[GYR_START + POWER] = newSetting;

			//Make necessary changes to accelerometer gyroscope drop downs
			accODRDropdown->setSelectedOption(lsm9ds1_get_settings_string(ACC_SENSOR, ODR, newSetting));
			accPowerDropdown->setSelectedOption(lsm9ds1_get_settings_string(ACC_SENSOR, POWER, newSetting));
			gyrODRDropdown->setSelectedOption(lsm9ds1_get_settings_string(GYR_SENSOR, ODR, newSetting));
			gyrPowerDropdown->setSelectedOption(lsm9ds1_get_settings_string(GYR_SENSOR, POWER, newSetting));
		}
	}
	else if (sensor_type == ACC_SENSOR && m_newSettings[ACC_START] == LSM9DS1_ACC)
	{
		//We have an LSM9DS1 acc that isn't tied to a gyro. Basically we just need to make sure that the power setting
		//and ODR match up.
		lsm9ds1_update_acc_setting(m_newSettings, setting_type, &newSetting, setting);

		//Chain drop-down text changes as necessary if the power or odr setting was altered
		if (setting_type == ODR || setting_type == POWER)
		{
			m_newSettings[ACC_START + ODR] = newSetting;
			m_newSettings[ACC_START + POWER] = newSetting;

			accODRDropdown->setSelectedOption(lsm9ds1_get_settings_string(ACC_SENSOR, ODR, newSetting));
			accPowerDropdown->setSelectedOption(lsm9ds1_get_settings_string(ACC_SENSOR, POWER, newSetting));
		}
	}
	else if (sensor_type == GYR_SENSOR && m_newSettings[GYR_START] == LSM9DS1_GYR)
	{
		//We have an LSM9DS1 acc that isn't tied to a gyro. Basically we just need to make sure that the power setting
		//and ODR match up.
		lsm9ds1_update_gyr_setting(m_newSettings, setting_type, &newSetting, setting);

		//Chain drop-down text changes as necessary if the power or odr setting was altered
		if (setting_type == ODR || setting_type == POWER)
		{
			m_newSettings[GYR_START + ODR] = newSetting;
			m_newSettings[GYR_START + POWER] = newSetting;

			gyrODRDropdown->setSelectedOption(lsm9ds1_get_settings_string(GYR_SENSOR, ODR, newSetting));
			gyrPowerDropdown->setSelectedOption(lsm9ds1_get_settings_string(GYR_SENSOR, POWER, newSetting));
		}
	}
	else if (sensor_type == MAG_SENSOR && m_newSettings[MAG_START] == LSM9DS1_MAG)
	{
		std::wstring mag_odr_text = magODRDropdown->getSelectedOption();
		lsm9ds1_update_mag_setting(m_newSettings, setting_type, &newSetting, setting, &mag_odr_text[0]);

		//Update the odr and power drop down menu text as neceessary
		magODRDropdown->setSelectedOption(lsm9ds1_get_settings_string(MAG_SENSOR, ODR, m_newSettings[MAG_START + ODR]));
		magPowerDropdown->setSelectedOption(lsm9ds1_get_settings_string(MAG_SENSOR, POWER, m_newSettings[MAG_START + POWER]));
	}
	else if ((sensor_type == ACC_SENSOR || sensor_type == MAG_SENSOR) && m_newSettings[ACC_START] == FXOS8700_ACC && m_newSettings[MAG_START] == FXOS8700_MAG)
	{
		//We have both an FXOS8700 acc and mag, which means that the ODR and Power settings of both sensors
		//are tied together.
		uint8_t original_power_settings[2] = { m_newSettings[ACC_START + POWER], m_newSettings[MAG_START + POWER] };
		fxos8700_update_acc_and_mag_setting(m_newSettings, sensor_type, setting_type, setting);

		//Chain drop-down text changes as necessary if the power or odr setting was altered
		if (setting_type == ODR || setting_type == POWER)
		{
			//If both sensors are turned on, we use a temp enum number to get the appropriate dropdown text
			int combined_odr = (m_newSettings[ACC_START + POWER] != 0xFF && m_newSettings[MAG_START + POWER] != 0xFF) ? 100 : ODR;

			//We also need to dynamically load the appropriate dropdown options depending on whether or not
			//either sensor was turned on/off. If both sensors are originally on and one turns off, we need to
			//reload the menus, or, if one sensor is off and it turns on we need to reload the options. If both
			//sensors are off and one turns on then we don't need to reload the options
			bool refresh = true, acc_flipped = false;
			if ((original_power_settings[0] == 0xFF) && (m_newSettings[ACC_START + POWER] <= 0xFF)) acc_flipped = true;
			else if ((original_power_settings[0] < 0xFF) && (m_newSettings[ACC_START + POWER] == 0xFF)) acc_flipped = true;

			if ((original_power_settings[0] == 0xFF) && (original_power_settings[1] == 0xFF)) refresh = false;
			else if ((m_newSettings[ACC_START + POWER] == 0xFF) && (m_newSettings[MAG_START + POWER] == 0xFF)) refresh = false;
			else if (!acc_flipped && (m_newSettings[MAG_START + POWER] == original_power_settings[1])) refresh = false;

			if (refresh)
			{
				//We need to reload the ODR drop down text boxes. To place them correctly we need to know the appropriate size for the new boxes
				//so we just trigger all boxes to be resized/replaced

				//Calling the handleUIElementStateChange() method will cause the m_newSettings array to be reset to it's default settings.
				//Because of this we need to make a copy of it and reapply the correct settings after the method is called.
				uint8_t settings_copy[SENSOR_SETTINGS_LENGTH];
				for (int i = 0; i < SENSOR_SETTINGS_LENGTH; i++) settings_copy[i] = m_newSettings[i];
				handleUIElementStateChange(m_accFirstDropDown - 3);
				for (int i = 0; i < SENSOR_SETTINGS_LENGTH; i++) m_newSettings[i] = settings_copy[i]; //reapply original settings
			}
			else
			{
				//Update the odr and power drop down menu text as neceessary
				accODRDropdown->setSelectedOption(fxas_fxos_get_settings_string(ACC_SENSOR, static_cast<sensor_settings_t>(combined_odr), m_newSettings[ACC_START + ODR]));
				accPowerDropdown->setSelectedOption(fxas_fxos_get_settings_string(ACC_SENSOR, POWER, m_newSettings[ACC_START + POWER]));
				magODRDropdown->setSelectedOption(fxas_fxos_get_settings_string(MAG_SENSOR, static_cast<sensor_settings_t>(combined_odr), m_newSettings[MAG_START + ODR]));
				magPowerDropdown->setSelectedOption(fxas_fxos_get_settings_string(MAG_SENSOR, POWER, m_newSettings[MAG_START + POWER]));
			}
		}
	}
	else if ((sensor_type == ACC_SENSOR && m_newSettings[ACC_START] == FXOS8700_ACC) || (sensor_type == MAG_SENSOR && m_newSettings[MAG_START] == FXOS8700_MAG))
	{
		fxos8700_update_acc_or_mag_setting(m_newSettings, sensor_type, setting_type, setting);

		//Chain drop-down text changes as necessary if the power or odr setting was altered
		if (setting_type == ODR || setting_type == POWER)
		{
			if (sensor_type == ACC_SENSOR)
			{
				accODRDropdown->setSelectedOption(fxas_fxos_get_settings_string(sensor_type, ODR, m_newSettings[sensor_start_locations[sensor_type] + ODR]));
				accPowerDropdown->setSelectedOption(fxas_fxos_get_settings_string(sensor_type, POWER, m_newSettings[sensor_start_locations[sensor_type] + POWER]));
			}
			else
			{
				magODRDropdown->setSelectedOption(fxas_fxos_get_settings_string(sensor_type, ODR, m_newSettings[sensor_start_locations[sensor_type] + ODR]));
				magPowerDropdown->setSelectedOption(fxas_fxos_get_settings_string(sensor_type, POWER, m_newSettings[sensor_start_locations[sensor_type] + POWER]));
			}
		}
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
	//In IMU Settings mode we use the update() method to ensure that drop down menus have the 
	//appropriate text and are in the correct locations on screen.
	if (!dropDownsSet)
	{
		//Before attempting to move anything, make sure that all the text boxes have been resized.
		//Due to the timing of when the drop downs get createded in the main render loop, a full
		//render cycle will pass before they get resized.
		if (m_uiManager.getElement<DropDownMenu>(L"Acc Setting Drop Down Menu 1")->getChildren()[0]->getAbsoluteSize().x == m_uiManager.getElement<DropDownMenu>(L"Acc Setting Drop Down Menu 1")->getChildren()[1]->getAbsoluteSize().x) return;

		//We place two drop downs next to each other, so we need to know the width of both 
		//before figuring out their locations. The goal is to have the arrow buttons of all
		//the drop downs line up.
		int sensor_start_locations[3] = { ACC_START, GYR_START, MAG_START };
		float location = 1.0f / 6.0f, marginWidth; //the margin size is the width of two buttons (all drop down menus will have the same size button)

		for (int sensor = 0; sensor < 3; sensor++)
		{
			std::wstring dropdown_name, title_name;
			if (sensor == 0)
			{
				dropdown_name = L"Acc Setting Drop Down Menu ";
				title_name = L"Acc Setting Drop Down Title ";
			}
			else if (sensor == 1) 
			{
				dropdown_name = L"Gyr Setting Drop Down Menu ";
			    title_name = L"Gyr Setting Drop Down Title ";
		    }
			else
			{
				dropdown_name = L"Mag Setting Drop Down Menu ";
		        title_name = L"Mag Setting Drop Down Title ";
	        }

			int index = 0, row = 0, first_index = 0; //variables for keeping track of where to put drop down menus

			for (int type = FS_RANGE; type <= EXTRA_2; type++)
			{
				auto dropDown = m_uiManager.getElement<DropDownMenu>(dropdown_name + std::to_wstring(type)); //Find the drop down based of its name

				if (dropDown->getSelectedOption() == L"") m_uiManager.removeElement<DropDownMenu>(dropdown_name + std::to_wstring(type)); //remove the drop down if its empty
				else
				{
					//TODO: Update the state of the Dropdown (and its scroll box) to include the NeedTextPixels flag.
					//We need to do this because of some legacy code which needed a full iteration of the render loop to
					//go by before we could force a resize to happen. I'll need to change the resize method of the UIElement
					//class at some point to get rid of this feature.
					dropDown->updateState(UIElementState::NeedTextPixels); //test, see if this helps invisibility issue
					dropDown->getChildren()[2]->updateState(UIElementState::NeedTextPixels);

					//Set the margin width equal to the width of the button.
					marginWidth = dropDown->getChildren()[1]->getAbsoluteSize().x * 1.0f;

					//Choose the correct setting for this drop down menu
					std::wstring initialText;
					switch (m_newSettings[sensor_start_locations[sensor] + SENSOR_MODEL])
					{
					case LSM9DS1_ACC:
						initialText = lsm9ds1_get_settings_string(static_cast<sensor_type_t>(sensor), static_cast<sensor_settings_t>(type), m_newSettings[sensor_start_locations[sensor] + type]);
						break;
					case BMI270_ACC:
						//Load filter bandwidth options for the BMI270 acc based on the filter's current operating mode
						if (sensor == ACC_SENSOR && type == LOW_PASS_FILTER && m_newSettings[sensor_start_locations[sensor] + EXTRA_FILTER] == 0x1)
						{
							initialText = bmi_bmm_get_settings_string(static_cast<sensor_type_t>(sensor), static_cast<sensor_settings_t>(100), m_newSettings[sensor_start_locations[sensor] + type]);
						}
						else initialText = bmi_bmm_get_settings_string(static_cast<sensor_type_t>(sensor), static_cast<sensor_settings_t>(type), m_newSettings[sensor_start_locations[sensor] + type]);
						break;
					case FXOS8700_ACC:
						int other_sensor = ((sensor - 2) % 4) * -1;  //if current sensor is 2 (mag), this yields 0 (acc). If current sensors is 0 (acc) this yields 2 (mag)

						//If both FXOS acc and mag are being used then we need to load their hybrid odrs
						if ((sensor == ACC_SENSOR || sensor == MAG_SENSOR) && (type == ODR) && m_newSettings[sensor_start_locations[other_sensor] + SENSOR_MODEL] == FXOS8700_ACC
							&& m_newSettings[sensor_start_locations[other_sensor] + POWER] != 0xFF)
						{
							initialText = fxas_fxos_get_settings_string(static_cast<sensor_type_t>(sensor), static_cast<sensor_settings_t>(100), m_newSettings[sensor_start_locations[sensor] + type]);
						}
						else initialText = fxas_fxos_get_settings_string(static_cast<sensor_type_t>(sensor), static_cast<sensor_settings_t>(type), m_newSettings[sensor_start_locations[sensor] + type]);
						break;
					}

					dropDown->setSelectedOption(initialText);
					m_dropDownCategories.push_back(type);

					//Then work on placing it
					if (index == 0)
					{
						first_index = type;
						index++;
					}
					else if (index == 1)
					{
						//This is the second option for this row. Since we now have
						//the lengths of both boxes we can set their locations
						auto firstDropDown = m_uiManager.getElement<DropDownMenu>(dropdown_name + std::to_wstring(first_index));

						firstDropDown->setAbsoluteLocation({ location - marginWidth - firstDropDown->getAbsoluteSize().x / 2.0f, 0.1f * (row + 3.0f) + 0.23f });
						dropDown->setAbsoluteLocation({ (1.0f / 6.0f + location) - marginWidth - dropDown->getAbsoluteSize().x / 2.0f, 0.1f * (row + 3.0f) + 0.23f });

						//after setting the location for each drop down it can be resized
						//firstDropDown->resize(m_uiManager.getScreenSize()); //since only the location has changed, not the size, this is ok
						//dropDown->resize(m_uiManager.getScreenSize()); //since only the location has changed, not the size, this is ok

						//We also place text overlays above each box letting the user know what the options represent
						TextOverlay one(dropDown->getCurrentWindowSize(), { dropDown->getAbsoluteLocation().x, dropDown->getAbsoluteLocation().y - dropDown->getAbsoluteSize().y }, { 0.25f, dropDown->getAbsoluteSize().y * 2.0f }, getSettingString(type), 0.025,
							{ UIColor::White }, { 0, (unsigned int)getSettingString(type).length() }, UITextJustification::UpperCenter);
						TextOverlay two(firstDropDown->getCurrentWindowSize(), { firstDropDown->getAbsoluteLocation().x, firstDropDown->getAbsoluteLocation().y - firstDropDown->getAbsoluteSize().y }, { 0.25f, firstDropDown->getAbsoluteSize().y * 2.0f }, getSettingString(first_index), 0.025,
							{ UIColor::White }, { 0, (unsigned int)getSettingString(first_index).length() }, UITextJustification::UpperCenter);
						
						m_uiManager.addElement<TextOverlay>(one, title_name + std::to_wstring(type));
						m_uiManager.addElement<TextOverlay>(two, title_name + std::to_wstring(first_index));

						//update the row and reset the index
						row++;
						index = 0;
					}
				}
			}

			//If there are an odd number of dropdown menus for this sensor then the final menu gets
			//placed in the cetner of the column at the bottom
			if (index == 1)
			{
				//we ended a row with only a single drop down so put it in the middle of the column
				auto dropDown = m_uiManager.getElement<DropDownMenu>(dropdown_name + std::to_wstring(first_index));
				dropDown->setAbsoluteLocation({ location, 0.10f * (row + 3.0f) + 0.23f });
				dropDown->resize(m_uiManager.getScreenSize());

				TextOverlay one(dropDown->getCurrentWindowSize(), { dropDown->getAbsoluteLocation().x, dropDown->getAbsoluteLocation().y - dropDown->getAbsoluteSize().y }, { 0.25f, dropDown->getAbsoluteSize().y * 2.0f }, getSettingString(first_index), 0.025,
					{ UIColor::White }, { 0, (unsigned int)getSettingString(first_index).length() }, UITextJustification::UpperCenter);
				m_uiManager.addElement<TextOverlay>(one, title_name + std::to_wstring(first_index));
			}

			location += 1.0f / 3.0f; //move over to the next column for the next sensor
		}

		//The last thing we do before returning is to pick the approrpiate text
		//for the currently selected sensor drop down menus. Since theres only one
		//of these boxes per column they're automatically placed in the center.
		std::wstring sensor_model = get_sensor_model_string(ACC_SENSOR, m_newSettings[ACC_START + SENSOR_MODEL]);
		m_uiManager.getElement<DropDownMenu>(L"Acc Model Drop Down Menu")->setSelectedOption(sensor_model);

		sensor_model = get_sensor_model_string(GYR_SENSOR, m_newSettings[GYR_START + SENSOR_MODEL]);
		m_uiManager.getElement<DropDownMenu>(L"Gyr Model Drop Down Menu")->setSelectedOption(sensor_model);

		sensor_model = get_sensor_model_string(MAG_SENSOR, m_newSettings[MAG_START + SENSOR_MODEL]);
		m_uiManager.getElement<DropDownMenu>(L"Mag Model Drop Down Menu")->setSelectedOption(sensor_model);

		dropDownsSet = true;
		m_uiManager.refreshGrid(); //Once all dropdowns are in the correct location, have the UIElement Manager refresh their new locations in the screen grid (needed to click on elements)
	}
}

std::wstring IMUSettingsMode::getSettingString(int setting)
{
	switch (setting)
	{
	case FS_RANGE: return L"Full Scale Range";
	case ODR: return L"Octal Data Rate";
	case POWER: return L"Power Level";
	case FILTER_SELECTION: return L"Filter Selection";
	case LOW_PASS_FILTER: return L"Low Pass Filter";
	case HIGH_PASS_FILTER: return L"High Pass Filter";
	case EXTRA_FILTER: return L"Extra Filter";
	case EXTRA_1: return L"Extra Settings";
	case EXTRA_2: return L"Extra Settings";
	default: return L"";
	}
}

void IMUSettingsMode::handlePersonalCaddieConnectionEvent(bool connectionStatus)
{
	//TODO::
	//If the connection is lost while on this page then disable all buttons and 
	//drop down menus, prompting the user to attempt to reconnect.
}