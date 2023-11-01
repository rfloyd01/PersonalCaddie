#include "pch.h"
#include "CalibrationMode.h"

CalibrationMode::CalibrationMode()
{
	//set a light gray background color for the mode
	m_backgroundColor = UIColor::DarkGray;
}

uint32_t CalibrationMode::initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState)
{
	//Create UI Elements on the page
	std::wstring introMessage = L"In calibration mode we can manually calculate the offsets and cross-axis gains for the accelerometer, gyroscope and magnetometer on the Personal Caddie to get more accurate data. Select one of the sensors using the buttons below to begin the calibration process. Before carrying out a calibration it's important to make sure that low and high pass filters for each sensor are turned off as they can effect calibration results (these can be deactivated in the IMU settings menu).";
	std::wstring accButtonText = L"Accelerometer Calibration";
	std::wstring gyrButtonText = L"Gyroscope Calibration";
	std::wstring magButtonText = L"Magnetometer Calibration";
	std::wstring continueButtonText = L"Continue";
	std::wstring noButtonText = L"No";
	std::wstring toggleButtonText = L"Use Calibrated Data";

	TextButton accButton(windowSize, { 0.16, 0.65 }, { 0.14, 0.1 }, accButtonText);
	TextButton gyrButton(windowSize, { 0.5, 0.65 }, { 0.14, 0.1 }, gyrButtonText);
	TextButton magButton(windowSize, { 0.83, 0.65 }, { 0.14, 0.1 }, magButtonText);
	TextButton continueButton(windowSize, { 0.16, 0.85 }, { 0.14, 0.1 }, continueButtonText);
	TextButton noButton(windowSize, { 0.32, 0.85 }, { 0.14, 0.1 }, noButtonText);
	TextButton toggleButton(windowSize, { 0.5, 0.85 }, { 0.14, 0.1 }, toggleButtonText);
	continueButton.setState(continueButton.getState() | UIElementState::Invisible); //this button is invisible to start off
	noButton.setState(noButton.getState() | UIElementState::Invisible); //this button is invisible to start off

	//The body and sub-title text overlays will change depending on the current mode state so they
	//aren't declared in the initializeTextOverlay() method
	TextOverlay body(windowSize, { UIConstants::BodyTextLocationX, UIConstants::BodyTextLocationY }, { UIConstants::BodyTextSizeX, UIConstants::BodyTextSizeY },
		introMessage, 0.75f * UIConstants::BodyTextPointSize, { UIColor::White }, { 0,  (unsigned int)introMessage.length() }, UITextJustification::UpperLeft);
	TextOverlay subTitle(windowSize, { UIConstants::SubTitleTextLocationX, UIConstants::SubTitleTextLocationY - 0.025f }, { UIConstants::SubTitleTextSizeX, UIConstants::SubTitleTextSizeY },
		L"Fill", 1.05f * UIConstants::SubTitleTextPointSize, {UIColor::White}, {0, 4}, UITextJustification::UpperCenter);
	subTitle.setState(subTitle.getState() | UIElementState::Invisible); ///the sub-title starts off invisible

	//Create a few graph ui elements, they will stay invisible until after individual calibrations are complete
	Graph accGraph(windowSize, { 0.7, 0.65 }, { 0.5, 0.5 });
	Graph magGraph1(windowSize, { 0.55, 0.8 }, { 0.25, 0.25 }, false);
	Graph magGraph2(windowSize, { 0.85, 0.5 }, { 0.25, 0.25 }, false);
	Graph magGraph3(windowSize, { 0.85, 0.8 }, { 0.25, 0.25 }, false);
	accGraph.setState(accGraph.getState() | UIElementState::Invisible);
	magGraph1.setState(magGraph1.getState() | UIElementState::Invisible);
	magGraph2.setState(magGraph2.getState() | UIElementState::Invisible);
	magGraph3.setState(magGraph3.getState() | UIElementState::Invisible);
	
	m_uiElements.push_back(std::make_shared<TextButton>(accButton));
	m_uiElements.push_back(std::make_shared<TextButton>(gyrButton));
	m_uiElements.push_back(std::make_shared<TextButton>(magButton));
	m_uiElements.push_back(std::make_shared<TextButton>(continueButton));
	m_uiElements.push_back(std::make_shared<TextOverlay>(body));
	m_uiElements.push_back(std::make_shared<TextOverlay>(subTitle));
	m_uiElements.push_back(std::make_shared<Graph>(accGraph));
	m_uiElements.push_back(std::make_shared<TextButton>(noButton));
	m_uiElements.push_back(std::make_shared<TextButton>(toggleButton));
	m_uiElements.push_back(std::make_shared<Graph>(magGraph1));
	m_uiElements.push_back(std::make_shared<Graph>(magGraph2));
	m_uiElements.push_back(std::make_shared<Graph>(magGraph3));

	m_state = initialState;
	m_useCalibratedData = false;

	//Initialize calibration variables
	initializeCalibrationVariables();

	//Initialize any unchanging text
	initializeTextOverlay(windowSize);

	//Start in the active state
	return ModeState::CanTransfer;
}

void CalibrationMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	for (int i = 0; i < m_uiElements.size(); i++) m_uiElements[i] = nullptr;
	m_uiElements.clear();
}

void CalibrationMode::initializeTextOverlay(winrt::Windows::Foundation::Size windowSize)
{
	//Title information
	std::wstring title_message = L"Calibration Mode";
	TextOverlay title(windowSize, { UIConstants::TitleTextLocationX, UIConstants::TitleTextLocationY }, { UIConstants::TitleTextSizeX, UIConstants::TitleTextSizeY },
		title_message, UIConstants::TitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)title_message.length() }, UITextJustification::CenterCenter);
	m_uiElements.push_back(std::make_shared<TextOverlay>(title));

	//Footnote information
	std::wstring footnote_message = L"Press Esc. to return to settings menu.";
	TextOverlay footnote(windowSize, { UIConstants::FootNoteTextLocationX, UIConstants::FootNoteTextLocationY }, { UIConstants::FootNoteTextSizeX, UIConstants::FootNoteTextSizeY },
		footnote_message, UIConstants::FootNoteTextPointSize, { UIColor::White }, { 0,  (unsigned int)footnote_message.length() }, UITextJustification::LowerRight);
	m_uiElements.push_back(std::make_shared<TextOverlay>(footnote));
}

void CalibrationMode::initializeCalibrationVariables()
{
	m_currentStage = 0;
	m_stageSet = false;
	m_timeStamp = 0.0f;
	avg_count = 0;
	accept_cal = true; //by default we choose to accept the calibration results
	data_time_stamps.clear();
	
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
	}
}

void CalibrationMode::startDataCapture()
{
	//We can't start data capture until the Personal Caddie has been placed into active mode, which happens asynchronously.
	//Because of that, this method is called from the ModeScreen class once the Personal Caddie has been placed into 
	//sensor active mode and is ready to start recording data. Furthermore, all IMU sensors have a slight wake up time
	//where data they generate will be "junk". We can also use this method to wait for a brief moment after turning on
	//the sensors to actually record data.
	if (m_state & CalibrationModeState::READY_TO_RECORD)
	{
		//Wait a moment to let the sensor warm up
		auto start = std::chrono::steady_clock::now();
		while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() < 750) {}

		m_state ^= (CalibrationModeState::READY_TO_RECORD | CalibrationModeState::RECORDING_DATA);
		data_timer = std::chrono::steady_clock::now();
	}
}

void CalibrationMode::stopDataCapture()
{
	//Like with the startDataCapture() method, this method is used byt the mode screen class to let us know
	//when the Personal Caddie has been taken out of sensor active mode
	if (m_state & CalibrationModeState::STOP_RECORD)
	{
		m_state ^= CalibrationModeState::STOP_RECORD;
		
		if (m_state & CalibrationModeState::ACCELEROMETER)
		{
			//At the end of each accelerometer calibration stage we need to take an average
			//of that stage's data
			int current_acc_stage = m_currentStage / 2 - 1;
			acc_cal[0][current_acc_stage] /= avg_count;
			acc_cal[1][current_acc_stage] /= avg_count;
			acc_cal[2][current_acc_stage] /= avg_count;
			avg_count = 0;
		}
	}
}

void CalibrationMode::updateComplete()
{
	//When we've successfully updated the calibration file in memory we use
	//this method to remove the update_cal_numbers flag from the current state.
	//This is called from a separate thread which is why this is necessary.
	if (m_state & CalibrationModeState::UPDATE_CAL_NUMBERS) m_state ^= CalibrationModeState::UPDATE_CAL_NUMBERS;
	if (m_state & ModeState::Active) m_state ^= ModeState::Active;

	m_state &= 0xFFFFFFF0; //This will remove the active flag as well as the current sensor flag
	initializeCalibrationVariables(); //reset calibration variables to their default values
}

uint32_t CalibrationMode::handleUIElementStateChange(int i)
{
	if (i <= 2)
	{
		//one of the sensor calibration buttons was clicked, update the current mode state
		//and make the buttons invisible
		m_state |= (2 << i);
		m_uiElements[0]->setState(m_uiElements[0]->getState() | UIElementState::Invisible);
		m_uiElements[1]->setState(m_uiElements[1]->getState() | UIElementState::Invisible);
		m_uiElements[2]->setState(m_uiElements[2]->getState() | UIElementState::Invisible);

		m_uiElements[3]->setState(m_uiElements[3]->getState() ^ UIElementState::Invisible); //make the continue button visible
		m_uiElements[5]->setState(m_uiElements[5]->getState() ^ UIElementState::Invisible); //make the sub-title visible

		((TextOverlay*)m_uiElements[5].get())->updateText(((TextButton*)m_uiElements[i].get())->getText()); //update the sub-title text

		m_uiElements[8]->setState(m_uiElements[8]->getState() | UIElementState::Invisible); //make the data toggle switch invisible

		switch (i)
		{
		case 0:
			((TextOverlay*)m_uiElements[4].get())->updateText(L"A 6-point tumble calibration will be performed on the accelerometer. Press the continue button when ready to begin and then follow the on-screen instructions.");
			break;
		case 1:
			((TextOverlay*)m_uiElements[4].get())->updateText(L"Two tests will be performed to calibrate the gyroscope. The first test is easy and just involves keeping the gyroscope still. The second test is carried out in three stages. In each of these stages we rotate the gyroscope by 90 degrees about one of its axes.");
			break;
		case 2:
			((TextOverlay*)m_uiElements[4].get())->updateText(L"To calibrate the magnetometer we need to take sources of interference into account. These include both the hard and soft iron deposits (large scale deviations in magnetic field, like large iron deposits in the ground, and small scale deviations in magnetic field, such as from nearby metal like a golf club shaft).");
			break;
		}

		m_state |= ModeState::Active;
	}
	else if (i == 3)
	{
		//The continue button was clicked, this has the effect of advancing the current stage by 1
		advanceToNextStage();
	}
	else if (i == 7)
	{
		//The no button was clicked, set the accept_cal bool to false and advance to the next stage
		accept_cal = false;
		advanceToNextStage();
	}
	else if (i == 8)
	{
		//Toggles whether or not we use raw or calibrated data during the calibration. The point of using
		//already calibrated data is to benchmark how good the current calibration numbers are.
		if (!m_useCalibratedData) m_uiElements[8]->getText()->message = L"Use Raw Data";
		else m_uiElements[8]->getText()->message = L"Use Calibrated Data";
		m_useCalibratedData = !m_useCalibratedData;
	}

	return m_state;
}

void CalibrationMode::advanceToNextStage()
{
	//We call this method to move to the next stage of the current calibration
	m_currentStage++;
	m_stageSet = false;
	update();
}

void CalibrationMode::update()
{
	//when in active mode it means that the device watcher is running
	if (m_state & CalibrationModeState::ACCELEROMETER) accelerometerCalibration();
	else if (m_state & CalibrationModeState::GYROSCOPE) gyroscopeCalibration();
	else if (m_state & CalibrationModeState::MAGNETOMETER) magnetometerCalibration();
}

void CalibrationMode::addData(std::vector<std::vector<std::vector<float> > > const& sensorData, float sensorODR, float timeStamp, int totalSamples)
{
	//This method gets called asynchronously whenever new data is ready. We simply iterate over the appropriate
	//data (given the current calibration we're doing) and add it to the internal data vector.
	m_sensorODR = sensorODR; //this value won't change, I just couldn't think of anywhere better to set it for right now

	if (m_state & CalibrationModeState::RECORDING_DATA) //only add date if we're actively recording
	{
		//m_timeStamp = (float)std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - data_timer).count() / 1000000000.0;
		data_time_stamps.push_back(m_timeStamp);
		std::wstring time_debug = L"Add data called at " + std::to_wstring(m_timeStamp) + L"\n";
		OutputDebugString(&time_debug[0]);

		int calibrationType;
		if (m_state & CalibrationModeState::ACCELEROMETER)
		{
			if (m_useCalibratedData) calibrationType = 0;
			else calibrationType = raw_acceleration;
		}
		else if (m_state & CalibrationModeState::GYROSCOPE)
		{
			if (m_useCalibratedData) calibrationType = 1;
		    else calibrationType = raw_rotation;
		}
		else if (m_state & CalibrationModeState::MAGNETOMETER)
		{
			if (m_useCalibratedData) calibrationType = 2;
		    else calibrationType = raw_magnetic;
		}

		float timeIncrement = 1.0f / sensorODR;
		int current_stage = m_currentStage / 2 - 1;

		for (int i = 0; i < totalSamples; i++)
		{
			//x, y and z data gets added to the overall data vectors
			m_graphDataX.push_back({ timeStamp + i * timeIncrement, sensorData[calibrationType][0][i] });
			m_graphDataY.push_back({ timeStamp + i * timeIncrement, sensorData[calibrationType][1][i] });
			m_graphDataZ.push_back({ timeStamp + i * timeIncrement, sensorData[calibrationType][2][i] });

			if (calibrationType == raw_acceleration || calibrationType == 0)
			{
				//add x, y and z data to accerlometer matrix
				acc_cal[0][current_stage] += sensorData[calibrationType][0][i];
				acc_cal[1][current_stage] += sensorData[calibrationType][1][i];
				acc_cal[2][current_stage] += sensorData[calibrationType][2][i];
				avg_count++;
			}

			//m_timeStamp += timeIncrement;
		}
	}
}

void CalibrationMode::accelerometerCalibration()
{
	if (m_state & CalibrationModeState::RECORDING_DATA)
	{
		//We're actively recording data from the sensor, check to see if the data timer has expired,
		//if not then no need to do anything. If the timer has expired, we leave the recording state
		//and advnace to the next stage of the calibration.
		data_timer_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - data_timer).count();
		if (data_timer_elapsed >= data_timer_duration)
		{
			m_state ^= (CalibrationModeState::RECORDING_DATA | CalibrationModeState::STOP_RECORD);
			advanceToNextStage();
			return; //since advance to next stage is called above we leave this method after it returns
		}
	}

	switch (m_currentStage)
	{
	case 1:
	{
		if (!m_stageSet)
		{
			((TextOverlay*)m_uiElements[4].get())->updateText(L"Lay the sensor flat on the table with the +Y axis pointing upwards like shown in the image. Leave the sensor stationary like this for 5 seconds as it collects data. Press the continue button when ready.");
			data_timer_duration = 5000; //the acceleromter needs 5 seconds of data at each stage
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
			((TextOverlay*)m_uiElements[4].get())->updateText(L"Data collection complete. Rotate the sensor so that the +X axis is pointing down as shown in the image. When ready, press the continue button. Make sure to keep the sensor as still as possible until the timer runs out.");
			m_stageSet = true;
		}
		break;
	}
	case 5:
	{
		if (!m_stageSet)
		{
			((TextOverlay*)m_uiElements[4].get())->updateText(L"Data collection complete. Rotate sensor 90 degrees so that the positive Z-axis is pointing up as shown in the image. When ready, press the continue button. Make sure to keep the sensor as still as possible until the timer runs out.");
			m_stageSet = true;
		}
		break;
	}
	case 7:
	{
		if (!m_stageSet)
		{
			((TextOverlay*)m_uiElements[4].get())->updateText(L"Data collection complete. Rotate sensor 90 degrees so that the positive X-axis is pointing up as shown in the image. When ready, press the continue button. Make sure to keep the sensor as still as possible until the timer runs out.");
			m_stageSet = true;
		}
		break;
	}
	case 9:
	{
		if (!m_stageSet)
		{
			((TextOverlay*)m_uiElements[4].get())->updateText(L"Data collection complete. Rotate sensor 90 degrees so that the positive Z-axis is pointing down as shown in the image. When ready, press the continue button. Make sure to keep the sensor as still as possible until the timer runs out.");
			m_stageSet = true;
		}
		break;
	}
	case 11:
	{
		if (!m_stageSet)
		{
			((TextOverlay*)m_uiElements[4].get())->updateText(L"Data collection complete. Rotate sensor 90 degrees so that the positive Y-axis is pointing down as shown in the image. When ready, press the continue button. Make sure to keep the sensor as still as possible until the timer runs out.");
			m_stageSet = true;
		}
		break;
	}
	case 13:
	{
		if (!m_stageSet)
		{
			if (!(m_state & CalibrationModeState::STOP_RECORD)) //don't advance until recording has been asynchronously turned off
			{
				//The tumble calibration is complete, use the recorded data to calculate the offset and gain values for the accelerometer
				calculateCalNumbers();
				std::wstring completionText = L"Calibration complete, see results below. Press continue to return to main calibration screen.\n\n\n"
					L"Offset = [" + std::to_wstring(acc_off[0]) + L", " + std::to_wstring(acc_off[1]) + L", " + std::to_wstring(acc_off[2]) + L"]\n\n"
					L"              [" + std::to_wstring(acc_gain[0][0]) + L", " + std::to_wstring(acc_gain[0][1]) + L", " + std::to_wstring(acc_gain[0][2]) + L"]\n"
					L"Gains  =  [" + std::to_wstring(acc_gain[1][0]) + L", " + std::to_wstring(acc_gain[1][1]) + L", " + std::to_wstring(acc_gain[1][2]) + L"]\n"
					L"               [" + std::to_wstring(acc_gain[2][0]) + L", " + std::to_wstring(acc_gain[2][1]) + L", " + std::to_wstring(acc_gain[2][2]) + L"]";
				((TextOverlay*)m_uiElements[4].get())->updateText(completionText);
				displayGraph();

				//momentarily change the text of the continue button to say "yes", also make the "no" button visible
				m_uiElements[3]->getText()->message = L"Yes";
				m_uiElements[7]->setState(m_uiElements[7]->getState() ^ UIElementState::Invisible);

				m_stageSet = true;
			}
		}
		break;
	}
	case 14:
	{
		if (!m_stageSet)
		{
			//If the results have been accepted we add the update_cal_numbers state to alert the mode screen
		    //that it needs to physically update the calibration numbers.
			if (accept_cal)
			{
				m_state |= CalibrationModeState::UPDATE_CAL_NUMBERS;
			}

			m_uiElements[0]->setState(m_uiElements[0]->getState() ^ UIElementState::Invisible);
			m_uiElements[1]->setState(m_uiElements[1]->getState() ^ UIElementState::Invisible);
			m_uiElements[2]->setState(m_uiElements[2]->getState() ^ UIElementState::Invisible);

			//Update the body text
			((TextOverlay*)m_uiElements[4].get())->updateText(L"In calibration mode we can manually calculate the offsets and cross-axis gains for the accelerometer, gyroscope and magnetometer on the Personal Caddie to get more accurate data. Select one of the sensors using the buttons below to begin the calibration process.");

			m_uiElements[3]->getText()->message = L"Continue"; //change the text back to "Continue" for the continue button
			m_uiElements[3]->setState(m_uiElements[3]->getState() | UIElementState::Invisible); //make the continue button invisible
			m_uiElements[5]->setState(m_uiElements[5]->getState() | UIElementState::Invisible); //make the sub-title invisible
			m_uiElements[6]->setState(m_uiElements[6]->getState() | UIElementState::Invisible); //make the graph invisible
			m_uiElements[7]->setState(m_uiElements[7]->getState() | UIElementState::Invisible); //make the no button invisible
			m_uiElements[8]->setState(m_uiElements[8]->getState() ^ UIElementState::Invisible); //make the data toggle switch visible

			((Graph*)m_uiElements[6].get())->removeAllLines(); //clear everything out from the graph when done viewing it

			m_stageSet = true;
		}

		if (!accept_cal) updateComplete();
	}
	}
}

void CalibrationMode::gyroscopeCalibration()
{
	if (m_state & CalibrationModeState::RECORDING_DATA)
	{
		//We're actively recording data from the sensor, check to see if the data timer has expired,
		//if not then no need to do anything. If the timer has expired, we leave the recording state
		//and advnace to the next stage of the calibration.
		data_timer_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - data_timer).count();
		if (data_timer_elapsed >= data_timer_duration)
		{
			m_state ^= (CalibrationModeState::RECORDING_DATA | CalibrationModeState::STOP_RECORD);
			advanceToNextStage();
			return; //since advance to next stage is called above we leave this method after it returns
		}
	}

	switch (m_currentStage)
	{
	case 1:
	{
		if (!m_stageSet)
		{
			((TextOverlay*)m_uiElements[4].get())->updateText(L"Lay the gyroscope flat on the table for 5 seconds while the axes offsets are calculated. Click continue when ready.");
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
			((TextOverlay*)m_uiElements[4].get())->updateText(L"Data collection complete. Next, the gryoscope axis gain needs to be calculated. This is done in three stages, one for each axis. For this first stage we rotate the sensor counter-clockwise about the y-axis. Slowly rotate the sensor as close to 90 degrees as possible over the course of 5 seconds. Press continue when ready.");
			m_stageSet = true;
		}
		break;
	}
	case 5:
	{
		if (!m_stageSet)
		{
			calculateCalNumbers(); //Calculate the gyroscope y-axis gain
			((TextOverlay*)m_uiElements[4].get())->updateText(L"Data collection complete. We know slowly rotate the sensor 90 degrees counter-clockwise about the Z-axis over the course of 5 seconds. Press continue when ready.");
			m_stageSet = true;
		}
		break;
	}
	case 7:
	{
		if (!m_stageSet)
		{
			calculateCalNumbers(); //Calculate the gyroscopezy-axis gain
			((TextOverlay*)m_uiElements[4].get())->updateText(L"Data collection complete. We know slowly rotate the sensor 90 degrees counter-clockwise about the X-axis over the course of 5 seconds. Press continue when ready.");
			m_stageSet = true;
		}
		break;
	}
	case 9:
	{
		if (!m_stageSet)
		{
			if (!(m_state & CalibrationModeState::STOP_RECORD)) //don't advance until recording has been asynchronously turned off
			{
				calculateCalNumbers(); //Calculate the gyroscope x-axis gain
				std::wstring completionText = L"Calibration complete, see results below. Press continue to return to main calibration screen.\n\n\n"
					L"Offset = [" + std::to_wstring(gyr_off[0]) + L", " + std::to_wstring(gyr_off[1]) + L", " + std::to_wstring(gyr_off[2]) + L"]\n\n"
					L"              [" + std::to_wstring(gyr_gain[0][0]) + L"]\n"
					L"Gains  =  [" + std::to_wstring(gyr_gain[1][1]) + L"]\n"
					L"               [" + std::to_wstring(gyr_gain[2][2]) + L"]";
				((TextOverlay*)m_uiElements[4].get())->updateText(completionText);
				displayGraph();

				//momentarily change the text of the continue button to say "yes", also make the "no" button visible
				m_uiElements[3]->getText()->message = L"Yes";
				m_uiElements[7]->setState(m_uiElements[7]->getState() ^ UIElementState::Invisible);

				m_stageSet = true;
			}
		}
		break;
	}
	case 10:
	{
		if (!m_stageSet)
		{
			//If the results have been accepted we add the update_cal_numbers state to alert the mode screen
			//that it needs to physically update the calibration numbers.
			if (accept_cal)
			{
				m_state |= CalibrationModeState::UPDATE_CAL_NUMBERS;
			}

			m_uiElements[0]->setState(m_uiElements[0]->getState() ^ UIElementState::Invisible);
			m_uiElements[1]->setState(m_uiElements[1]->getState() ^ UIElementState::Invisible);
			m_uiElements[2]->setState(m_uiElements[2]->getState() ^ UIElementState::Invisible);

			//Update the body text
			((TextOverlay*)m_uiElements[4].get())->updateText(L"In calibration mode we can manually calculate the offsets and cross-axis gains for the accelerometer, gyroscope and magnetometer on the Personal Caddie to get more accurate data. Select one of the sensors using the buttons below to begin the calibration process.");

			m_uiElements[3]->getText()->message = L"Continue"; //change the text back to "Continue" for the continue button
			m_uiElements[3]->setState(m_uiElements[3]->getState() | UIElementState::Invisible); //make the continue button invisible
			m_uiElements[5]->setState(m_uiElements[5]->getState() | UIElementState::Invisible); //make the sub-title invisible
			m_uiElements[6]->setState(m_uiElements[6]->getState() | UIElementState::Invisible); //make the graph invisible
			m_uiElements[7]->setState(m_uiElements[7]->getState() | UIElementState::Invisible); //make the no button invisible
			m_uiElements[8]->setState(m_uiElements[8]->getState() ^ UIElementState::Invisible); //make the data toggle switch visible

			m_stageSet = true;
		}

		if (!accept_cal) updateComplete();
	}
	}
}

void CalibrationMode::magnetometerCalibration()
{
	if (m_state & CalibrationModeState::RECORDING_DATA)
	{
		//We're actively recording data from the sensor, check to see if the data timer has expired,
		//if not then no need to do anything. If the timer has expired, we leave the recording state
		//and advnace to the next stage of the calibration.
		data_timer_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - data_timer).count();
		if (data_timer_elapsed >= data_timer_duration)
		{
			m_state ^= (CalibrationModeState::RECORDING_DATA | CalibrationModeState::STOP_RECORD);
			advanceToNextStage();
			return; //since advance to next stage is called above we leave this method after it returns
		}
	}

	switch (m_currentStage)
	{
	case 1:
	{
		if (!m_stageSet)
		{
			((TextOverlay*)m_uiElements[4].get())->updateText(L"Take the sensor in your hand and rotate it along all three axes in figure-8 patterns for 20 seconds. Imaging the sensor is inside a sphere and you're trying to make the front of the sensor point at as many locations in the sphere as possible. Press the continue button when ready.");
			data_timer_duration = 20000; //the acceleromter needs 5 seconds of data at each stage
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
			((TextOverlay*)m_uiElements[4].get())->updateText(completionText);
			displayGraph();

			//momentarily change the text of the continue button to say "yes", also make the "no" button visible
			m_uiElements[3]->getText()->message = L"Yes";
			m_uiElements[7]->setState(m_uiElements[7]->getState() ^ UIElementState::Invisible);

			m_stageSet = true;
		}
		break;
	}
	case 4:
	{
		if (!m_stageSet)
		{
			//If the results have been accepted we add the update_cal_numbers state to alert the mode screen
			//that it needs to physically update the calibration numbers.
			if (accept_cal)
			{
				m_state |= CalibrationModeState::UPDATE_CAL_NUMBERS;
			}

			m_uiElements[0]->setState(m_uiElements[0]->getState() ^ UIElementState::Invisible);
			m_uiElements[1]->setState(m_uiElements[1]->getState() ^ UIElementState::Invisible);
			m_uiElements[2]->setState(m_uiElements[2]->getState() ^ UIElementState::Invisible);

			//Update the body text
			((TextOverlay*)m_uiElements[4].get())->updateText(L"In calibration mode we can manually calculate the offsets and cross-axis gains for the accelerometer, gyroscope and magnetometer on the Personal Caddie to get more accurate data. Select one of the sensors using the buttons below to begin the calibration process.");

			m_uiElements[3]->getText()->message = L"Continue"; //change the text back to "Continue" for the continue button
			m_uiElements[3]->setState(m_uiElements[3]->getState() | UIElementState::Invisible); //make the continue button invisible
			m_uiElements[5]->setState(m_uiElements[5]->getState() | UIElementState::Invisible); //make the sub-title invisible
			m_uiElements[7]->setState(m_uiElements[7]->getState() | UIElementState::Invisible); //make the no button invisible
			m_uiElements[8]->setState(m_uiElements[8]->getState() ^ UIElementState::Invisible); //make the data toggle switch visible
			m_uiElements[9]->setState(m_uiElements[9]->getState() | UIElementState::Invisible); //make the graph invisible
			m_uiElements[10]->setState(m_uiElements[10]->getState() | UIElementState::Invisible); //make the graph invisible
			m_uiElements[11]->setState(m_uiElements[11]->getState() | UIElementState::Invisible); //make the graph invisible

			//Clear everything out from the graphs when done viewing them
			((Graph*)m_uiElements[9].get())->removeAllLines();
			((Graph*)m_uiElements[10].get())->removeAllLines();
			((Graph*)m_uiElements[11].get())->removeAllLines();

			m_stageSet = true;
		}

		if (!accept_cal) updateComplete();
	}
	}
}

void CalibrationMode::calculateCalNumbers()
{
	if (m_state & CalibrationModeState::ACCELEROMETER)
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
		acc_gain[0][0] = (acc_cal[0][3] - acc_cal[0][1]) / (2 * GRAVITY);
		acc_gain[1][0] = (acc_cal[1][3] - acc_cal[1][1]) / (2 * GRAVITY);
		acc_gain[2][0] = (acc_cal[2][3] - acc_cal[2][1]) / (2 * GRAVITY);

		acc_gain[0][1] = (acc_cal[0][0] - acc_cal[0][5]) / (2 * GRAVITY);
		acc_gain[1][1] = (acc_cal[1][0] - acc_cal[1][5]) / (2 * GRAVITY);
		acc_gain[2][1] = (acc_cal[2][0] - acc_cal[2][5]) / (2 * GRAVITY);

		acc_gain[0][2] = (acc_cal[0][2] - acc_cal[0][4]) / (2 * GRAVITY);
		acc_gain[1][2] = (acc_cal[1][2] - acc_cal[1][4]) / (2 * GRAVITY);
		acc_gain[2][2] = (acc_cal[2][2] - acc_cal[2][4]) / (2 * GRAVITY);

		invertAccMatrix(); //invert the matrix

		//Calcs when using only a single reading for gain
		/*acc_gain[0][0] = (acc_cal[0][3] - acc_off[0]) / GRAVITY;
		acc_gain[0][1] = (acc_cal[0][0] - acc_off[0]) / GRAVITY;
		acc_gain[0][2] = (acc_cal[0][2] - acc_off[0]) / GRAVITY;

		acc_gain[1][0] = (acc_cal[1][3] - acc_off[1]) / GRAVITY;
		acc_gain[1][1] = (acc_cal[1][0] - acc_off[1]) / GRAVITY;
		acc_gain[1][2] = (acc_cal[1][2] - acc_off[1]) / GRAVITY;

		acc_gain[2][0] = (acc_cal[2][3] - acc_off[2]) / GRAVITY;
		acc_gain[2][1] = (acc_cal[2][0] - acc_off[2]) / GRAVITY;
		acc_gain[2][2] = (acc_cal[2][2] - acc_off[2]) / GRAVITY;*/
	}
	else if (m_state & CalibrationModeState::GYROSCOPE)
	{
		//If the odr doesn't appear accurate we add the odr error state to the
		//current mode state. The next time we click a button in this mode the 
		//mode screen class will be alerted of the discrepancy and flash an
		//error message on screen. This is necessary for the gyroscope calibration
		//due to the integration step. If the time interval between data points
		//isn't correct then it will throw off the gyroscope gain calculation.
		//The calculated ODR will never be 100% accurate due to differences in 
		//ODR and connection interval. Because of this, only eggregious differences
		//will be noticed. A calculated ODR of 0.5 or 2.0 x the expected (or worse)
		//will flag the error.
		float calculated_odr = 0.0f, odr_error = 1.0f;
		calculated_odr = m_graphDataX.size() / m_graphDataX.back().x;

		odr_error = calculated_odr / m_sensorODR;
		data_time_stamps.clear();

		if (odr_error < 0.5f || odr_error > 2.0f) m_state |= CalibrationModeState::ODR_ERROR;

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
	else if (m_state & CalibrationModeState::MAGNETOMETER)
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
		mag_off[0] = (mag_max[0] + mag_min[0]) / 2.0;
		mag_off[1] = (mag_max[1] + mag_min[1]) / 2.0;
		mag_off[2] = (mag_max[2] + mag_min[2]) / 2.0;

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

void CalibrationMode::prepareRecording()
{
	//We end up doing the same actions a lot right before we begin recording data,
	//so it's more efficient to just create a method to carry the actions out
	if (!m_stageSet)
	{
		m_state |= CalibrationModeState::READY_TO_RECORD; //This will initiae data recording
		m_stageSet = true;
	}
	else if (m_state & CalibrationModeState::RECORDING_DATA)
	{
		std::wstring message = L"Recording Data, hold sensor steady for " + std::to_wstring((float)(data_timer_duration - data_timer_elapsed) / 1000.0f) + L" more seconds.";
		((TextOverlay*)m_uiElements[4].get())->updateText(message);
	}
}

void CalibrationMode::displayGraph()
{
	//Although not necessary for the calibration process it's useful to look at a graphical
	//representation of the accumulated data to make sure that things look correct. This
	//method get's called after all calibration data has been gathered.

	if (m_state & CalibrationModeState::ACCELEROMETER)
	{
		//set the min and max data values for the graph, this value will change depending on which 
		//calibration is being carried out.
		float centerLineLocation, upperLineLocation, lowerLineLocation;

		((Graph*)m_uiElements[6].get())->setAxisMaxAndMins({ 0,  -12.0 }, { m_graphDataX.back().x, 12.0 });
		//add a few axis lines to the graph
		centerLineLocation = 0.0f; //The average of the highest and lowest data point
		upperLineLocation = GRAVITY; //95% of the highest data point
		lowerLineLocation = -GRAVITY; //95% of the lowest data point

		((Graph*)m_uiElements[6].get())->addDataSet(m_graphDataX, UIColor::Red);
		((Graph*)m_uiElements[6].get())->addDataSet(m_graphDataY, UIColor::Blue);
		((Graph*)m_uiElements[6].get())->addDataSet(m_graphDataZ, UIColor::Green);

		((Graph*)m_uiElements[6].get())->addAxisLine(0, centerLineLocation);
		((Graph*)m_uiElements[6].get())->addAxisLine(0, upperLineLocation);
		((Graph*)m_uiElements[6].get())->addAxisLine(0, lowerLineLocation);

		std::wstring axisText = std::to_wstring(centerLineLocation);
		((Graph*)m_uiElements[6].get())->addAxisLabel(axisText, centerLineLocation);

		axisText = std::to_wstring(upperLineLocation);
		((Graph*)m_uiElements[6].get())->addAxisLabel(axisText, upperLineLocation);

		axisText = std::to_wstring(lowerLineLocation);
		((Graph*)m_uiElements[6].get())->addAxisLabel(axisText, lowerLineLocation);

		m_uiElements[6]->setState(m_uiElements[6]->getState() ^ UIElementState::Invisible); //lastly, make the graph visible
	}
	else if (m_state & CalibrationModeState::MAGNETOMETER)
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

		((Graph*)m_uiElements[9].get())->setAxisMaxAndMins({ -125.0f,  -125.0f }, { 125.0f, 125.0f });
		((Graph*)m_uiElements[9].get())->addDataSet(xy, UIColor::Red);
		((Graph*)m_uiElements[9].get())->addDataSet(xyc, UIColor::Green);
		((Graph*)m_uiElements[9].get())->addAxisLine(0, 0);
		((Graph*)m_uiElements[9].get())->addAxisLine(1, 0);
		
		((Graph*)m_uiElements[10].get())->setAxisMaxAndMins({ -125.0f,  -125.0f }, { 125.0f, 125.0f });
		((Graph*)m_uiElements[10].get())->addDataSet(xz, UIColor::Blue);
		((Graph*)m_uiElements[10].get())->addDataSet(xzc, UIColor::Green);
		((Graph*)m_uiElements[10].get())->addAxisLine(0, 0);
		((Graph*)m_uiElements[10].get())->addAxisLine(1, 0);

		((Graph*)m_uiElements[11].get())->setAxisMaxAndMins({ -125.0f,  -125.0f }, { 125.0f, 125.0f });
		((Graph*)m_uiElements[11].get())->addDataSet(yz, UIColor::Yellow);
		((Graph*)m_uiElements[11].get())->addDataSet(yzc, UIColor::Green);
		((Graph*)m_uiElements[11].get())->addAxisLine(0, 0);
		((Graph*)m_uiElements[11].get())->addAxisLine(1, 0);

		//TODO: add axis lines

		//lastly, make the graphs visible
		m_uiElements[9]->setState(m_uiElements[9]->getState() ^ UIElementState::Invisible); 
		m_uiElements[10]->setState(m_uiElements[10]->getState() ^ UIElementState::Invisible);
		m_uiElements[11]->setState(m_uiElements[11]->getState() ^ UIElementState::Invisible);
	}
}

std::pair<float*, float**> CalibrationMode::getCalibrationResults()
{
	//If we like the calibration results, we use this method to get them to the IMU class
	//and update the external calibration files for future use
	if (m_state & CalibrationModeState::ACCELEROMETER) return { acc_off, acc_gain};
	else if (m_state & CalibrationModeState::GYROSCOPE) return { gyr_off, gyr_gain };
	else if (m_state & CalibrationModeState::MAGNETOMETER) return { mag_off, mag_gain };
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