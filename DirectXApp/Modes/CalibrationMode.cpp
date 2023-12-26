#include "pch.h"
#include "CalibrationMode.h"
#include "Graphics/Objects/3D/Elements/Face.h"
#include "Math/quaternion_functions.h"

CalibrationMode::CalibrationMode()
{
	//set a light gray background color for the mode
	m_backgroundColor = UIColor::DarkGray;
}

uint32_t CalibrationMode::initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState)
{
	//Take the current screen size and pass it to the UIElementManager, this is so that the manager knows
	//how large to make each element.
	m_uiManager.updateScreenSize(windowSize);

	//Create UI Elements on the page
	std::wstring introMessage = L""; //This get's filled out when the loadMainPage() method is called
	std::wstring accButtonText = L"Accelerometer Calibration";
	std::wstring gyrButtonText = L"Gyroscope Calibration";
	std::wstring magButtonText = L"Magnetometer Calibration";
	std::wstring continueButtonText = L"Continue";
	std::wstring noButtonText = L"No";
	std::wstring toggleButtonText = L"Data Type: Standard";
	std::wstring axesButtonText = L"Test Type: Data Calibration";

	TextButton accButton(windowSize, { 0.16, 0.85 }, { 0.14, 0.1 }, accButtonText);
	TextButton gyrButton(windowSize, { 0.5, 0.85 }, { 0.14, 0.1 }, gyrButtonText);
	TextButton magButton(windowSize, { 0.83, 0.85 }, { 0.14, 0.1 }, magButtonText);
	TextButton continueButton(windowSize, { 0.16, 0.85 }, { 0.14, 0.1 }, continueButtonText);
	TextButton noButton(windowSize, { 0.32, 0.85 }, { 0.14, 0.1 }, noButtonText);
	TextButton toggleButton(windowSize, { 0.83, 0.65 }, { 0.14, 0.1 }, toggleButtonText);
	TextButton axesButton(windowSize, { 0.5, 0.65 }, { 0.14, 0.1 }, axesButtonText);
	continueButton.updateState(UIElementState::Invisible); //this button is invisible to start off
	noButton.updateState(UIElementState::Invisible); //this button is invisible to start off

	//The body and sub-title text overlays will change depending on the current mode state so they
	//aren't declared in the initializeTextOverlay() method
	TextOverlay body(windowSize, { UIConstants::BodyTextLocationX, UIConstants::BodyTextLocationY }, { UIConstants::BodyTextSizeX, UIConstants::BodyTextSizeY },
		introMessage, 0.65f * UIConstants::BodyTextPointSize, { UIColor::White }, { 0,  (unsigned int)introMessage.length() }, UITextJustification::UpperLeft);
	TextOverlay subTitle(windowSize, { UIConstants::SubTitleTextLocationX, UIConstants::SubTitleTextLocationY - 0.025f }, { UIConstants::SubTitleTextSizeX, UIConstants::SubTitleTextSizeY },
		L"Fill", 1.05f * UIConstants::SubTitleTextPointSize, {UIColor::White}, {0, 4}, UITextJustification::UpperCenter);
	subTitle.setState(subTitle.getState() | UIElementState::Invisible); ///the sub-title starts off invisible

	//Create a few graph ui elements, they will stay invisible until after individual calibrations are complete
	Graph accGraph(windowSize, { 0.7, 0.65 }, { 0.5, 0.5 });
	Graph magGraph1(windowSize, { 0.55, 0.8 }, { 0.25, 0.25 }, false, UIColor::White, UIColor::Black, true);
	Graph magGraph2(windowSize, { 0.85, 0.5 }, { 0.25, 0.25 }, false, UIColor::White, UIColor::Black, true);
	Graph magGraph3(windowSize, { 0.85, 0.8 }, { 0.25, 0.25 }, false, UIColor::White, UIColor::Black, true);
	accGraph.setState(accGraph.getState() | UIElementState::Invisible);
	magGraph1.setState(magGraph1.getState() | UIElementState::Invisible);
	magGraph2.setState(magGraph2.getState() | UIElementState::Invisible);
	magGraph3.setState(magGraph3.getState() | UIElementState::Invisible);
	
	m_uiManager.addElement<TextButton>(accButton, L"Acc Button");
	m_uiManager.addElement<TextButton>(gyrButton, L"Gyr Button");
	m_uiManager.addElement<TextButton>(magButton, L"Mag Button");
	m_uiManager.addElement<TextButton>(continueButton, L"Continue Button");
	m_uiManager.addElement<TextOverlay>(body, L"Body Text");
	m_uiManager.addElement<TextOverlay>(subTitle, L"Subtitle Text");
	m_uiManager.addElement<Graph>(accGraph, L"Acc Graph");
	m_uiManager.addElement<TextButton>(noButton, L"No Button");
	m_uiManager.addElement<TextButton>(toggleButton, L"Toggle Button");
	m_uiManager.addElement<Graph>(magGraph1, L"Mag1 Graph");
	m_uiManager.addElement<Graph>(magGraph2, L"Mag2 Graph");
	m_uiManager.addElement<Graph>(magGraph3, L"Mag3 Graph");
	m_uiManager.addElement<TextButton>(axesButton, L"Axes Button");

	m_state = initialState;
	m_useCalibratedData = false;
	m_axisCalibration = false;

	//Initialize any unchanging text
	initializeTextOverlay(windowSize);

	//This mode has a lot of UI Elements so the following method is used to 
	//quickly load the landing page
	loadModeMainPage();

	//initialize a model to render on screen
	initializeModel();

	//The NeedMaterial modeState lets the mode screen know that it needs to pass
	//a list of materials to this mode that it can use to initialize 3d objects
	return (ModeState::CanTransfer | ModeState::NeedMaterial);
}

void CalibrationMode::uninitializeMode()
{
	//Delete everything in the UIElement Manager
	m_uiManager.removeAllElements();

	//Clear out all data sets
	initializeCalibrationVariables();

	//Clear out all volume elements
	m_volumeElements.clear();
	m_materialTypes.clear();
}

void CalibrationMode::initializeTextOverlay(winrt::Windows::Foundation::Size windowSize)
{
	//Title information
	std::wstring title_message = L"Calibration Mode";
	TextOverlay title(windowSize, { UIConstants::TitleTextLocationX, UIConstants::TitleTextLocationY }, { UIConstants::TitleTextSizeX, UIConstants::TitleTextSizeY },
		title_message, UIConstants::TitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)title_message.length() }, UITextJustification::CenterCenter);
	//m_uiElements.push_back(std::make_shared<TextOverlay>(title));
	m_uiManager.addElement<TextOverlay>(title, L"Title Text");

	//Footnote information
	std::wstring footnote_message = L"Press Esc. to return to settings menu.";
	TextOverlay footnote(windowSize, { UIConstants::FootNoteTextLocationX, UIConstants::FootNoteTextLocationY }, { UIConstants::FootNoteTextSizeX, UIConstants::FootNoteTextSizeY },
		footnote_message, UIConstants::FootNoteTextPointSize, { UIColor::White }, { 0,  (unsigned int)footnote_message.length() }, UITextJustification::LowerRight);
	//m_uiElements.push_back(std::make_shared<TextOverlay>(footnote));
	m_uiManager.addElement<TextOverlay>(footnote, L"Footnote Text");
}

void CalibrationMode::initializeCalibrationVariables()
{
	m_currentStage = 0;
	m_stageSet = false;
	unlimited_record = false;
	m_timeStamp = 0.0f;
	avg_count = 0;
	accept_cal = true; //by default we choose to accept the calibration results
	data_time_stamps.clear();
	m_currentSensor = -1; //default to a non-sensor number
	m_currentlyRecording = false;
	
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 6; j++) acc_cal[i][j] = 0.0f;

		acc_off[i] = 0.0f;
		acc_gain_x[i] = 0.0f;
		acc_gain_y[i] = 0.0f;
		acc_gain_z[i] = 0.0f;

		gyr_off[i] = 0.0f;
		gyr_gain_x[i] = 0.0f;
		gyr_gain_y[i] = 0.0f;
		gyr_gain_z[i] = 0.0f;

		mag_off[i] = 0.0f;
		mag_gain_x[i] = 0.0f;
		mag_gain_y[i] = 0.0f;
		mag_gain_z[i] = 0.0f;

		acc_axis_swap[i] = 0;
		acc_axis_polarity[i] = 0;

		gyr_axis_swap[i] = 0;
		gyr_axis_polarity[i] = 0;

		mag_axis_swap[i] = -1;
		mag_axis_polarity[i] = 1;
	}

	m_graphDataX.clear();
	m_graphDataY.clear();
	m_graphDataZ.clear();
}

void CalibrationMode::prepareRecording()
{
	//We end up doing the same actions a lot right before we begin recording data,
	//so it's more efficient to just create a method to carry the actions out
	if (!m_stageSet)
	{
		auto mode = PersonalCaddiePowerMode::SENSOR_ACTIVE_MODE;
		m_mode_screen_handler(ModeAction::PersonalCaddieChangeMode, (void*)&mode);//request the Personal Caddie to be placed into active mode to start recording data

		m_needsCamera = false; //it's easier to stop animations while recording data
		m_stageSet = true;
	}
}

void CalibrationMode::record()
{
	if (m_currentlyRecording)
	{
		std::wstring message = L"Recording Data, hold sensor steady for " + std::to_wstring((float)(data_timer_duration - data_timer_elapsed) / 1000.0f) + L" more seconds.";
		m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(message);
	}
}

void CalibrationMode::stopRecording()
{
	if (m_currentlyRecording)
	{
		m_currentlyRecording = false;
		auto mode = PersonalCaddiePowerMode::SENSOR_IDLE_MODE;
		m_mode_screen_handler(ModeAction::PersonalCaddieChangeMode, (void*)&mode);//request the Personal Caddie to be placed into active mode to start recording data
	}
}

void CalibrationMode::uiElementStateChangeHandler(std::shared_ptr<ManagedUIElement> element)
{
	std::wstring elementName = element->name;

	if (elementName == L"Acc Button" || elementName == L"Gyr Button" || elementName == L"Mag Button")
	{
		//one of the calibration buttons was clicked, update the current mode state
		//and make the buttons invisible
		m_uiManager.getElement<TextButton>(L"Acc Button")->updateState(UIElementState::Invisible);
		m_uiManager.getElement<TextButton>(L"Gyr Button")->updateState(UIElementState::Invisible);
		m_uiManager.getElement<TextButton>(L"Mag Button")->updateState(UIElementState::Invisible);
		m_uiManager.getElement<TextButton>(L"Toggle Button")->updateState(UIElementState::Invisible);
		m_uiManager.getElement<TextButton>(L"Axes Button")->updateState(UIElementState::Invisible);

		m_uiManager.getElement<TextButton>(L"Continue Button")->removeState(UIElementState::Invisible); //make the continue button visible

		//Change the footenote text
		m_uiManager.getElement<TextOverlay>(L"Footnote Text")->updateText(L"Press Esc. to quit calibration");

		if (elementName == L"Acc Button") m_currentSensor = ACC_SENSOR;
		else if (elementName == L"Gyr Button") m_currentSensor = GYR_SENSOR;
		else if (elementName == L"Mag Button") m_currentSensor = MAG_SENSOR;

		//m_state |= ModeState::Active;

		//Turn on BLE notifications, this will in turn put the Personal Caddie into senor idle mode
		auto mode = PersonalCaddiePowerMode::SENSOR_IDLE_MODE;
		m_mode_screen_handler(ModeAction::PersonalCaddieChangeMode, (void*)&mode);//request the Personal Caddie to be placed into active mode to start recording data
	}
	else if (elementName == L"Continue Button")
	{
		//The continue button was clicked, this has the effect of advancing the current stage by 1
		advanceToNextStage();
	}
	else if (elementName == L"No Button")
	{
		//The no button was clicked, set the accept_cal bool to false and advance to the next stage
		accept_cal = false;
		advanceToNextStage();
	}
	else if (elementName == L"Toggle Button")
	{
		//Toggles whether or not we use raw or calibrated data during the calibration. The point of using
		//already calibrated data is to benchmark how good the current calibration numbers are.
		m_useCalibratedData = !m_useCalibratedData;
		if (!m_useCalibratedData) m_uiManager.getElement<TextButton>(L"Toggle Button")->updateText(L"Data Type: Standard");
		else m_uiManager.getElement<TextButton>(L"Toggle Button")->updateText(L"Data Type: Pre-Calibrated");
	}
	else if (elementName == L"Axes Button")
	{
		//Clicking the axes button changes each test from a standard calibration to a calibration of the selected sensor's axes
		m_axisCalibration = !m_axisCalibration;
		if (!m_axisCalibration) m_uiManager.getElement<TextButton>(L"Axes Button")->updateText(L"Test Type: Data Calibration");
		else m_uiManager.getElement<TextButton>(L"Axes Button")->updateText(L"Test Type: Axis Calibration");
	}
	else
	{
		OutputDebugString(L"Couldn't find the proper UI Element.\n");
	}
}

void CalibrationMode::pc_ModeChange(PersonalCaddiePowerMode newMode)
{
	//Since we need to pass through sensor idle stage to move from active to connected modes (this is only until the 
	//next firmware update), the timing of when to start recording, when to move onto the next calibration stage, etc.
	//gets a little bit tricky.

	//In plain English the way it works is:
	//1. When a calibration is first selected we place the sensor into idle mode, this enables the TWI buses for communication with the sensors
	//2. When we're ready to record data the prepareRecording() method gets called, requesting the sensor be placed into active mode
	//3. When active mode is engaged on the device this method is again called. The else if statement below will activate, starting the data timer and putting us into record mode.
	//4. While the record flag is active data will be recorded for a set amount of time. When this timer is done the stopRecording() method above is called, putting the sensor back into idle mode
	//5. When sensor idle mode is again activated, since the recording flag is active the embedded if statement will be true this time which causes the recording flag to go away and we proceed to the next stage of the cal..

	if (newMode == PersonalCaddiePowerMode::SENSOR_IDLE_MODE)
	{
		if (m_currentStage > 1) advanceToNextStage(); //at stage 1 is when we move into idle mode from connected mode, which requires no action
	}
	else if (newMode == PersonalCaddiePowerMode::SENSOR_ACTIVE_MODE)
	{
		m_currentlyRecording = true;
		data_timer = std::chrono::steady_clock::now();
	}
}

void CalibrationMode::ble_NotificationsChange(int state)
{
	//If notifications have been enabled we put the sensor into idle mode, if notifications have been
	//disabled then we put the sensor into connected mode.
	auto mode = PersonalCaddiePowerMode::CONNECTED_MODE;
	if (state) mode = PersonalCaddiePowerMode::SENSOR_IDLE_MODE;
	
	m_mode_screen_handler(ModeAction::PersonalCaddieChangeMode, (void*)&mode);//request the Personal Caddie to be placed into active mode to start recording data
}

void CalibrationMode::advanceToNextStage()
{
	//We call this method to move to the next stage of the current calibration
	m_currentStage++;
	m_stageSet = false;
}

void CalibrationMode::update()
{
	//If we're currently recording data, check to see if the timer has expired. If not,
	//then simply display the amount of time that the timer has left on screen.
	if (m_currentlyRecording)
	{
		if (!unlimited_record)
		{
			//We're actively recording data from the sensor, check to see if the data timer has expired,
			//if not then no need to do anything. If the timer has expired, we leave the recording state
			//and advnace to the next stage of the calibration.
			data_timer_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - data_timer).count();
			if (data_timer_elapsed >= data_timer_duration) stopRecording();
			else record();
		}
	}
	
	//Take action based on the current calibration being run and the current stage variable
	if (m_currentSensor == ACC_SENSOR)
	{
		if (!m_axisCalibration) accelerometerCalibration();
		else accelerometerAxisCalibration();
	}
	else if (m_currentSensor == GYR_SENSOR)
	{
		if (!m_axisCalibration) gyroscopeCalibration();
		else gyroscopeAxisCalibration();
	}
	else if (m_currentSensor == MAG_SENSOR)
	{
		if (!m_axisCalibration) magnetometerCalibration();
		else magnetometerAxisCalibration();
	}

	//If the image of the sensor is currently being rendered then translate and rotate
	//it accordingly.
	if (m_needsCamera) for (int i = 0; i < m_volumeElements.size(); i++) ((Face*)m_volumeElements[i].get())->translateAndRotateFace({ 0.0f, -0.25f, 1.0f }, m_renderQuaternion);
}

void CalibrationMode::handleKeyPress(winrt::Windows::System::VirtualKey pressedKey)
{
	//The only key that does anything in this mode is the escape key. If it's pressed while a 
	//test is currently ongoing then it will bring us back to the main splash page for this mode
	//and put the Personal Caddie into connected mode. If we're already on this main splash page then
	//we'll be sent back to the settings menu
	
	if (pressedKey == winrt::Windows::System::VirtualKey::Escape)
	{
		if (m_currentSensor == -1)
		{
			ModeType newMode = ModeType::SETTINGS_MENU;
			m_mode_screen_handler(ModeAction::ChangeMode, (void*)&newMode);
		}
		else
		{
			PersonalCaddiePowerMode powerMode = PersonalCaddiePowerMode::CONNECTED_MODE;
			m_mode_screen_handler(ModeAction::PersonalCaddieChangeMode, (void*)&powerMode);//request the Personal Caddie to be placed into active mode to start recording data
			initializeCalibrationVariables(); //reset all calibration variables back to their initial states
			loadModeMainPage();
		}
	}
}

void CalibrationMode::addData(std::vector<std::vector<std::vector<float> > > const& sensorData, float sensorODR, float timeStamp, int totalSamples)
{
	//This method gets called asynchronously whenever new data is ready. We simply iterate over the appropriate
	//data (given the current calibration we're doing) and add it to the internal data vector.
	m_sensorODR = sensorODR; //this value won't change, I just couldn't think of anywhere better to set it for right now

	if (m_currentlyRecording) //only add data if we're actively recording
	{
		float timeIncrement = 1.0f / sensorODR;
		int current_stage = m_currentStage / 2 - 1;

		if (m_graphDataX.size() > 0 && (m_currentSensor == ACC_SENSOR)) timeStamp += m_graphDataX.back().x; //TODO: This is done for the acc test only, but is currently in the wrong location. Need to call when moving from one part of the tumble calibration to the next, not when new data comes in

		int calibrationType;
		if (m_currentSensor == ACC_SENSOR)
		{
			if (m_useCalibratedData) calibrationType = 0;
			else calibrationType = raw_acceleration;
		}
		else if (m_currentSensor == GYR_SENSOR)
		{
			if (m_useCalibratedData) calibrationType = 1;
		    else calibrationType = raw_rotation;
		}
		else if (m_currentSensor == MAG_SENSOR)
		{
			if (m_useCalibratedData) calibrationType = 2;
		    else calibrationType = raw_magnetic;
		}

		for (int i = 0; i < totalSamples; i++)
		{
			//If we're in axes calibration mode then we need to revert the sensor data back into 
			//it's raw form. This is done by simply reversing the axes calibration, i.e.
			//actual_reading[x] = axes_calibrated_reading[swap[x]] * polarity[x]. Otherwise just
			//take the data as is.
			if (m_axisCalibration)
			{
				m_graphDataX.push_back({ timeStamp + i * timeIncrement, sensorData[calibrationType][m_axis_swap[0]][i] * m_axis_polarity[0] });
				m_graphDataY.push_back({ timeStamp + i * timeIncrement, sensorData[calibrationType][m_axis_swap[1]][i] * m_axis_polarity[1] });
				m_graphDataZ.push_back({ timeStamp + i * timeIncrement, sensorData[calibrationType][m_axis_swap[2]][i] * m_axis_polarity[2] });
			}
			else
			{
				m_graphDataX.push_back({ timeStamp + i * timeIncrement, sensorData[calibrationType][0][i] });
				m_graphDataY.push_back({ timeStamp + i * timeIncrement, sensorData[calibrationType][1][i] });
				m_graphDataZ.push_back({ timeStamp + i * timeIncrement, sensorData[calibrationType][2][i] });
			}

			//TODO: The below block just adds all the data points in the array. There's no reason it needs to happen here,
			//this should probably be moved into one of the accelerometer specific methods.
			if (calibrationType == raw_acceleration || calibrationType == 0)
			{
				//add x, y and z data to accerlometer matrix
				acc_cal[0][current_stage] += m_graphDataX.back().y;
				acc_cal[1][current_stage] += m_graphDataY.back().y;
				acc_cal[2][current_stage] += m_graphDataZ.back().y;
				avg_count++;
			}
		}
	}
}

void CalibrationMode::getSensorAxisCalibrationNumbers(sensor_type_t sensor, std::pair<const int*, const int*> cal_numbers)
{
	int* axis_numbers, *polarity_numbers;
	switch (sensor)
	{
	case ACC_SENSOR:
	default:
	{
		axis_numbers = acc_axis_swap;
		polarity_numbers = acc_axis_polarity;
		break;
	}
	case GYR_SENSOR:
	{
		axis_numbers = gyr_axis_swap;
		polarity_numbers = gyr_axis_polarity;
		break;
	}
	case MAG_SENSOR:
	{
		axis_numbers = mag_axis_swap;
		polarity_numbers = mag_axis_polarity;
		break;
	}
	}

	axis_numbers[0] = cal_numbers.first[0];
	axis_numbers[1] = cal_numbers.first[1];
	axis_numbers[2] = cal_numbers.first[2];

	polarity_numbers[0] = cal_numbers.second[0];
	polarity_numbers[1] = cal_numbers.second[1];
	polarity_numbers[2] = cal_numbers.second[2];
}

void CalibrationMode::accAverageData()
{
	//The accelerometer data (whether we're doing the axis alignment calibration
	//or the accelerometer calibration) needs to be average after it's collected
	//so do so here. This method always gets called in the next stage after recording
	//is complete so we need to subtract 1 from the current stage to put the data in 
	//the correct vector
	int current_acc_stage = (m_currentStage - 1) / 2 - 1; //Average the acc data obtained in the last stage
	acc_cal[0][current_acc_stage] /= avg_count;
	acc_cal[1][current_acc_stage] /= avg_count;
	acc_cal[2][current_acc_stage] /= avg_count;
	avg_count = 0;
}

void CalibrationMode::accAxisCalculate(int axis)
{
	//Average the data first
	accAverageData();
	
	//Look at the average data for each axis. See which of the axis has the largest value (if the sensor was held correctly
	//than one axis should have an average value of +/-9.8 while the others are close to 0. Convert all values to be positive,
	//and then take the max
	int invert[3] = { 1, 1, 1 };
	int current_stage = (m_currentStage - 1) / 2 - 1; //this method gets called every other stage, starting at 2, which needs to map to the X (0) axis
	for (int i = 0; i < 3; i++)
	{
		if (acc_cal[i][current_stage] < 0)
		{
			acc_cal[i][current_stage] *= -1;
			invert[i] = -1;
		}
	}

	int largest_axis = 0, largest_value = acc_cal[0][current_stage];
	for (int i = 1; i <= 2; i++)
	{
		if (acc_cal[i][current_stage] > largest_value)
		{
			largest_value = acc_cal[i][current_stage];
			largest_axis = i;
		}
	}

	test_acc_axis_swap[axis] = largest_axis;
	test_acc_axis_polarity[axis] = invert[largest_axis];
}

void CalibrationMode::gyrAxisCalculate(int axis)
{
	//Integrate the gyroscope data over all three axes to see the total degree of movement.
	//We then calcualte the proper axis swap and polarity as in the accAxisCalculate() method
	float total_movement[3] = { 0, 0, 0 };
	for (int i = 1; i < m_graphDataX.size(); i++)
	{
		total_movement[0] += integrateData(m_graphDataX[i].y, m_graphDataX[i - 1].y, m_graphDataX[i].x - m_graphDataX[i - 1].x);
		total_movement[1] += integrateData(m_graphDataY[i].y, m_graphDataY[i - 1].y, m_graphDataY[i].x - m_graphDataY[i - 1].x);
		total_movement[2] += integrateData(m_graphDataZ[i].y, m_graphDataZ[i - 1].y, m_graphDataZ[i].x - m_graphDataZ[i - 1].x);
	}

	int invert[3] = { 1, 1, 1 };
	for (int i = 0; i < 3; i++)
	{
		if (total_movement[i] < 0)
		{
			total_movement[i] *= -1;
			invert[i] = -1;
		}
	}

	int largest_axis = 0, largest_value = total_movement[0];
	for (int i = 1; i <= 2; i++)
	{
		if (total_movement[i] > largest_value)
		{
			largest_value = total_movement[i];
			largest_axis = i;
		}
	}

	test_gyr_axis_swap[axis] = largest_axis;
	test_gyr_axis_polarity[axis] = invert[largest_axis];
}

void CalibrationMode::magAxisCalculate(int axis)
{
	if (axis == 1 || axis == 2)
	{
		//We take the y and z axes to be the ones with the least amount of fluctuation in the data at the given time,
		//that is, that data set with the lowest standard deviation.
		float data_average[3] = { 0, 0, 0 }, std_deviation[3] = { 0, 0, 0 };
		for (int i = 0; i < m_graphDataX.size(); i++)
		{
			data_average[0] += m_graphDataX[i].y;
			data_average[1] += m_graphDataY[i].y;
			data_average[2] += m_graphDataZ[i].y;
		}

		data_average[0] /= m_graphDataX.size();
		data_average[1] /= m_graphDataY.size();
		data_average[2] /= m_graphDataZ.size();

		for (int i = 0; i < m_graphDataX.size(); i++)
		{
			std_deviation[0] += ((m_graphDataX[i].y - data_average[0]) * (m_graphDataX[i].y - data_average[0]));
			std_deviation[1] += ((m_graphDataY[i].y - data_average[1]) * (m_graphDataY[i].y - data_average[1]));
			std_deviation[2] += ((m_graphDataZ[i].y - data_average[2]) * (m_graphDataZ[i].y - data_average[2]));
		}

		std_deviation[0] = sqrt(std_deviation[0] / m_graphDataX.size());
		std_deviation[1] = sqrt(std_deviation[1] / m_graphDataY.size());
		std_deviation[2] = sqrt(std_deviation[2] / m_graphDataZ.size());

		//From this point we carry out the same algorithm as in accAxisCalculate, however, we choose the 
		//smallest value instead of the largest value
		int smallest_deviation_axis = 0, smallest_deviation = std_deviation[0];
		for (int i = 1; i <= 2; i++)
		{
			if (std_deviation[i] < smallest_deviation)
			{
				smallest_deviation = std_deviation[i];
				smallest_deviation_axis = i;
			}
		}

		test_mag_axis_swap[axis] = smallest_deviation_axis;
	}
	else
	{
		//The x-axis will be the only element in the mag_axis_swap array that still has a
		//value of -1. Identify that index, and also identify which number isn't in the 
		//array yet, either 0, 1 or 2. Place the missing number in the last unfilled index.

		int used = 0b000, missing_index = 0;
		for (int i = 0; i < 3; i++)
		{
			if (test_mag_axis_swap[i] == 0) used |= 0b1;
			else if (test_mag_axis_swap[i] == 1) used |= 0b10;
			else if (test_mag_axis_swap[i] == 2) used |= 0b100;
			else missing_index = i;
		}

		if (!(used & 0b1)) test_mag_axis_swap[missing_index] = 0;
		else if (!(used & 0b10)) test_mag_axis_swap[missing_index] = 1;
		else if (!(used & 0b100)) test_mag_axis_swap[missing_index] = 2;
	}
}

std::wstring CalibrationMode::axisResultString(int* axis_swap, int* axis_polarity)
{
	std::wstring result = L"";

	for (int i = 0; i < 3; i++)
	{
		if (*(axis_polarity + i) == -1) result += L"-";

		switch (*(axis_swap + i))
		{
		case 0:
			result += L"X ";
			break;
		case 1:
			result += L"Y ";
			break;
		case 2:
			result += L"Z ";
			break;
		}
	}

	return result.substr(0, result.length() - 1); //remove the trailing space character
}

void CalibrationMode::accelerometerCalibration()
{
	switch (m_currentStage)
	{
	case 0:
	{
		if (!m_stageSet)
		{
			//Give a brief introduction on what the test entails
			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"A 6-point tumble calibration will be performed on the accelerometer. Press the continue button when ready to begin and then follow the on-screen instructions.");
			
			//Update and display the subtitle text
			m_uiManager.getElement<TextOverlay>(L"Subtitle Text")->updateText(L"Accelerometer: Data Calibration");
			m_uiManager.getElement<TextOverlay>(L"Subtitle Text")->removeState(UIElementState::Invisible);
			
			data_timer_duration = 5000; //the acceleromter needs 5 seconds of data at each stage
			m_stageSet = true;
		}

		break;
	}
	case 1:
	{
		if (!m_stageSet)
		{
			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"Rotate the sensor so that  the +Y axis is pointing upwards like shown in the image. Leave the sensor stationary like this for 5 seconds as it collects data. Press the continue button when ready.");
			m_quaternion = QuaternionMultiply({ 0.707f, 0.0f, 0.0f, 0.707f }, { 0.707f, -0.707f, 0.0f, 0.0f }); //glm quaternions are of the form {w, x, y, z} but DirectX are {x, y, z, w} which can lead to some confusion
			m_renderQuaternion = {m_quaternion.x, m_quaternion.y , m_quaternion.z , m_quaternion.w };
			m_needsCamera = true; //alerts the mode screen class to actually render the 3d image
			m_stageSet = true;
		}
		
		break;
	}
	case 2:
	case 4:
	case 6:
	case 8:
	case 10:
	case 12:
	{
		prepareRecording();
		break;
	}
	case 3:
	{
		if (!m_stageSet)
		{
			accAverageData(); //Average the acc data obtained in the last stage
			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"Data collection complete. Rotate the sensor so that the -X axis is pointing up as shown in the image. When ready, press the continue button. Make sure to keep the sensor as still as possible until the timer runs out.");
			m_renderQuaternion = { 0.707f, 0.0f, 0.0f, 0.707f };
			m_needsCamera = true;
			m_stageSet = true;
		}
		break;
	}
	case 5:
	{
		if (!m_stageSet)
		{
			accAverageData(); //Average the acc data obtained in the last stage
			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"Data collection complete. Lay the sensor flat on the table so that the +Z-axis is pointing up as shown in the image. When ready, press the continue button. Make sure to keep the sensor as still as possible until the timer runs out.");
			m_renderQuaternion = { -0.174f, 0.0f, 0.0f, 0.985f }; //20 degrees of tilt upwards to help see the front face of the sensor
			m_needsCamera = true;
			m_stageSet = true;
		}
		break;
	}
	case 7:
	{
		if (!m_stageSet)
		{
			accAverageData(); //Average the acc data obtained in the last stage
			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"Data collection complete. Rotate sensor 90 degrees so that the +X-axis is pointing up as shown in the image. When ready, press the continue button. Make sure to keep the sensor as still as possible until the timer runs out.");
			m_renderQuaternion = { -0.707f, 0.0f, 0.0f, 0.707f };
			m_needsCamera = true;
			m_stageSet = true;
		}
		break;
	}
	case 9:
	{
		if (!m_stageSet)
		{
			accAverageData(); //Average the acc data obtained in the last stage
			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"Data collection complete. Rotate sensor 90 degrees so that the -Z-axis is pointing up as shown in the image. When ready, press the continue button. Make sure to keep the sensor as still as possible until the timer runs out.");
			m_quaternion = QuaternionMultiply({ 0.985f, -0.174f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }); //glm quaternions are of the form {w, x, y, z} but DirectX are {x, y, z, w} which can lead to some confusion
			m_renderQuaternion = { m_quaternion.x, m_quaternion.y , m_quaternion.z , m_quaternion.w };
			m_needsCamera = true;
			m_stageSet = true;
		}
		break;
	}
	case 11:
	{
		if (!m_stageSet)
		{
			accAverageData(); //Average the acc data obtained in the last stage
			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"Data collection complete. Rotate sensor 90 degrees so that the -Y-axis is pointing up as shown in the image. When ready, press the continue button. Make sure to keep the sensor as still as possible until the timer runs out.");
			m_quaternion = QuaternionMultiply({ 0.707f, -0.707f, 0.0f, 0.0f }, { 0.707f, 0.0f, 0.707f, 0.0f }); //glm quaternions are of the form {w, x, y, z} but DirectX are {x, y, z, w} which can lead to some confusion
			m_renderQuaternion = { m_quaternion.x, m_quaternion.y , m_quaternion.z , m_quaternion.w };
			m_needsCamera = true;
			m_stageSet = true;
		}
		break;
	}
	case 13:
	{
		if (!m_stageSet)
		{
			//The tumble calibration is complete, use the recorded data to calculate the offset and gain values for the accelerometer
			accAverageData(); //Average the acc data obtained in the last stage
			calculateCalNumbers();
			std::wstring completionText = L"Calibration complete, see results below. Press continue to return to main calibration screen.\n\n\n"
				L"Offset = [" + std::to_wstring(acc_off[0]) + L", " + std::to_wstring(acc_off[1]) + L", " + std::to_wstring(acc_off[2]) + L"]\n\n"
				L"              [" + std::to_wstring(acc_gain[0][0]) + L", " + std::to_wstring(acc_gain[0][1]) + L", " + std::to_wstring(acc_gain[0][2]) + L"]\n"
				L"Gains  =  [" + std::to_wstring(acc_gain[1][0]) + L", " + std::to_wstring(acc_gain[1][1]) + L", " + std::to_wstring(acc_gain[1][2]) + L"]\n"
				L"               [" + std::to_wstring(acc_gain[2][0]) + L", " + std::to_wstring(acc_gain[2][1]) + L", " + std::to_wstring(acc_gain[2][2]) + L"]";
			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(completionText);
			displayGraph();

			//momentarily change the text of the continue button to say "yes", also make the "no" button visible
			m_uiManager.getElement<TextButton>(L"Continue Button")->updateText(L"Yes");
			m_uiManager.getElement<TextButton>(L"No Button")->removeState(UIElementState::Invisible);

			m_stageSet = true;
		}
		break;
	}
	case 14:
	{

		//Finally, transition the Personal Caddie back into Connected Mode when the calibration is over to lower power consumption
		auto mode = PersonalCaddiePowerMode::CONNECTED_MODE;
		m_mode_screen_handler(ModeAction::PersonalCaddieChangeMode, (void*)&mode);//request the Personal Caddie to be placed into active mode to start recording data

		//If we chose to accept the new calibration numbers then make the update now
		if (accept_cal)
		{
			CalibrationRequest new_cal{ SensorCalibrationAction::SET_SENSOR_CAL, ACC_SENSOR, {acc_off, acc_gain},  {} };
			m_mode_screen_handler(ModeAction::SensorCalibration, (void*)&new_cal); //Wrap the calibration info into a void pointer and send it off
		}
		loadModeMainPage();
	}
	}
}

void CalibrationMode::accelerometerAxisCalibration()
{
	switch (m_currentStage)
	{
	case 0:
	{
		if (!m_stageSet)
		{
			//In the first phase of the test we give a brief intro message on what to expect and set up a few important variables for the test.
			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"Welcome to the accelerometer axis calibration, this is a simple test carried out in three phases where in each phase we point one of the accelerometer's three principle axis directly upwards towards gravity."
				" Based on the magnitude and polarity of the reading during each phase of the test we can figure out if the data from any of the axes need to be swapped or inverted.");

			//Update and display the subtitle text
			m_uiManager.getElement<TextOverlay>(L"Subtitle Text")->updateText(L"Accelerometer: Axis Calibration");
			m_uiManager.getElement<TextOverlay>(L"Subtitle Text")->removeState(UIElementState::Invisible);

			//Load existing axis data so that we can counteract it as data comes in
			CalibrationRequest getAccAxisData{ SensorCalibrationAction::GET_SENSOR_AXIS_CAL, ACC_SENSOR };
			m_mode_screen_handler(ModeAction::SensorCalibration, (void*)&getAccAxisData); //this happens synchronously

			//Set pointers to current axis swap data
			m_axis_swap = acc_axis_swap;
			m_axis_polarity = acc_axis_polarity;
			
			data_timer_duration = 2000;
			m_stageSet = true;
		}

		break;
	}
	case 1:
	{
		if (!m_stageSet)
		{
			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"To start calibration of the accelerometer axes, point the +X-axis of the sensor straight upwards like depicted in the image and hold it there for 2 seconds. Press continue when ready to proceed.");
			m_uiManager.getElement<TextOverlay>(L"Subtitle Text")->updateText(L"Accelerometer: Axis Calibration"); //update the sub-title text
			m_renderQuaternion = { -0.707f, 0.0f, 0.0f, 0.707f };
			m_needsCamera = true; //alerts the mode screen class to actually render the 3d image
			m_stageSet = true;
		}

		break;
	}
	case 2:
	case 4:
	case 6:
	{
		prepareRecording();
		break;
	}
	case 3:
	{
		if (!m_stageSet)
		{
			//With the data collected, see if a swap or inversion needs to occur for the x-axis
			accAxisCalculate(0);

			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"Data collection complete. Rotate the sensor so that the +Z axis is pointing up as shown in the image and hold it there for 2 seconds. When ready, press the continue button.");
			m_renderQuaternion = { -0.174f, 0.0f, 0.0f, 0.985f }; //20 degrees of tilt upwards to help see the front face of the sensor
			m_needsCamera = true;
			m_stageSet = true;
		}
		break;
	}
	case 5:
	{
		if (!m_stageSet)
		{
			//With the data collected, see if a swap or inversion needs to occur for the z-axis
			accAxisCalculate(2);

			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"Data collection complete. Rotate sensor 90 degrees so that the +Y-axis is pointing up as shown in the image. When ready, press the continue button.");
			m_quaternion = QuaternionMultiply({ 0.707f, 0.0f, 0.0f, 0.707f }, { 0.707f, -0.707f, 0.0f, 0.0f }); //glm quaternions are of the form {w, x, y, z} but DirectX are {x, y, z, w} which can lead to some confusion
			m_renderQuaternion = { m_quaternion.x, m_quaternion.y, m_quaternion.z, m_quaternion.w };
			m_needsCamera = true;
			m_stageSet = true;

		}
		break;
	}
	case 7:
	{
		if (!m_stageSet)
		{
			//With the data collected, see if a swap or inversion needs to occur for the y-axis
			accAxisCalculate(1);

			std::wstring completionText = L"Accelerometer axis calibration complete, see results below. Press continue to proceed to the gyroscope axis calibration.\n\n\n"
				L"Standard Axes = [X, Y, Z]\n"
				L"New Axes = [" + axisResultString(test_acc_axis_swap, test_acc_axis_polarity) + L"]\n\n";

			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(completionText);

			//momentarily change the text of the continue button to say "yes", also make the "no" button visible
			m_uiManager.getElement<TextButton>(L"Continue Button")->updateText(L"Yes");
			m_uiManager.getElement<TextButton>(L"No Button")->removeState(UIElementState::Invisible);

			m_stageSet = true;
		}
		break;
	}
	case 8:
	{
		//Finally, transition the Personal Caddie back into Connected Mode when the calibration is over to lower power consumption
		auto mode = PersonalCaddiePowerMode::CONNECTED_MODE;
		m_mode_screen_handler(ModeAction::PersonalCaddieChangeMode, (void*)&mode);//request the Personal Caddie to be placed into active mode to start recording data

		//If we chose to accept the new calibration numbers then make the update now
		if (accept_cal)
		{
			CalibrationRequest new_cal{ SensorCalibrationAction::SET_SENSOR_AXIS_CAL, ACC_SENSOR, {},  {test_acc_axis_swap, test_acc_axis_polarity} };
			m_mode_screen_handler(ModeAction::SensorCalibration, (void*)&new_cal); //Wrap the calibration info into a void pointer and send it off
		}
		loadModeMainPage();
	}
	}
}

void CalibrationMode::gyroscopeCalibration()
{
	switch (m_currentStage)
	{
	case 0:
	{
		if (!m_stageSet)
		{
			//Give a brief introduction on what the test entails
			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"Two tests will be performed to calibrate the gyroscope. "
				L"The first test is easy and just involves keeping the gyroscope still.The second test is carried out in three stages.In each of these stages we rotate the gyroscope by 90 degrees about one of its axes.");

			//Update and display the subtitle text
			m_uiManager.getElement<TextOverlay>(L"Subtitle Text")->updateText(L"Gyroscope: Data Calibration");
			m_uiManager.getElement<TextOverlay>(L"Subtitle Text")->removeState(UIElementState::Invisible);

			data_timer_duration = 5000; //the acceleromter needs 5 seconds of data at each stage
			m_stageSet = true;
		}

		break;
	}
	case 1:
	{
		if (!m_stageSet)
		{
			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"Lay the gyroscope flat on the table for 5 seconds while the axes offsets are calculated. Click continue when ready.");
			data_timer_duration = 5000; //the acceleromter needs 5 seconds of data at each stage
			m_stageSet = true;
		}

		break;
	}
	case 2:
	case 4:
	case 6:
	case 8:
	{
		prepareRecording();
		break;
	}
	case 3:
	{
		if (!m_stageSet)
		{
			
			calculateCalNumbers(); //Calculate the gyroscope axes offsets
			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"Data collection complete. Next, the gryoscope axis gain needs to be calculated. This is done in three stages, one for each axis. For this first stage we rotate the sensor clockwise about the Y-axis like shown in the animation. Slowly rotate the sensor as close to 90 degrees as possible over the course of 5 seconds. Press continue when ready.");
			m_renderQuaternion = { 0.0f, 0.0f, 0.0f, 1.0f };
			m_needsCamera = true;
			m_stageSet = true;
		}
		else
		{
			//animate the sensor image so that it's rotating about the y-axis. To be easier to see the 
			//full rotation of 90 degrees should take 2 seconds. At a frame rate of 60 Hz this means we
			//need to rotate by 45 deg/s / 60 frame/s = 0.75 deg/frame
			m_quaternion = QuaternionMultiply({ 0.99997f, 0.00654f, 0.0f, 0.0f }, { m_renderQuaternion.m128_f32[3], m_renderQuaternion.m128_f32[0], m_renderQuaternion.m128_f32[1], m_renderQuaternion.m128_f32[2] }); //glm quaternions are of the form {w, x, y, z} but DirectX are {x, y, z, w} which can lead to some confusion
			m_renderQuaternion = { m_quaternion.x, m_quaternion.y, m_quaternion.z, m_quaternion.w };

			if (m_quaternion.w < 0.707f) m_renderQuaternion = { 0.0f, 0.0f, 0.0f, 1.0f }; //reset the animation after going about 90 degrees
		}
		break;
	}
	case 5:
	{
		if (!m_stageSet)
		{
			calculateCalNumbers(); //Calculate the gyroscope y-axis gain
			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"Data collection complete. We know slowly rotate the sensor 90 degrees clockwise about the Z-axis over the course of 5 seconds like shown in the animation. Press continue when ready.");
			m_renderQuaternion = { 0.0f, 0.0f, 0.0f, 1.0f };
			m_needsCamera = true;

			m_stageSet = true;
		}
		else
		{
			//animate the sensor image so that it's rotating about the x-axis. To be easier to see the 
			//full rotation of 90 degrees should take 2 seconds. At a frame rate of 60 Hz this means we
			//need to rotate by 45 deg/s / 60 frame/s = 0.75 deg/frame
			m_quaternion = QuaternionMultiply({ 0.99997f, 0.0f, 0.00654f, 0.0f }, { m_renderQuaternion.m128_f32[3], m_renderQuaternion.m128_f32[0], m_renderQuaternion.m128_f32[1], m_renderQuaternion.m128_f32[2] }); //glm quaternions are of the form {w, x, y, z} but DirectX are {x, y, z, w} which can lead to some confusion
			m_renderQuaternion = { m_quaternion.x, m_quaternion.y, m_quaternion.z, m_quaternion.w };

			if (m_quaternion.w < 0.707f) m_renderQuaternion = { 0.0f, 0.0f, 0.0f, 1.0f }; //reset the animation after going about 90 degrees
		}
		break;
	}
	case 7:
	{
		if (!m_stageSet)
		{
			calculateCalNumbers(); //Calculate the gyroscopezy-axis gain
			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"Data collection complete. We know slowly rotate the sensor 90 degrees clockwise about the X-axis over the course of 5 seconds like shown in the animation. Press continue when ready.");
			m_renderQuaternion = { 0.0f, 0.0f, 0.0f, 1.0f };
			m_needsCamera = true;
			m_stageSet = true;
		}
		else
		{
			//animate the sensor image so that it's rotating about the x-axis. To be easier to see the 
			//full rotation of 90 degrees should take 2 seconds. At a frame rate of 60 Hz this means we
			//need to rotate by 45 deg/s / 60 frame/s = 0.75 deg/frame
			m_quaternion = QuaternionMultiply({ 0.99997f, 0.0f, 0.0f, 0.00654f }, { m_renderQuaternion.m128_f32[3], m_renderQuaternion.m128_f32[0], m_renderQuaternion.m128_f32[1], m_renderQuaternion.m128_f32[2] }); //glm quaternions are of the form {w, x, y, z} but DirectX are {x, y, z, w} which can lead to some confusion
			m_renderQuaternion = { m_quaternion.x, m_quaternion.y, m_quaternion.z, m_quaternion.w };

			if (m_quaternion.w < 0.707f) m_renderQuaternion = { 0.0f, 0.0f, 0.0f, 1.0f }; //reset the animation after going about 90 degrees
		}
		break;
	}
	case 9:
	{
		if (!m_stageSet)
		{
			calculateCalNumbers(); //Calculate the gyroscope x-axis gain
			std::wstring completionText = L"Calibration complete, see results below. Press continue to return to main calibration screen.\n\n\n"
				L"Offset = [" + std::to_wstring(gyr_off[0]) + L", " + std::to_wstring(gyr_off[1]) + L", " + std::to_wstring(gyr_off[2]) + L"]\n\n"
				L"              [" + std::to_wstring(gyr_gain[0][0]) + L"]\n"
				L"Gains  =  [" + std::to_wstring(gyr_gain[1][1]) + L"]\n"
				L"               [" + std::to_wstring(gyr_gain[2][2]) + L"]";
			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(completionText);
			displayGraph();

			//momentarily change the text of the continue button to say "yes", also make the "no" button visible
			m_uiManager.getElement<TextButton>(L"Continue Button")->updateText(L"Yes");
			m_uiManager.getElement<TextButton>(L"No Button")->removeState(UIElementState::Invisible);

			m_stageSet = true;
		}
		break;
	}
	case 10:
	{
		//Finally, transition the Personal Caddie back into Connected Mode when the calibration is over to lower power consumption
		auto mode = PersonalCaddiePowerMode::CONNECTED_MODE;
		m_mode_screen_handler(ModeAction::PersonalCaddieChangeMode, (void*)&mode);//request the Personal Caddie to be placed into active mode to start recording data

		//If we chose to accept the new calibration numbers then make the update now
		if (accept_cal)
		{
			CalibrationRequest new_cal{ SensorCalibrationAction::SET_SENSOR_CAL, GYR_SENSOR, {gyr_off, gyr_gain},  {} };
			m_mode_screen_handler(ModeAction::SensorCalibration, (void*)&new_cal); //Wrap the calibration info into a void pointer and send it off
		}
		loadModeMainPage();
	}
	}
}

void CalibrationMode::gyroscopeAxisCalibration()
{
	switch (m_currentStage)
	{
	case 0:
	{
		if (!m_stageSet)
		{
			//In the first phase of the test we give a brief intro message on what to expect and set up a few important variables for the test.
			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"Welcome to the gyroscope axis calibration, this is a simple test carried out in three phases where in each phase we rotate the gyroscope about one of its principle axes."
				" Based on the magnitude and polarity of the reading during each phase of the test we can figure out if the data from any of the axes need to be swapped or inverted.");

			//Update and display the subtitle text
			m_uiManager.getElement<TextOverlay>(L"Subtitle Text")->updateText(L"Gyroscope: Axis Calibration");
			m_uiManager.getElement<TextOverlay>(L"Subtitle Text")->removeState(UIElementState::Invisible);

			//Load existing axis data so that we can counteract it as data comes in
			CalibrationRequest getGyrAxisData{ SensorCalibrationAction::GET_SENSOR_AXIS_CAL, GYR_SENSOR };
			m_mode_screen_handler(ModeAction::SensorCalibration, (void*)&getGyrAxisData); //this happens synchronously

			//Set pointers to current axis swap data. This is necessary so that we don't
			//look at already swapped axis data.
			m_axis_swap = gyr_axis_swap;
			m_axis_polarity = gyr_axis_polarity;

			data_timer_duration = 2000;
			m_stageSet = true;
		}

		break;
	}
	case 1:
	{
		if (!m_stageSet)
		{
			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"Rotate the sensor clockwise about the X-axis by 90 degrees, like depicted in the animation. Data will be collected for 2 seconds, press continue when ready");
			m_renderQuaternion = { 0.0f, 0.0f, 0.0f, 1.0f };
			m_needsCamera = true;

			m_stageSet = true;
		}
		else
		{
			//animate the sensor image so that it's rotating about the x-axis. To be easier to see the 
			//full rotation of 90 degrees should take 2 seconds. At a frame rate of 60 Hz this means we
			//need to rotate by 45 deg/s / 60 frame/s = 0.75 deg/frame
			m_quaternion = QuaternionMultiply({ 0.99997f, 0.0f, 0.0f, 0.00654f }, { m_renderQuaternion.m128_f32[3], m_renderQuaternion.m128_f32[0], m_renderQuaternion.m128_f32[1], m_renderQuaternion.m128_f32[2] }); //glm quaternions are of the form {w, x, y, z} but DirectX are {x, y, z, w} which can lead to some confusion
			m_renderQuaternion = { m_quaternion.x, m_quaternion.y, m_quaternion.z, m_quaternion.w };

			if (m_quaternion.w < 0.707f) m_renderQuaternion = { 0.0f, 0.0f, 0.0f, 1.0f }; //reset the animation after going about 90 degrees
		}
		break;
	}
	case 2:
	case 4:
	case 6:
	{
		prepareRecording();
		break;
	}
	case 3:
	{
		if (!m_stageSet)
		{
			//With the data collected, see if a swap or inversion needs to occur for the y-axis
			gyrAxisCalculate(0);

			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"Data collection complete. Rotate the sensor clockwise about the Y-axis by 90 degrees, like depicted in the animation. Data will be collected for 2 seconds, press continue when ready");

			//Clear accumulated date before moving the next gyroscope axis calibration stage
			m_graphDataX.clear();
			m_graphDataY.clear();
			m_graphDataZ.clear();

			m_renderQuaternion = { 0.0f, 0.0f, 0.0f, 1.0f };
			m_needsCamera = true;

			m_stageSet = true;
		}
		else
		{
			//animate the sensor image so that it's rotating about the y-axis. To be easier to see the 
			//full rotation of 90 degrees should take 2 seconds. At a frame rate of 60 Hz this means we
			//need to rotate by 45 deg/s / 60 frame/s = 0.75 deg/frame
			m_quaternion = QuaternionMultiply({ 0.99997f, 0.00654f, 0.0f, 0.0f }, { m_renderQuaternion.m128_f32[3], m_renderQuaternion.m128_f32[0], m_renderQuaternion.m128_f32[1], m_renderQuaternion.m128_f32[2] }); //glm quaternions are of the form {w, x, y, z} but DirectX are {x, y, z, w} which can lead to some confusion
			m_renderQuaternion = { m_quaternion.x, m_quaternion.y, m_quaternion.z, m_quaternion.w };

			if (m_quaternion.w < 0.707f) m_renderQuaternion = { 0.0f, 0.0f, 0.0f, 1.0f }; //reset the animation after going about 90 degrees
		}
		break;
	}
	case 5:
	{
		if (!m_stageSet)
		{
			//With the data collected, see if a swap or inversion needs to occur for the y-axis
			gyrAxisCalculate(1);

			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"Data collection complete. Rotate the sensor clockwise about the Z-axis by 90 degrees, like depicted in the animation. Data will be collected for 2 seconds, press continue when ready");

			//Clear accumulated date before moving the next gyroscope axis calibration stage
			m_graphDataX.clear();
			m_graphDataY.clear();
			m_graphDataZ.clear();

			m_renderQuaternion = { 0.0f, 0.0f, 0.0f, 1.0f };
			m_needsCamera = true;

			m_stageSet = true;
		}
		else
		{
			//animate the sensor image so that it's rotating about the x-axis. To be easier to see the 
			//full rotation of 90 degrees should take 2 seconds. At a frame rate of 60 Hz this means we
			//need to rotate by 45 deg/s / 60 frame/s = 0.75 deg/frame
			m_quaternion = QuaternionMultiply({ 0.99997f, 0.0f, 0.00654f, 0.0f }, { m_renderQuaternion.m128_f32[3], m_renderQuaternion.m128_f32[0], m_renderQuaternion.m128_f32[1], m_renderQuaternion.m128_f32[2] }); //glm quaternions are of the form {w, x, y, z} but DirectX are {x, y, z, w} which can lead to some confusion
			m_renderQuaternion = { m_quaternion.x, m_quaternion.y, m_quaternion.z, m_quaternion.w };

			if (m_quaternion.w < 0.707f) m_renderQuaternion = { 0.0f, 0.0f, 0.0f, 1.0f }; //reset the animation after going about 90 degrees
		}
		break;
	}
	case 7:
	{
		if (!m_stageSet)
		{
			//With the data collected, see if a swap or inversion needs to occur for the y-axis
			gyrAxisCalculate(2);

			std::wstring completionText = L"Gyroscope axis calibration complete, see results below. Press continue to proceed to the magnetometer axis calibration.\n\n\n"
				L"Standard Axes = [X, Y, Z]\n"
				L"New Axes = [" + axisResultString(test_gyr_axis_swap, test_gyr_axis_polarity) + L"]\n\n";

			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(completionText);

			//momentarily change the text of the continue button to say "yes", also make the "no" button visible
			m_uiManager.getElement<TextButton>(L"Continue Button")->updateText(L"Yes");
			m_uiManager.getElement<TextButton>(L"No Button")->removeState(UIElementState::Invisible);

			m_stageSet = true;
		}
		break;
	}
	case 8:
	{
		//Finally, transition the Personal Caddie back into Connected Mode when the calibration is over to lower power consumption
		auto mode = PersonalCaddiePowerMode::CONNECTED_MODE;
		m_mode_screen_handler(ModeAction::PersonalCaddieChangeMode, (void*)&mode);//request the Personal Caddie to be placed into active mode to start recording data

		//If we chose to accept the new calibration numbers then make the update now
		if (accept_cal)
		{
			CalibrationRequest new_cal{ SensorCalibrationAction::SET_SENSOR_AXIS_CAL, GYR_SENSOR, {},  {test_gyr_axis_swap, test_gyr_axis_polarity} };
			m_mode_screen_handler(ModeAction::SensorCalibration, (void*)&new_cal); //Wrap the calibration info into a void pointer and send it off
		}
		loadModeMainPage();
	}
	}
}

void CalibrationMode::magnetometerCalibration()
{
	switch (m_currentStage)
	{
	case 0:
	{
		if (!m_stageSet)
		{
			//Give a brief introduction on what the test entails
			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"To calibrate the magnetometer we need to take sources of interference into account.These include "
				L"both the hard and soft iron deposits(large scale deviations in magnetic field, like large iron deposits in the ground, and small scale deviations in magnetic field, such as from nearby metal like a golf club shaft).");

			//Update and display the subtitle text
			m_uiManager.getElement<TextOverlay>(L"Subtitle Text")->updateText(L"Magnetometer: Data Calibration");
			m_uiManager.getElement<TextOverlay>(L"Subtitle Text")->removeState(UIElementState::Invisible);

			m_stageSet = true;
		}

		break;
	}
	case 1:
	{
		if (!m_stageSet)
		{
			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"Take the sensor in your hand and rotate it along all three axes in figure-8 patterns for 20 seconds. Imaging the sensor is inside a sphere and you're trying to make the front of the sensor point at as many locations in the sphere as possible. Press the continue button when ready.");
			data_timer_duration = 20000; //the magnetometer collects 20 seconds worth of data
			m_stageSet = true;
		}

		break;
	}
	case 2:
	{
		prepareRecording();
		break;
	}
	case 3:
	{
		if (!m_stageSet)
		{
			calculateCalNumbers();
			std::wstring completionText = L"Data collection is complete. The three graphs repsent the calibrated data (green) and uncalibrated data (red) as seen from the XY, XZ and YZ planes.\n\n\n"
				L"Offset = [" + std::to_wstring(mag_off[0]) + L", " + std::to_wstring(mag_off[1]) + L", " + std::to_wstring(mag_off[2]) + L"]\n\n"
				L"              [" + std::to_wstring(mag_gain[0][0]) + L", " + std::to_wstring(mag_gain[0][1]) + L", " + std::to_wstring(mag_gain[0][2]) + L"]\n"
				L"Gains  =  [" + std::to_wstring(mag_gain[1][0]) + L", " + std::to_wstring(mag_gain[1][1]) + L", " + std::to_wstring(mag_gain[1][2]) + L"]\n"
				L"               [" + std::to_wstring(mag_gain[2][0]) + L", " + std::to_wstring(mag_gain[2][1]) + L", " + std::to_wstring(mag_gain[2][2]) + L"]";
			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(completionText);
			displayGraph();

			//momentarily change the text of the continue button to say "yes", also make the "no" button visible
			m_uiManager.getElement<TextButton>(L"Continue Button")->updateText(L"Yes");
			m_uiManager.getElement<TextButton>(L"No Button")->removeState(UIElementState::Invisible);

			m_stageSet = true;
		}
		break;
	}
	case 4:
	{
		//Finally, transition the Personal Caddie back into Connected Mode when the calibration is over to lower power consumption
		auto mode = PersonalCaddiePowerMode::CONNECTED_MODE;
		m_mode_screen_handler(ModeAction::PersonalCaddieChangeMode, (void*)&mode);//request the Personal Caddie to be placed into active mode to start recording data

		//If we chose to accept the new calibration numbers then make the update now
		if (accept_cal)
		{
			CalibrationRequest new_cal{ SensorCalibrationAction::SET_SENSOR_CAL, MAG_SENSOR, {mag_off, mag_gain},  {} };
			m_mode_screen_handler(ModeAction::SensorCalibration, (void*)&new_cal); //Wrap the calibration info into a void pointer and send it off
		}
		loadModeMainPage();
	}
	}
}

void CalibrationMode::magnetometerAxisCalibration()
{
	switch (m_currentStage)
	{
	case 0:
	{
		if (!m_stageSet)
		{
			//In the first phase of the test we give a brief intro message on what to expect and set up a few important variables for the test.
			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"Welcome to the magnetometer axis calibration. Unlike the accelerometer and gyroscope axis calibrations, the magnetometer calibration is somewhat complicated. "
				L"This is because the magnetometer relies on Earth's magnetic field, which changes magnitdue and direction depending on where on the Earth you currently are. "
				L"Since the direction of the Magnetic field isn't known, we need to calibrate the axes of the magnetometer in a few distinct stages.");

			//Update and display the subtitle text
			m_uiManager.getElement<TextOverlay>(L"Subtitle Text")->updateText(L"Magnetometer: Axis Calibration");
			m_uiManager.getElement<TextOverlay>(L"Subtitle Text")->removeState(UIElementState::Invisible);

			//Load existing axis data so that we can counteract it as data comes in
			CalibrationRequest getMagAxisData{ SensorCalibrationAction::GET_SENSOR_AXIS_CAL, MAG_SENSOR };
			m_mode_screen_handler(ModeAction::SensorCalibration, (void*)&getMagAxisData); //this happens synchronously

			//Set pointers to current axis swap data. This is necessary so that we don't
			//look at already swapped axis data.
			m_axis_swap = mag_axis_swap;
			m_axis_polarity = mag_axis_polarity;

			data_timer_duration = 5000;
			m_stageSet = true;
		}
		break;
	}
	case 1:
	{
		if (!m_stageSet)
		{
			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"The first stage of the calibration involves properly mappining each axis of the sensor to its appropriate axis of the Personal Caddie. This can be done "
				L"by rotating the Personal Caddie about two of its principle axes and examining the data. Whichever axis we rotate the sensor around will have a relatively flat set of data vs. the other two axes whose data "
			    L"will have ups and downs. First we carry out this test with the Z-axis as it will be the easiest. Lay the sensor flat on the table and  rotate it 360 degrees over the course of 5 seconds as depicted in the animation. Press continue when ready to proceed.");

			m_renderQuaternion = { 0.0f, 0.0f, 0.0f, 1.0f };
			m_needsCamera = true;

			m_stageSet = true;
		}
		else
		{
			//animate the sensor image so that it's rotating about the z-axis. To be easier to see the 
			//full rotation of 180 degrees should take 2 seconds. At a frame rate of 60 Hz this means we
			//need to rotate by 90 deg/s / 60 frame/s = 1.5 deg/frame
			m_quaternion = QuaternionMultiply({ 0.99991f, 0.0f, 0.01314f, 0.0f }, { m_renderQuaternion.m128_f32[3], m_renderQuaternion.m128_f32[0], m_renderQuaternion.m128_f32[1], m_renderQuaternion.m128_f32[2] }); //glm quaternions are of the form {w, x, y, z} but DirectX are {x, y, z, w} which can lead to some confusion
			m_renderQuaternion = { m_quaternion.x, m_quaternion.y, m_quaternion.z, m_quaternion.w };

			if (m_quaternion.w < 0.05f) m_renderQuaternion = { 0.0f, 0.0f, 0.0f, 1.0f }; //reset the animation after going about 90 degrees
		}
		break;
	}
	case 2:
	case 5:
	{
		prepareRecording();
		break;
	}
	case 3:
	{
		if (!m_stageSet)
		{
			//With the data collected, see if a swap or inversion needs to occur for the y-axis
			magAxisCalculate(2);
			displayGraph();

			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"Data collection complete, the graph shows the data from each axis. The line with the least variation in it represents the data taken From the Personal Caddie's Z axis.");

			m_stageSet = true;
		}
		break;
	}
	case 4:
	{
		if (!m_stageSet)
		{
			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"Next will be the figuring out the Y-axis, as it's the next easiest. Take the sensor in your hand and rotate it 360 degrees about the Y axis over the course of "
			    L"5 seconds like depicted in the animation. Press the continue button when ready to proceed.");

			//Make the graph invisible again
			m_uiManager.getElement<Graph>(L"Acc Graph")->updateState(UIElementState::Invisible);

			//Clear accumulated date before moving the next stage
			m_graphDataX.clear();
			m_graphDataY.clear();
			m_graphDataZ.clear();

			m_renderQuaternion = { 0.0f, 0.0f, 0.0f, 1.0f };
			m_needsCamera = true;

			m_stageSet = true;
		}
		else
		{
			//animate the sensor image so that it's rotating about the y-axis. To be easier to see the 
			//full rotation of 180 degrees should take 2 seconds. At a frame rate of 60 Hz this means we
			//need to rotate by 90 deg/s / 60 frame/s = 1.5 deg/frame
			m_quaternion = QuaternionMultiply({ 0.99991f, 0.01314f, 0.0f, 0.0f }, { m_renderQuaternion.m128_f32[3], m_renderQuaternion.m128_f32[0], m_renderQuaternion.m128_f32[1], m_renderQuaternion.m128_f32[2] }); //glm quaternions are of the form {w, x, y, z} but DirectX are {x, y, z, w} which can lead to some confusion
			m_renderQuaternion = { m_quaternion.x, m_quaternion.y, m_quaternion.z, m_quaternion.w };

			if (m_quaternion.w < 0.05f) m_renderQuaternion = { 0.0f, 0.0f, 0.0f, 1.0f }; //reset the animation after going about 90 degrees
		}
		break;
	}
	case 6:
	{
		if (!m_stageSet)
		{
			//With the data collected, see if a swap or inversion needs to occur for the y-axis. Once that is complete
			//we can deduce which axis is the x-axis by the process of elimination
			magAxisCalculate(1);
			magAxisCalculate(0);

			//Show graph of most recent data set
			m_uiManager.getElement<Graph>(L"Acc Graph")->removeAllLines(); //clear previous data from graph
			displayGraph();

			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"Data collection complete, the graph shows the data from each axis. The line with the least variation in it represents the data taken From the Personal Caddie's Y axis.");

			m_stageSet = true;
		}
		break;
	}
	case 7:
	{
		if (!m_stageSet)
		{
			std::wstring completionText = L"The X-axis can be deduced vai the process of elimination, so at this point we've now figured out the orientation of each axis of the magnetometer, see them displayed below. We still need to calculate the polarity of each axis, press continue to do so.\n\n\n"
				L"Standard Axes = [X, Y, Z]\n"
				L"New Axes = [" + axisResultString(test_mag_axis_swap, test_mag_axis_polarity) + L"]\n\n";

			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(completionText);
			m_stageSet = true;
		}
		break;
	}
	case 8:
	{
		if (!m_stageSet)
		{
			std::wstring completionText = L"When calculating the polarity of each axis, it isn't actually important that we match true North, all that matters is that each axis agrees in the direction they 'think' North is. "
				L"Since we now know which axis is which, we take turns pointing each one directly along the North-South line (this gives us the highest reading possible for each axis) and see whether the readings is positive "
				L"or negative. To do this, the value of the X-axis will be displayed on screen. Rotate the sensor in your hand until the number on the screen is as big of a positive number is you can make it. Press continue to start.";

			//Clear accumulated date before moving the next gyroscope axis calibration stage
			m_graphDataX.clear();
			m_graphDataY.clear();
			m_graphDataZ.clear();

			m_uiManager.getElement<Graph>(L"Acc Graph")->updateState(UIElementState::Invisible); //make the graph invisible from the previous step
			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(completionText);

			m_stageSet = true;
		}
		break;
	}
	case 9:
	{
		if (!m_stageSet)
		{
			prepareRecording();
			unlimited_record = true; //this allows us to continuously record data instead of recording over a set time limit
		}
		else if (m_currentlyRecording)
		{
			if (m_graphDataX.size() > 0)
			{
				//The axes swaps that were figured out in the previous few steps won't be applied to the sensor data so we need
				//to manually make sure that we're taking data from the true x-axis
				float true_x_reading = 0.0f;
				if (test_mag_axis_swap[0] == 0) true_x_reading = m_graphDataX.back().y;
				else if (test_mag_axis_swap[0] == 1) true_x_reading = m_graphDataY.back().y;
				else if (test_mag_axis_swap[0] == 2) true_x_reading = m_graphDataZ.back().y;

				std::wstring message = L"Magnetometer X-axis reading: " + std::to_wstring(true_x_reading) + L" Gauss";

				//Data will be continuously added to the data vectors, although, we really only care about the current x-reading.
				//If the data vectors get too long, erase everything except the current last reading
				if (m_graphDataX.size() > 1000)
				{
					DirectX::XMFLOAT2 current_data = { m_graphDataX.back().x, m_graphDataX.back().y };
					m_graphDataX = { current_data };
					m_graphDataY = { current_data };
					m_graphDataZ = { current_data };
				}

				m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(message);
			}
		}
		break;
	}
	case 10:
	{
		if (!m_stageSet)
		{
			//unlimited_record = false; //this will stop the current recording session
			stopRecording();

			//record the polarity of the x-axis
			float true_x_reading = 0.0f;
			int correct_axis = 0;
			if (test_mag_axis_swap[0] == 0) true_x_reading = m_graphDataX.back().y;
			else if (test_mag_axis_swap[0] == 1)
			{
				true_x_reading = m_graphDataY.back().y;
				correct_axis = 1;
			}
			else if (test_mag_axis_swap[0] == 2)
			{
				true_x_reading = m_graphDataZ.back().y;
				correct_axis = 2;
			}

			if (true_x_reading < 0) test_mag_axis_polarity[correct_axis] *= -1;

			m_stageSet = true;
		}
		break;
	}
	case 11:
	{
		if (!m_stageSet)
		{
			std::wstring completionText = L"Magnetic X-axis polarity calculated, moving on to the Y-axis. Keep the sensor in the exact same orientation, and then rotate it by 90 degrees about the Z-axis "
				L"so that the +Y-axis points in the same direction that the +X-axis was just pointing in. The value from the Y-axis will be displayed on screen as a guide. It's possible that the number will "
				L"be negative now which is perfectly ok for this stage of the calibration. Press Continue when ready to proceed.";

			//Clear accumulated date before moving the next gyroscope axis calibration stage
			m_graphDataX.clear();
			m_graphDataY.clear();
			m_graphDataZ.clear();

			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(completionText);

			m_stageSet = true;
		}
		break;
	}
	case 12:
	{
		if (!m_stageSet)
		{
			prepareRecording();
			//unlimited_record = true; //this allows us to continuously record data instead of recording over a set time limit
		}
		else if (m_currentlyRecording)
		{
			if (m_graphDataX.size() > 0)
			{
				//The axes swaps that were figured out in the previous few steps won't be applied to the sensor data so we need
				//to manually make sure that we're taking data from the true y-axis
				float true_y_reading = 0.0f;
				if (test_mag_axis_swap[1] == 0) true_y_reading = m_graphDataX.back().y;
				else if (test_mag_axis_swap[1] == 1) true_y_reading = m_graphDataY.back().y;
				else if (test_mag_axis_swap[1] == 2) true_y_reading = m_graphDataZ.back().y;

				std::wstring message = L"Magnetometer Y-axis reading: " + std::to_wstring(true_y_reading) + L" Gauss";

				//Data will be continuously added to the data vectors, although, we really only care about the current x-reading.
				//If the data vectors get too long, erase everything except the current last reading
				if (m_graphDataX.size() > 1000)
				{
					DirectX::XMFLOAT2 current_data = { m_graphDataX.back().x, m_graphDataX.back().y };
					m_graphDataX = { current_data };
					m_graphDataY = { current_data };
					m_graphDataZ = { current_data };
				}

				m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(message);
			}
		}
		break;
	}
	case 13:
	{
		if (!m_stageSet)
		{
			//unlimited_record = false; //this will stop the current recording session
			stopRecording();

			//record the polarity of the y-axis
			float true_y_reading = 0.0f;
			int correct_axis = 1;
			if (test_mag_axis_swap[1] == 0)
			{
				true_y_reading = m_graphDataX.back().y;
				correct_axis = 0;
			}
			else if (test_mag_axis_swap[1] == 1) true_y_reading = m_graphDataY.back().y;
			else if (test_mag_axis_swap[1] == 2)
			{
				true_y_reading = m_graphDataZ.back().y;
				correct_axis = 2;
			}

			if (true_y_reading < 0) test_mag_axis_polarity[correct_axis] *= -1;

			m_stageSet = true;
		}
		break;
	}
	case 14:
	{
		if (!m_stageSet)
		{
			std::wstring completionText = L"Magnetic Y-axis polarity calculated, moving on to the Z-axis. Keep the sensor in the exact same orientation, and then rotate it by 90 degrees about the X-axis "
				L"so that the +Z-axis points in the same direction that the +Y-axis was just pointing in. The value from the Y-axis will be displayed on screen as a guide. It's possible that the number will "
				L"be negative now which is perfectly ok for this stage of the calibration. Press Continue when ready to proceed.";

			//Clear accumulated date before moving the next gyroscope axis calibration stage
			m_graphDataX.clear();
			m_graphDataY.clear();
			m_graphDataZ.clear();

			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(completionText);

			m_stageSet = true;
		}
		break;
	}
	case 15:
	{
		if (!m_stageSet)
		{
			prepareRecording();
			//unlimited_record = true; //this allows us to continuously record data instead of recording over a set time limit
		}
		else if (m_currentlyRecording)
		{
			if (m_graphDataX.size() > 0)
			{
				//The axes swaps that were figured out in the previous few steps won't be applied to the sensor data so we need
				//to manually make sure that we're taking data from the true y-axis
				float true_z_reading = 0.0f;
				if (test_mag_axis_swap[2] == 0) true_z_reading = m_graphDataX.back().y;
				else if (test_mag_axis_swap[2] == 1) true_z_reading = m_graphDataY.back().y;
				else if (test_mag_axis_swap[2] == 2) true_z_reading = m_graphDataZ.back().y;

				std::wstring message = L"Magnetometer Z-axis reading: " + std::to_wstring(true_z_reading) + L" Gauss";

				//Data will be continuously added to the data vectors, although, we really only care about the current x-reading.
				//If the data vectors get too long, erase everything except the current last reading
				if (m_graphDataX.size() > 1000)
				{
					DirectX::XMFLOAT2 current_data = { m_graphDataX.back().x, m_graphDataX.back().y };
					m_graphDataX = { current_data };
					m_graphDataY = { current_data };
					m_graphDataZ = { current_data };
				}

				m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(message);
			}
		}
		break;
	}
	case 16:
	{
		if (!m_stageSet)
		{
			//unlimited_record = false; //this will stop the current recording session
			stopRecording();

			//record the polarity of the y-axis
			float true_z_reading = 0.0f;
			int correct_axis = 2;
			if (test_mag_axis_swap[2] == 0)
			{
				true_z_reading = m_graphDataX.back().y;
				correct_axis = 0;
			}
			else if (test_mag_axis_swap[2] == 1)
			{
				true_z_reading = m_graphDataY.back().y;
				correct_axis = 1;
			}
			else if (test_mag_axis_swap[2] == 2) true_z_reading = m_graphDataZ.back().y;

			if (true_z_reading < 0) test_mag_axis_polarity[correct_axis] *= -1;

			m_stageSet = true;
		}
		break;
	}
	case 17:
	{
		if (!m_stageSet)
		{
			std::wstring completionText = L"Magnetometer axis calibration complete, see results below.\n\n\n"
				L"Standard Axes = [X, Y, Z]\n"
				L"New Axes = [" + axisResultString(test_mag_axis_swap, test_mag_axis_polarity) + L"]\n\n";

			m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(completionText);

			//momentarily change the text of the continue button to say "yes", also make the "no" button visible
			m_uiManager.getElement<TextButton>(L"Continue Button")->updateText(L"Yes");
			m_uiManager.getElement<TextButton>(L"No Button")->removeState(UIElementState::Invisible);

			m_stageSet = true;
		}
		break;
	}
	case 18:
	{
		//Finally, transition the Personal Caddie back into Connected Mode when the calibration is over to lower power consumption
		auto mode = PersonalCaddiePowerMode::CONNECTED_MODE;
		m_mode_screen_handler(ModeAction::PersonalCaddieChangeMode, (void*)&mode);//request the Personal Caddie to be placed into active mode to start recording data

		//If we chose to accept the new calibration numbers then make the update now
		if (accept_cal)
		{
			CalibrationRequest new_cal{ SensorCalibrationAction::SET_SENSOR_AXIS_CAL, MAG_SENSOR, {},  {test_mag_axis_swap, test_mag_axis_polarity} };
			m_mode_screen_handler(ModeAction::SensorCalibration, (void*)&new_cal); //Wrap the calibration info into a void pointer and send it off
		}
		loadModeMainPage();
	}
	}
}

void CalibrationMode::calculateCalNumbers()
{
	if (m_currentSensor == ACC_SENSOR)
	{
		//For reference, the axis order for the tumble calibration is: [+Y, -X, +Z, +X, -Z, -Y]

		//Calculate the axis offsets by averaging the axis values during the positive and negative tests
		//for each individual axis (i.e. x-offset = ((-Xx + Xx)/2 + (-Yx + Yx)/2 + (-Zx + Zx)/2) / 3
		acc_off[0] = (((acc_cal[0][1] + acc_cal[0][3]) / 2.0f) + ((acc_cal[0][0] + acc_cal[0][5]) / 2.0f) + ((acc_cal[0][2] + acc_cal[0][4]) / 2.0f)) / 3.0f;
		acc_off[1] = (((acc_cal[1][0] + acc_cal[1][5]) / 2.0f) + ((acc_cal[1][1] + acc_cal[1][3]) / 2.0f) + ((acc_cal[1][2] + acc_cal[1][4]) / 2.0f)) / 3.0f;
		acc_off[2] = (((acc_cal[2][2] + acc_cal[2][4]) / 2.0f) + ((acc_cal[2][1] + acc_cal[2][3]) / 2.0f) + ((acc_cal[2][0] + acc_cal[2][5]) / 2.0f)) / 3.0f;

		//Calculate Gain Matrix using the offsets above. We take the average gain for both 
		//positive and negative tests of each axis, which actually causes the offset terms
		//to cancel out so each gain is found by (+reading - -reading) / 2G
		acc_gain[0][0] = (acc_cal[0][3] - acc_cal[0][1]) / (2.0f * (float)GRAVITY);
		acc_gain[1][0] = (acc_cal[1][3] - acc_cal[1][1]) / (2.0f * (float)GRAVITY);
		acc_gain[2][0] = (acc_cal[2][3] - acc_cal[2][1]) / (2.0f * (float)GRAVITY);

		acc_gain[0][1] = (acc_cal[0][0] - acc_cal[0][5]) / (2.0f * (float)GRAVITY);
		acc_gain[1][1] = (acc_cal[1][0] - acc_cal[1][5]) / (2.0f * (float)GRAVITY);
		acc_gain[2][1] = (acc_cal[2][0] - acc_cal[2][5]) / (2.0f * (float)GRAVITY);

		acc_gain[0][2] = (acc_cal[0][2] - acc_cal[0][4]) / (2.0f * (float)GRAVITY);
		acc_gain[1][2] = (acc_cal[1][2] - acc_cal[1][4]) / (2.0f * (float)GRAVITY);
		acc_gain[2][2] = (acc_cal[2][2] - acc_cal[2][4]) / (2.0f * (float)GRAVITY);

		invertAccMatrix(); //invert the matrix
	}
	else if (m_currentSensor == GYR_SENSOR)
	{
		if (m_currentStage == 3)
		{
			//this stage represents the gyroscope offest calculation
			gyr_off[0] = 0, gyr_off[1] = 0, gyr_off[2] = 0;
			for (int i = 0; i < m_graphDataX.size(); i++)
			{
				gyr_off[0] += m_graphDataX[i].y;
				gyr_off[1] += m_graphDataY[i].y;
				gyr_off[2] += m_graphDataZ[i].y;
			}

			gyr_off[0] /= m_graphDataX.size();
			gyr_off[1] /= m_graphDataY.size();
			gyr_off[2] /= m_graphDataZ.size();
		}
		else
		{
			//this stage represents the gyroscope gain calculation. We subtract the zero-offset bias
			//(calculated in the first stage of the calibration) before integrating the data.
			if (m_currentStage == 5)
			{
				for (int i = 1; i < m_graphDataY.size(); i++) gyr_gain_y[1] += integrateData(m_graphDataY[i].y - gyr_off[1], m_graphDataY[i - 1].y - gyr_off[1], m_graphDataY[i].x - m_graphDataY[i - 1].x);
				gyr_gain_y[1] = 90 / gyr_gain_y[1];
			}
			else if (m_currentStage == 7)
			{
				for (int i = 1; i < m_graphDataZ.size(); i++) gyr_gain_z[2] += integrateData(m_graphDataZ[i].y - gyr_off[2], m_graphDataZ[i - 1].y - gyr_off[2], m_graphDataZ[i].x - m_graphDataZ[i - 1].x);
				gyr_gain_z[2] = 90 / gyr_gain_z[2];
			}
			else if (m_currentStage == 9)
			{
				for (int i = 1; i < m_graphDataX.size(); i++) gyr_gain_x[0] += integrateData(m_graphDataX[i].y - gyr_off[0], m_graphDataX[i - 1].y - gyr_off[0], m_graphDataX[i].x - m_graphDataX[i - 1].x);
				gyr_gain_x[0] = 90 / gyr_gain_x[0];
			}
		}

		//After utilizing the current gyro data, clear out the data vectors
		//and reset the time stamp
		m_graphDataX.clear();
		m_graphDataY.clear();
		m_graphDataZ.clear();
		m_timeStamp = 0.0f;
	}
	else if (m_currentSensor == MAG_SENSOR)
	{
		//Get the minimum and maximum values for each of the principle axes. Also
		//add data points without timestamps to calibrated data vectors.
		float mag_min[3] = { m_graphDataX[0].y, m_graphDataY[0].y , m_graphDataZ[0].y };
		float mag_max[3] = { m_graphDataX[0].y, m_graphDataY[0].y , m_graphDataZ[0].y };

		mx.clear();
		my.clear();
		mz.clear();

		for (int i = 0; i < m_graphDataX.size(); i++)
		{
			if (m_graphDataX[i].y < mag_min[0]) mag_min[0] = m_graphDataX[i].y;
			if (m_graphDataY[i].y < mag_min[1]) mag_min[1] = m_graphDataY[i].y;
			if (m_graphDataZ[i].y < mag_min[2]) mag_min[2] = m_graphDataZ[i].y;

			if (m_graphDataX[i].y > mag_max[0]) mag_max[0] = m_graphDataX[i].y;
			if (m_graphDataY[i].y > mag_max[1]) mag_max[1] = m_graphDataY[i].y;
			if (m_graphDataZ[i].y > mag_max[2]) mag_max[2] = m_graphDataZ[i].y;

			mx.push_back(m_graphDataX[i].y);
			my.push_back(m_graphDataY[i].y);
			mz.push_back(m_graphDataZ[i].y);
		}

		//Set the offset values
		mag_off[0] = (mag_max[0] + mag_min[0]) / 2.0f;
		mag_off[1] = (mag_max[1] + mag_min[1]) / 2.0f;
		mag_off[2] = (mag_max[2] + mag_min[2]) / 2.0f;

		float og_x_off = mag_off[0], og_y_off = mag_off[1], og_z_off = mag_off[2]; //use these variables to reset data back to original location before applying new cal numbers

		//correct hard iron so data points can be used in best fit algorithm
		for (int i = 0; i < mx.size(); i++)
		{
			mx[i] -= mag_off[0];
			my[i] -= mag_off[1];
			mz[i] -= mag_off[2];
		}

		//convert each data point to spherical coordinates and add to RUV array
		//TODO: This can most likely be taken out
		std::vector<std::vector<double> > RUV;
		for (int i = 0; i < mx.size(); i++)
		{
			std::vector<double> ruv;
			float xi = mx[i];
			float yi = my[i];
			float zi = mz[i];
			ruv.push_back(sqrt(xi * xi + yi * yi + zi * zi)); //r
			ruv.push_back(atan2(yi, xi)); //u
			ruv.push_back(atan2(sqrt(xi * xi + yi * yi), zi)); //v
			RUV.push_back(ruv);
		}

		//figure out best fit ellipse for data set and calculate soft-iron gain from it, also slightly changes location of hard iron when best fit ellipse isn't at origin
		ellipseBestFit(mx, my, mz, RUV, &mag_off[0], &mag_gain[0][0]);

		//put data points back to original location
		for (int i = 0; i < mx.size(); i++)
		{
			mx[i] += og_x_off;
			my[i] += og_y_off;
			mz[i] += og_z_off;
		}
	}
}

void CalibrationMode::initializeModel()
{
	//This method is used to initialize a model of the sensor that gets rendered on screen. This is to 
	//aid the user with the proper orientation and rotations for each portion of the calibration
	std::shared_ptr<Face> sensorTop = std::make_shared<Face>(DirectX::XMFLOAT3(-0.15f, 0.026f, -0.25f), DirectX::XMFLOAT3(0.15f, 0.026f, -0.25f), DirectX::XMFLOAT3(-0.15f, 0.026f, 0.25f));
	std::shared_ptr<Face> sensorLeft = std::make_shared<Face>(DirectX::XMFLOAT3(-0.15f, 0.0f, -0.25f), DirectX::XMFLOAT3(-0.15f, -0.052f, -0.25f), DirectX::XMFLOAT3(-0.15f, 0.0f, 0.25f));
	std::shared_ptr<Face> sensorRight = std::make_shared<Face>(DirectX::XMFLOAT3(0.15f, 0.0f, -0.25f), DirectX::XMFLOAT3(0.15f, -0.052f, -0.25f), DirectX::XMFLOAT3(0.15f, 0.0f, 0.25f));
	std::shared_ptr<Face> sensorFront = std::make_shared<Face>(DirectX::XMFLOAT3(0.15f, -0.026f, 0.25f), DirectX::XMFLOAT3(-0.15f, -0.026f, 0.25f), DirectX::XMFLOAT3(0.15f, 0.026f, 0.25f));
	std::shared_ptr<Face> sensorBack = std::make_shared<Face>(DirectX::XMFLOAT3(-0.15f, -0.026f, -0.25f), DirectX::XMFLOAT3(0.15f, -0.026f, -0.25f), DirectX::XMFLOAT3(-0.15f, 0.026f, -0.25f));
	std::shared_ptr<Face> sensorBottom = std::make_shared<Face>(DirectX::XMFLOAT3(0.15f, -0.026, -0.25f), DirectX::XMFLOAT3(-0.15f, -0.026f, -0.25f), DirectX::XMFLOAT3(0.15f, -0.026f, 0.25f));
	//note* - the left and right face don't seem to be in the right spot to me, but the sensor is rendered correctly so I'm leaving it

	m_volumeElements.push_back(sensorTop);
	m_volumeElements.push_back(sensorLeft);
	m_volumeElements.push_back(sensorRight);
	m_volumeElements.push_back(sensorFront);
	m_volumeElements.push_back(sensorBack);
	m_volumeElements.push_back(sensorBottom);

	//After creating the faces of the sensor, add the appropriate material types for each face.
	//The index of each material type in the vector needs to match the index of the appropriate
	//face in the m_volumeElements vector.
	m_materialTypes.push_back(MaterialType::SENSOR_TOP);
	m_materialTypes.push_back(MaterialType::SENSOR_LONG_SIDE);
	m_materialTypes.push_back(MaterialType::SENSOR_LONG_SIDE);
	m_materialTypes.push_back(MaterialType::SENSOR_SHORT_SIDE);
	m_materialTypes.push_back(MaterialType::SENSOR_SHORT_SIDE);
	m_materialTypes.push_back(MaterialType::SENSOR_BOTTOM);

	//After creating the necessary volume elements and material types, map each volume element 
	//to its given material through the master renderer class (accessed via the ModeScreenHandler)
	m_mode_screen_handler(ModeAction::RendererGetMaterial, nullptr);
}

void CalibrationMode::loadModeMainPage()
{
	//When the calibration mode is first loaded, or when a calibration is complete we should
	//see the same options displayed.

	//Make all the main buttons visible
	m_uiManager.getElement<TextButton>(L"Acc Button")->removeState(UIElementState::Invisible);
	m_uiManager.getElement<TextButton>(L"Gyr Button")->removeState(UIElementState::Invisible);
	m_uiManager.getElement<TextButton>(L"Mag Button")->removeState(UIElementState::Invisible);
	m_uiManager.getElement<TextButton>(L"Axes Button")->removeState(UIElementState::Invisible);
	m_uiManager.getElement<TextButton>(L"Toggle Button")->removeState(UIElementState::Invisible);

	//Make the no and continue/yes buttons invisible
	m_uiManager.getElement<TextButton>(L"Continue Button")->updateText(L"Continue"); //Make sure the continue/yes button says "Continue" and not "Yes"
	m_uiManager.getElement<TextButton>(L"Continue Button")->updateState(UIElementState::Invisible);
	m_uiManager.getElement<TextButton>(L"No Button")->updateState(UIElementState::Invisible);

	//Update Text
	m_uiManager.getElement<TextOverlay>(L"Body Text")->updateText(L"In calibration mode we can manually calculate the offsets and cross-axis gains for the accelerometer, gyroscope"
		L" and magnetometer on the Personal Caddie to get more accurate data. We can also swap data between axes or invert axis data if the Personal Caddie is in a non-standard orientation."
		L" The test type button can be used to toggle between a data calibration or axis calibration for each sensor. The data type button can be used to carry out a standard calibration, or, to use already calibrated data to confirm the strength of the current calibration numbers.");
	m_uiManager.getElement<TextOverlay>(L"Subtitle Text")->updateState(UIElementState::Invisible); //make the sub-title invisible
	m_uiManager.getElement<TextOverlay>(L"Footnote Text")->updateText(L"Press Esc. to return to settings menu."); //reset the footnote message
	
	//Make all graphs invisible and remove all date from them
	m_uiManager.getElement<Graph>(L"Acc Graph")->updateState(UIElementState::Invisible); //make the continue button invisible
	m_uiManager.getElement<Graph>(L"Mag1 Graph")->updateState(UIElementState::Invisible); //make the continue button invisible
	m_uiManager.getElement<Graph>(L"Mag2 Graph")->updateState(UIElementState::Invisible); //make the continue button invisible
	m_uiManager.getElement<Graph>(L"Mag3 Graph")->updateState(UIElementState::Invisible); //make the continue button invisible
	m_uiManager.getElement<Graph>(L"Acc Graph")->removeAllLines(); //clear everything out from the graph when done viewing it
	m_uiManager.getElement<Graph>(L"Mag1 Graph")->removeAllLines(); //clear everything out from the graph when done viewing it
	m_uiManager.getElement<Graph>(L"Mag2 Graph")->removeAllLines(); //clear everything out from the graph when done viewing it
	m_uiManager.getElement<Graph>(L"Mag3 Graph")->removeAllLines(); //clear everything out from the graph when done viewing it

	//Reset all the calibration variables
	initializeCalibrationVariables();
}

void CalibrationMode::displayGraph()
{
	//Although not necessary for the calibration process it's useful to look at a graphical
	//representation of the accumulated data to make sure that things look correct. This
	//method get's called after all calibration data has been gathered.

	if (m_axisCalibration)
	{
		//set the min and max data values for the graph, this value will change depending on which 
		//calibration is being carried out.
		float centerLineLocation, upperLineLocation, lowerLineLocation;

		m_uiManager.getElement<Graph>(L"Acc Graph")->setAxisMaxAndMins({0,  -60.0}, {m_graphDataX.back().x, 60.0});

		//add a few axis lines to the graph
		centerLineLocation = 0.0f;
		upperLineLocation = 50.0f;
		lowerLineLocation = -50.0f;

		m_uiManager.getElement<Graph>(L"Acc Graph")->addDataSet(m_graphDataX, UIColor::Red);
		m_uiManager.getElement<Graph>(L"Acc Graph")->addDataSet(m_graphDataY, UIColor::Blue);
		m_uiManager.getElement<Graph>(L"Acc Graph")->addDataSet(m_graphDataZ, UIColor::Green);

		m_uiManager.getElement<Graph>(L"Acc Graph")->addAxisLine(0, centerLineLocation);
		m_uiManager.getElement<Graph>(L"Acc Graph")->addAxisLine(0, upperLineLocation);
		m_uiManager.getElement<Graph>(L"Acc Graph")->addAxisLine(0, lowerLineLocation);

		std::wstring axisText = std::to_wstring(centerLineLocation);
		m_uiManager.getElement<Graph>(L"Acc Graph")->addAxisLabel(axisText, centerLineLocation);

		axisText = std::to_wstring(upperLineLocation);
		m_uiManager.getElement<Graph>(L"Acc Graph")->addAxisLabel(axisText, upperLineLocation);

		axisText = std::to_wstring(lowerLineLocation);
		m_uiManager.getElement<Graph>(L"Acc Graph")->addAxisLabel(axisText, lowerLineLocation);

		m_uiManager.getElement<Graph>(L"Acc Graph")->removeState(UIElementState::Invisible); //lastly, make the graph visible
	}
	else if (m_currentSensor == ACC_SENSOR)
	{
		//set the min and max data values for the graph, this value will change depending on which 
		//calibration is being carried out.
		float centerLineLocation, upperLineLocation, lowerLineLocation;

		m_uiManager.getElement<Graph>(L"Acc Graph")->setAxisMaxAndMins({ 0,  -12.0 }, { m_graphDataX.back().x, 12.0 });
		//add a few axis lines to the graph
		centerLineLocation = 0.0f; //The average of the highest and lowest data point
		upperLineLocation = GRAVITY; //95% of the highest data point
		lowerLineLocation = -GRAVITY; //95% of the lowest data point

		m_uiManager.getElement<Graph>(L"Acc Graph")->addDataSet(m_graphDataX, UIColor::Red);
		m_uiManager.getElement<Graph>(L"Acc Graph")->addDataSet(m_graphDataY, UIColor::Blue);
		m_uiManager.getElement<Graph>(L"Acc Graph")->addDataSet(m_graphDataZ, UIColor::Green);

		m_uiManager.getElement<Graph>(L"Acc Graph")->addAxisLine(0, centerLineLocation);
		m_uiManager.getElement<Graph>(L"Acc Graph")->addAxisLine(0, upperLineLocation);
		m_uiManager.getElement<Graph>(L"Acc Graph")->addAxisLine(0, lowerLineLocation);

		std::wstring axisText = std::to_wstring(centerLineLocation);
		m_uiManager.getElement<Graph>(L"Acc Graph")->addAxisLabel(axisText, centerLineLocation);

		axisText = std::to_wstring(upperLineLocation);
		m_uiManager.getElement<Graph>(L"Acc Graph")->addAxisLabel(axisText, upperLineLocation);

		axisText = std::to_wstring(lowerLineLocation);
		m_uiManager.getElement<Graph>(L"Acc Graph")->addAxisLabel(axisText, lowerLineLocation);

		m_uiManager.getElement<Graph>(L"Acc Graph")->removeState(UIElementState::Invisible); //lastly, make the graph visible
	}
	else if (m_currentSensor == MAG_SENSOR)
	{
		//The magnetometer calibration has three different graphs associated with it. On in the XY plane, the
		//XZ pland and the YZ plane. We need to create new vectors with the appropriate information.
		std::vector<DirectX::XMFLOAT2> xy, xz, yz, xyc, xzc, yzc;

		for (int i = 0; i < m_graphDataX.size(); i++)
		{
			xy.push_back({ m_graphDataX[i].y, m_graphDataY[i].y });
			xz.push_back({ m_graphDataX[i].y, m_graphDataZ[i].y });
			yz.push_back({ m_graphDataY[i].y, m_graphDataZ[i].y });
		}

		//We also add the calibrated values for a comparison
		for (int i = 0; i < mx.size(); i++)
		{
			mx[i] = (mag_gain[0][0] * (m_graphDataX[i].y - mag_off[0])) + (mag_gain[0][1] * (m_graphDataY[i].y - mag_off[1])) + (mag_gain[0][2] * (m_graphDataZ[i].y - mag_off[2]));
			my[i] = (mag_gain[1][0] * (m_graphDataX[i].y - mag_off[0])) + (mag_gain[1][1] * (m_graphDataY[i].y - mag_off[1])) + (mag_gain[1][2] * (m_graphDataZ[i].y - mag_off[2]));
			mz[i] = (mag_gain[2][0] * (m_graphDataX[i].y - mag_off[0])) + (mag_gain[2][1] * (m_graphDataY[i].y - mag_off[1])) + (mag_gain[2][2] * (m_graphDataZ[i].y - mag_off[2]));

			xyc.push_back({ mx[i], my[i] });
			xzc.push_back({ mx[i], mz[i] });
			yzc.push_back({ my[i], mz[i] });
		}

		m_uiManager.getElement<Graph>(L"Mag1 Graph")->setAxisMaxAndMins({ -30.0f,  -30.0f }, { 30.0f, 30.0f });
		m_uiManager.getElement<Graph>(L"Mag1 Graph")->addDataSet(xy, UIColor::Red);
		m_uiManager.getElement<Graph>(L"Mag1 Graph")->addDataSet(xyc, UIColor::Green);
		m_uiManager.getElement<Graph>(L"Mag1 Graph")->addAxisLine(0, 0);
		m_uiManager.getElement<Graph>(L"Mag1 Graph")->addAxisLine(1, 0);
		m_uiManager.getElement<Graph>(L"Mag1 Graph")->addAxisLine(0, -50);
		m_uiManager.getElement<Graph>(L"Mag1 Graph")->addAxisLine(1, -50);
		
		m_uiManager.getElement<Graph>(L"Mag2 Graph")->setAxisMaxAndMins({ -125.0f,  -125.0f }, { 125.0f, 125.0f });
		m_uiManager.getElement<Graph>(L"Mag2 Graph")->addDataSet(xz, UIColor::Blue);
		m_uiManager.getElement<Graph>(L"Mag2 Graph")->addDataSet(xzc, UIColor::Green);
		m_uiManager.getElement<Graph>(L"Mag2 Graph")->addAxisLine(0, 0);
		m_uiManager.getElement<Graph>(L"Mag2 Graph")->addAxisLine(1, 0);

		m_uiManager.getElement<Graph>(L"Mag3 Graph")->setAxisMaxAndMins({ -125.0f,  -125.0f }, { 125.0f, 125.0f });
		m_uiManager.getElement<Graph>(L"Mag3 Graph")->addDataSet(yz, UIColor::Yellow);
		m_uiManager.getElement<Graph>(L"Mag3 Graph")->addDataSet(yzc, UIColor::Green);
		m_uiManager.getElement<Graph>(L"Mag3 Graph")->addAxisLine(0, 0);
		m_uiManager.getElement<Graph>(L"Mag3 Graph")->addAxisLine(1, 0);

		//TODO: add axis lines

		//lastly, make the graphs visible
		m_uiManager.getElement<Graph>(L"Mag1 Graph")->removeState(UIElementState::Invisible);
		m_uiManager.getElement<Graph>(L"Mag2 Graph")->removeState(UIElementState::Invisible);
		m_uiManager.getElement<Graph>(L"Mag3 Graph")->removeState(UIElementState::Invisible);
	}
}

std::pair<float*, float**> CalibrationMode::getCalibrationResults()
{
	//If we like the calibration results, we use this method to get them to the IMU class
	//and update the external calibration files for future use

	if (m_currentSensor == ACC_SENSOR) return { acc_off, acc_gain};
	else if (m_currentSensor == GYR_SENSOR) return { gyr_off, gyr_gain };
	else if (m_currentSensor == MAG_SENSOR) return { mag_off, mag_gain };
}

std::vector<int> CalibrationMode::getNewAxesOrientations()
{
	//If we like the axis calibration results, we use this method to send them to
	//the IMU class and update the external axis orientation files for future use
	return {acc_axis_swap[0], acc_axis_swap[1], acc_axis_swap[2], acc_axis_polarity[0], acc_axis_polarity[1], acc_axis_polarity[2],
	gyr_axis_swap[0], gyr_axis_swap[1], gyr_axis_swap[2], gyr_axis_polarity[0], gyr_axis_polarity[1], gyr_axis_polarity[2],
	mag_axis_swap[0], mag_axis_swap[1], mag_axis_swap[2], mag_axis_polarity[0], mag_axis_polarity[1], mag_axis_polarity[2]};
}

void CalibrationMode::handlePersonalCaddieConnectionEvent(bool connectionStatus)
{
	if (!connectionStatus)
	{

	}
	else
	{

	}
}

void CalibrationMode::invertAccMatrix()
{
	//inverts the calculated Acc Gain matrix
	float determinant = acc_gain[0][0] * (acc_gain[1][1] * acc_gain[2][2] - acc_gain[1][2] * acc_gain[2][1]) - acc_gain[0][1] * (acc_gain[1][0] * acc_gain[2][2] - acc_gain[1][2] * acc_gain[2][0]) + acc_gain[0][2] * (acc_gain[1][0] * acc_gain[2][1] - acc_gain[1][1] * acc_gain[2][0]);
	for (int i = 0; i < 3; i++)
	{
		//Transpose the original matrix
		for (int j = i + 1; j < 3; j++)
		{
			float temp = acc_gain[i][j];
			acc_gain[i][j] = acc_gain[j][i];
			acc_gain[j][i] = temp;
		}
	}

	//Get determinants of 2x2 minor matrices
	float new_nums[3][3];
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			int row1, row2, col1, col2;
			if (i == 0)
			{
				row1 = 1; row2 = 2;
			}
			if (i == 1)
			{
				row1 = 0; row2 = 2;
			}
			if (i == 2)
			{
				row1 = 0; row2 = 1;
			}
			if (j == 0)
			{
				col1 = 1; col2 = 2;
			}
			if (j == 1)
			{
				col1 = 0; col2 = 2;
			}
			if (j == 2)
			{
				col1 = 0; col2 = 1;
			}
			float num = acc_gain[row1][col1] * acc_gain[row2][col2] - acc_gain[row1][col2] * acc_gain[row2][col1];
			if ((i + j) % 2 == 1) num *= -1;
			new_nums[i][j] = num;
		}
	}

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			acc_gain[i][j] = new_nums[i][j] / determinant;
		}
	}
}