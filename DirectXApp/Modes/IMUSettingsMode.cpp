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
	for (int i = 0; i < 3; i++)
	{
		m_dropDownText.push_back({});
		for (int j = SENSOR_MODEL; j <= EXTRA_2; j++) m_dropDownText.back().push_back(L"");
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
	}
	return m_state;
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

void IMUSettingsMode::handlePersonalCaddieConnectionEvent(bool connectionStatus)
{
	//TODO::
	//If the connection is lost while on this page then disable all buttons and 
	//drop down menus, prompting the user to attempt to reconnect.
}