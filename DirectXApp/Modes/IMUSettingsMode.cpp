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

void IMUSettingsMode::getCurrentSettings(winrt::Windows::Foundation::Size windowSize, std::vector<uint8_t*> settings)
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

	//After getting the current settings we need to add some text, as well as
	//all of the drop down menus on screen
	createDropDownMenus(windowSize);
}

void IMUSettingsMode::createDropDownMenus(winrt::Windows::Foundation::Size windowSize)
{
	//The screen is split into three columns, one each for the acc, gyr and mag sensors.
	//Each of these columns features 7 drop down menus.

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

	std::wstring sensor_name = lsm9ds1_get_settings_string(ACC_SENSOR, SENSOR_MODEL, 0);
	TextOverlay acc_name(windowSize, { 0.15, 0.425 }, { 0.33, 0.1 },
		sensor_name, UIConstants::SubTitleTextPointSize * 0.8f, { UIColor::White }, { 0,  (unsigned int)sensor_name.length() }, UITextJustification::UpperCenter);

	sensor_name = lsm9ds1_get_settings_string(GYR_SENSOR, SENSOR_MODEL, 0);
	TextOverlay gyr_name(windowSize, { 0.5, 0.425 }, { 0.33, 0.1 },
		sensor_name, UIConstants::SubTitleTextPointSize * 0.8f, { UIColor::White }, { 0,  (unsigned int)sensor_name.length() }, UITextJustification::UpperCenter);

	sensor_name = lsm9ds1_get_settings_string(MAG_SENSOR, SENSOR_MODEL, 0);
	TextOverlay mag_name(windowSize, { 0.85, 0.425 }, { 0.33, 0.1 },
		sensor_name, UIConstants::SubTitleTextPointSize * 0.8f, { UIColor::White }, { 0,  (unsigned int)sensor_name.length() }, UITextJustification::UpperCenter);
	
	Line line1(windowSize, { 0.33, 0.3 }, { 0.33, 0.92 }, UIColor::White, 2.0f);
	Line line2(windowSize, { 0.67, 0.3 }, { 0.67, 0.92 }, UIColor::White, 2.0f);

	m_uiElements.push_back(std::make_shared<TextOverlay>(acc));
	m_uiElements.push_back(std::make_shared<TextOverlay>(gyr));
	m_uiElements.push_back(std::make_shared<TextOverlay>(mag));
	m_uiElements.push_back(std::make_shared<TextOverlay>(acc_name));
	m_uiElements.push_back(std::make_shared<TextOverlay>(gyr_name));
	m_uiElements.push_back(std::make_shared<TextOverlay>(mag_name));
	m_uiElements.push_back(std::make_shared<Line>(line1));
	m_uiElements.push_back(std::make_shared<Line>(line2));

	//Then, add the drop down menus. The width of the drop down menus is
	//dependent on the length of the text inside them, so final placements
	//will be calculated separately.

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

	if (m_currentSettings[ACC_START] == LSM9DS1_ACC)
	{
		//Get text using the LSM9DS1 method
		for (int i = SENSOR_MODEL; i <= EXTRA_2; i++) m_dropDownText[ACC_SENSOR][i] = lsm9ds1_get_complete_settings_string(ACC_SENSOR, static_cast<sensor_settings_t>(i));
	}

	if (m_currentSettings[GYR_START] == LSM9DS1_GYR)
	{
		//Get text using the LSM9DS1 method
		for (int i = SENSOR_MODEL; i <= EXTRA_2; i++) m_dropDownText[GYR_SENSOR][i] = lsm9ds1_get_complete_settings_string(GYR_SENSOR, static_cast<sensor_settings_t>(i));
	}

	if (m_currentSettings[MAG_START] == LSM9DS1_MAG)
	{
		//Get text using the LSM9DS1 method
		for (int i = SENSOR_MODEL; i <= EXTRA_2; i++) m_dropDownText[MAG_SENSOR][i] = lsm9ds1_get_complete_settings_string(MAG_SENSOR, static_cast<sensor_settings_t>(i));
	}
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
	else if (i >= m_accFirstDropDown)
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
			if (i >= m_gyrFirstDropDown) sensor = 1;
			if (i >= m_magFirstDropDown) sensor = 2;

			uint8_t newSetting = convertStringToHex(selectedOption);

			//Before applying the new setting we need to see if it's a compound setting that will potentially alter the
			//values in other text boxes. In most cases we'll only change other options on the same sensor, however, there
			//are instances where setting on other sensors will be affected as well. For example, if both the LSM9DS1 
			//accelerometer and gyrocsope are used then their ODR and Power levels are tied together. We call a separate
			//method to help us update all settings appropriately.
			updateSetting(static_cast<sensor_type_t>(sensor), static_cast<sensor_settings_t>(m_dropDownCategories[i - m_accFirstDropDown]), newSetting);

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
		}
	}
	return m_state;
}

void IMUSettingsMode::updateSetting(sensor_type_t sensor_type, sensor_settings_t setting_type, uint8_t setting)
{
	//This method will look at the setting and update it for the given sensor and setting type. This function
	//will also figure out if any other settings should be updated as a result of this setting being updated
	//(for example, turning off a sensor will also change its ODR to 0 Hz).
	int sensor_start_locations[3] = { ACC_START, GYR_START, MAG_START }; //useful to know where each sensor's settings begins

	if (sensor_type == ACC_SENSOR || sensor_type == GYR_SENSOR)
	{
		if ((m_currentSettings[ACC_START] == LSM9DS1_ACC) && (m_currentSettings[GYR_START] == LSM9DS1_GYR) && (setting_type == ODR || setting_type == POWER))
		{
			//We're using both an LSM9DS1 accelerometer and gyroscope. These two sensors will have a shared
			//ODR and power level. Furthermore, the odr and power level settings are also tied together.
			//This means that changing any of these four settings will case the other three to update as well.

			uint8_t newSetting = m_newSettings[sensor_start_locations[sensor_type] + setting_type]; //will be the same at all four locations

			//calcualte and save a few variables that may come in handy down below
			int accOdrIndex = 0, gyrOdrIndex, accPowerIndex = 0, gyrPowerIndex = 0; //figure out which dropdown menu represents the acc ODR
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

			uint8_t accOdr = m_newSettings[ACC_START + ODR] & 0b01110000, accPower = m_newSettings[ACC_START + ODR] & 0b01110000;
			uint8_t gyrOdr = m_newSettings[GYR_START + ODR] & 0b00000111, gyrPower = m_newSettings[GYR_START + ODR] & 0b10000111;

			//If the new setting is a power level setting
			if (setting_type == POWER)
			{
				if (sensor_type == ACC_SENSOR)
				{
					if (setting == 0)
					{
						//If we're here it means we want to turn off the accelerometer. To do this we set bits 4, 5 and 6 to 0
						newSetting &= 0b10001111;
					}
					else
					{
						//We're turning the accelerometer on. If the accelerometer was previously off than its ODR was set at 
						//0 Hz so we need to change the ODR to match that of the gyroscope. If the gyroscope is also off,
						//then we pick a default value of 50 Hz for it.
						
						if (gyrOdr != 0)
						{
							newSetting |= (gyrOdr << 4);
						}
						else
						{
							newSetting |= 0b00100000;
						}
					}
				}
				else
				{
					//we're looking at gyroscope power. The gyroscope can either be turned off, put into low power mode, or be put into 
					//normal operating mode.
					if (setting == 0)
					{
						//If we're here it means we want to turn off the gyroscope. To do this we set bits 0, 1 and 2 to 0
						newSetting &= 0b01111000;
					}
					else if (setting == 0x87)
					{
						//We're putting the gyroscope into low power, flip the MSB to 1. Only 3 ODR's are available in low power mode:
						//14.9 Hz, 59.5 Hz and 119 Hz. If we aren't using one of these ODR's already then put the
						//sensors at 59.5 Hz.
						newSetting |= 0b10000000;

						if (gyrOdr > 3 || gyrOdr == 0)
						{
							newSetting &= 0b11111000; //remove current gyr odr
							gyrOdr = 0b00000010;
							newSetting |= gyrOdr; //set current gyr odr to 59.5
						}

						//We also need to alter the acc odr to match the gyrODR (if the acc is on).
						if (accOdr > 0)
						{
							newSetting &= 0b10001111; //remove current acc odr
							newSetting |= (gyrOdr << 4); //set current acc odr to 59.5
						}
					}
					else if (setting == 0x07)
					{
						//We're putting the gyroscope into normal power mode. All odr's are available at normal power mode.
						//The only thing that we need to make sure of is that if the gyroscope was previously off we match
						//its odr to that of the acc. 
						if (gyrOdr == 0)
						{
							if (accOdr == 0)
							{
								//put the gyro at 59.5 Hz.
								newSetting = 0b00000010;
							}
							else
							{
								newSetting = accOdr | (accOdr >> 4);
							}
						}
					}
				}
			}
			else
			{
				if (sensor_type == ACC_SENSOR)
				{
					//We just changed the acc odr drop down. If the gyroscope is on then we need to change its ODR
					//to match with a few exceptions. The acc can only be set at 10 or 50 Hz if the gyroscope is off,
					//so if we change to one of these odrs then the gyroscope must be turned off. Also, if we set the 
					//acc odr over a frequency of 119 Hz then the gyroscope can't be in low power mode so we need to
					//take it out if it is. Finally, if we turn the acc off by making its odr 0 then we don't need
					//to change anything with the gyroscope.
					if (setting == 0)
					{
						//an odr reading of 0 will turn off the accelerometer. This shouldn't have any effect on 
						//the gyroscope.
						newSetting &= 0b10001111;
					}
					else if (setting == 0x10 || setting == 0x20)
					{
						//both of these options have the potential to turn off the gyro, but we can't know without looking
						//at the text in the acc odr drop down.
						std::wstring acc_odr_text = m_uiElements[m_accFirstDropDown + accOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->message;
						if (acc_odr_text == L"10 Hz 0x10" || acc_odr_text == L"50 Hz 0x20")
						{
							//we need to turn off the gyroscope. Set all settings to the new setting
							newSetting = setting;
						}
						else
						{
							//We've selected an ODR that requires the gyroscope to be on. Make the gyroscope odr
							//match the acc one, even if we need to turn on the gyro to do it.
							newSetting = setting | (setting >> 4);
						}
					}
					else
					{
						//We picked one of the standard ODR's, match the gyroscope ODR (unless the gyro is off). If the ODR is above
						//119 Hz then make sure the gyro isn't in low power mode.
						if (newSetting & 0b00000111)
						{
							//the gyro is on so match the odrs
							if ((newSetting & 0b10000000) && (setting > 0x30))
							{
								//the gyro is in low power mode and the new odr is too high, take the gyro
								//out of low power mode
								newSetting &= 0b01111111;
							}

							newSetting = (newSetting & 0b10000000) | (setting >> 4);
						}
						newSetting |= setting;
					}
				}
				else
				{
					//we just changed to gyroscope odr drop down menu. All we really need to do is see if
					//this new odr will take us out of lower power mode and make sure that the acc odr matches.
					if (setting > 3 && (newSetting & 0b10000000))
					{
						//we need to leave low power mode 
						newSetting &= 0b01111111;
					}

					//make the acc and gyr odrs match (if the acc is on). If the gyroscope is off then it will turn on here
					//and default to normal power mode.
					if (newSetting & 0b01110000) newSetting = (newSetting & 0b10000000) | (setting << 4);
					newSetting &= 0b11111000; //remove the current gyr odr
					newSetting |= setting;
				}
			}

			//After making the necessaru adjustment, update the m_newSettings array and any text boxes that need it.
			m_newSettings[ACC_START + ODR]   = newSetting;
			m_newSettings[ACC_START + POWER] = newSetting;
			m_newSettings[GYR_START + ODR]   = newSetting;
			m_newSettings[GYR_START + POWER] = newSetting;

			m_uiElements[m_accFirstDropDown + accOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->message = lsm9ds1_get_settings_string(ACC_SENSOR, ODR, newSetting);
			m_uiElements[m_accFirstDropDown + accOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->colorLocations.back() = m_uiElements[m_accFirstDropDown + accOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->message.length();

			m_uiElements[m_accFirstDropDown + accPowerIndex]->getChildren()[0]->getChildren()[1]->getText()->message = lsm9ds1_get_settings_string(ACC_SENSOR, POWER, newSetting);
			m_uiElements[m_accFirstDropDown + accPowerIndex]->getChildren()[0]->getChildren()[1]->getText()->colorLocations.back() = m_uiElements[m_accFirstDropDown + accPowerIndex]->getChildren()[0]->getChildren()[1]->getText()->message.length();

			m_uiElements[m_accFirstDropDown + gyrOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->message = lsm9ds1_get_settings_string(GYR_SENSOR, ODR, newSetting);
			m_uiElements[m_accFirstDropDown + gyrOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->colorLocations.back() = m_uiElements[m_accFirstDropDown + gyrOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->message.length();

			m_uiElements[m_accFirstDropDown + gyrPowerIndex]->getChildren()[0]->getChildren()[1]->getText()->message = lsm9ds1_get_settings_string(GYR_SENSOR, POWER, newSetting);
			m_uiElements[m_accFirstDropDown + gyrPowerIndex]->getChildren()[0]->getChildren()[1]->getText()->colorLocations.back() = m_uiElements[m_accFirstDropDown + gyrPowerIndex]->getChildren()[0]->getChildren()[1]->getText()->message.length();
		}
		else
		{
			//We've selected a setting that doesn't affect any other settings. We just need to update the
			//m_new settings array accordingly.
			m_newSettings[sensor_start_locations[sensor_type] + setting_type] = setting;
        }
	}
	else
	{
		//we're looking at the magnetometer. The ODR and power level are tied to each other so changing one will change the other.
		//The full scale range doesn't have any carry over effects though.
		int magOdrIndex = 0, magPowerIndex = 0; //find the indices for the mag odr and power drop down menus
		for (int i = m_magFirstDropDown - m_accFirstDropDown; i < m_dropDownCategories.size(); i++)
		{
			if (m_dropDownCategories[i] == ODR) magOdrIndex = i;
			else if (m_dropDownCategories[i] == POWER) magPowerIndex = i;
		}

		if (setting_type == ODR)
		{
			//Update the mag odr setting
			m_newSettings[sensor_start_locations[sensor_type] + setting_type] &= 0xF0; //remove the current odr setting (least significant byte)
			m_newSettings[sensor_start_locations[sensor_type] + setting_type] |= setting; //apply the new odr

			//If one of the 0x08 odr options is selected it will cause the power level to change,
			//otherwise the power level remains the same.
			if (setting == 0x08)
			{
				//there are four options here, each one will causes us to go to a different power level.
				//We look at the actual text in the drop down to figure out which power level we need.
				m_newSettings[sensor_start_locations[sensor_type] + POWER] = 0x08; //erase the current power mode and set the second byte to 8

				std::wstring mag_odr_text = m_uiElements[m_accFirstDropDown + magOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->message;
				if (mag_odr_text == L"155 Hz 0x08") m_newSettings[sensor_start_locations[sensor_type] + POWER] |= 0x30; //put the sensor into ultra high power mode
				else if (mag_odr_text == L"300 Hz 0x08") m_newSettings[sensor_start_locations[sensor_type] + POWER] |= 0x20; //put the sensor into high power mode
				else if (mag_odr_text == L"560 Hz 0x08") m_newSettings[sensor_start_locations[sensor_type] + POWER] |= 0x10; //put the sensor into medium power mode
				//if none of the three if statements above gets triggered then the chip will be put into low power mode

				m_newSettings[sensor_start_locations[sensor_type] + setting_type] = m_newSettings[sensor_start_locations[sensor_type] + POWER]; //update the odr setting to reflect the power setting
			}
			else if (setting == 0xC0)
			{
				//putting the odr at 0 hz will turn off the magnetometer
				m_newSettings[sensor_start_locations[sensor_type] + ODR] = setting;
				m_newSettings[sensor_start_locations[sensor_type] + POWER] = setting;
			}
			else
			{
				//a normal odr was chosen. If the magnetometer is off turn it on by placing it into
				//low power mode
				if (m_newSettings[sensor_start_locations[sensor_type] + POWER] == 0xC0)
				{
					m_newSettings[sensor_start_locations[sensor_type] + ODR] &= 0x0F;
				}
				m_newSettings[sensor_start_locations[sensor_type] + POWER] = m_newSettings[sensor_start_locations[sensor_type] + ODR];;
			}
		}
		else if (setting_type == POWER)
		{
			//Changing the power will only effect the ODR if we're in high odr mode. This should happen automatically
			//though, just update the settings.
			m_newSettings[sensor_start_locations[sensor_type] + POWER] &= 0x0F; //erase the current power setting
			m_newSettings[sensor_start_locations[sensor_type] + POWER] |= setting; //erase the current power setting

			if (setting == 0xC0) m_newSettings[sensor_start_locations[sensor_type] + POWER] = setting; //turning the power off changes the whole setting
			m_newSettings[sensor_start_locations[sensor_type] + ODR] = m_newSettings[sensor_start_locations[sensor_type] + POWER];
		}
		else m_newSettings[sensor_start_locations[sensor_type] + setting_type] = setting; //update the full scale range

		//Update the odr and power drop down menu text as neceessary
		m_uiElements[m_accFirstDropDown + magOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->message = lsm9ds1_get_settings_string(MAG_SENSOR, ODR, m_newSettings[MAG_START + ODR]);
		m_uiElements[m_accFirstDropDown + magOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->colorLocations.back() = m_uiElements[m_accFirstDropDown + magOdrIndex]->getChildren()[0]->getChildren()[1]->getText()->message.length();

		m_uiElements[m_accFirstDropDown + magPowerIndex]->getChildren()[0]->getChildren()[1]->getText()->message = lsm9ds1_get_settings_string(MAG_SENSOR, POWER, m_newSettings[MAG_START + POWER]);
		m_uiElements[m_accFirstDropDown + magPowerIndex]->getChildren()[0]->getChildren()[1]->getText()->colorLocations.back() = m_uiElements[m_accFirstDropDown + magPowerIndex]->getChildren()[0]->getChildren()[1]->getText()->message.length();
    }

	//TODO: I should break this method up into smaller methods to make things easier to read, and some code reusable
	//TODO: Need to tie together some other dependent settings, and make non-dependent settings actually change the settings array
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
				marginWidth = m_uiElements[j]->getChildren()[1]->getAbsoluteSize().x * 2.0f;
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
				std::wstring initialText = lsm9ds1_get_settings_string(static_cast<sensor_type_t>(sensor), static_cast<sensor_settings_t>(type), m_currentSettings[sensor_start_locations[sensor] + type]);
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
					m_uiElements[indices.first]->setAbsoluteLocation({ location - marginWidth - m_uiElements[indices.first]->getAbsoluteSize().x / 2.0f, 0.1f * (row + 3.0f) + 0.2f });
					m_uiElements[indices.second]->setAbsoluteLocation({ (1.0f / 6.0f + location) - marginWidth - m_uiElements[indices.second]->getAbsoluteSize().x / 2.0f, 0.1f * (row + 3.0f) + 0.2f });

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
					m_uiElements[indices.first]->setAbsoluteLocation({ location, 0.10f * (row + 3.0f) + 0.2f });
					m_uiElements[indices.first]->resize(m_uiElements[indices.first]->getCurrentWindowSize()); //since only the location has changed, not the size, this is ok
					index = 0;
				}

				sensor++;
				row = 0;
				type = 0;
				location += 1.0f / 3.0f;
			}
		}

		int x = 10;

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
			m_uiElements.insert(m_uiElements.begin() + m_accFirstDropDown, std::make_shared<TextOverlay>(dropDownTitle));

			m_accFirstDropDown++;
			m_gyrFirstDropDown++;
			m_magFirstDropDown++;
		}

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