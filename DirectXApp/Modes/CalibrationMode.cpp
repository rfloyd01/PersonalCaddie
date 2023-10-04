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
	std::wstring introMessage = L"In calibration mode we can manually calculate the offsets and cross-axis gains for the accelerometer, gyroscope and magnetometer on the Personal Caddie to get more accurate data. Select one of the sensors using the buttons below to begin the calibration process.";
	std::wstring accButtonText = L"Accelerometer Calibration";
	std::wstring gyrButtonText = L"Gyroscope Calibration";
	std::wstring magButtonText = L"Magnetometer Calibration";
	std::wstring continueButtonText = L"Continue";

	TextButton accButton(windowSize, { 0.16, 0.65 }, { 0.14, 0.1 }, accButtonText);
	TextButton gyrButton(windowSize, { 0.5, 0.65 }, { 0.14, 0.1 }, gyrButtonText);
	TextButton magButton(windowSize, { 0.83, 0.65 }, { 0.14, 0.1 }, magButtonText);
	TextButton continueButton(windowSize, { 0.16, 0.85 }, { 0.14, 0.1 }, continueButtonText);
	continueButton.setState(continueButton.getState() | UIElementState::Invisible); //this button is invisible to start off

	//The body and sub-title text overlays will change depending on the current mode state so they
	//aren't declared in the initializeTextOverlay() method
	TextOverlay body(windowSize, { UIConstants::BodyTextLocationX, UIConstants::BodyTextLocationY }, { UIConstants::BodyTextSizeX, UIConstants::BodyTextSizeY },
		introMessage, 0.75f * UIConstants::BodyTextPointSize, { UIColor::White }, { 0,  (unsigned int)introMessage.length() }, UITextJustification::UpperLeft);
	TextOverlay subTitle(windowSize, { UIConstants::SubTitleTextLocationX, UIConstants::SubTitleTextLocationY - 0.025f }, { UIConstants::SubTitleTextSizeX, UIConstants::SubTitleTextSizeY },
		L"Fill", 1.05f * UIConstants::SubTitleTextPointSize, {UIColor::White}, {0, 4}, UITextJustification::UpperCenter);
	subTitle.setState(subTitle.getState() | UIElementState::Invisible); ///the sub-title starts off invisible

	//Create a graph ui element, it will stay invisible until after individual calibrations are complete
	Graph graph(windowSize, { 0.7, 0.65 }, { 0.5, 0.5 });
	graph.setState(graph.getState() | UIElementState::Invisible);
	
	m_uiElements.push_back(std::make_shared<TextButton>(accButton));
	m_uiElements.push_back(std::make_shared<TextButton>(gyrButton));
	m_uiElements.push_back(std::make_shared<TextButton>(magButton));
	m_uiElements.push_back(std::make_shared<TextButton>(continueButton));
	m_uiElements.push_back(std::make_shared<TextOverlay>(body));
	m_uiElements.push_back(std::make_shared<TextOverlay>(subTitle));
	m_uiElements.push_back(std::make_shared<Graph>(graph));

	m_state = initialState;

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
	
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 6; j++) acc_cal[i][j] = 0.0f;
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

		switch (i)
		{
		case 0:
			((TextOverlay*)m_uiElements[4].get())->updateText(L"A 6-point tumble calibration will be performed on the accelerometer. Press the continue button when ready to begin and then follow the on-screen instructions.");
			break;
			//TODO: add gyro and mag stuff after finishing accelerometer
		}

		m_state |= ModeState::Active;
	}
	else if (i == 3)
	{
		//The continue button was clicked, this has the effect of advancing the current stage by 1
		advanceToNextStage();
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

void CalibrationMode::addData(std::vector<std::vector<std::vector<float> > > const& sensorData, float sensorODR)
{
	//This method gets called asynchronously whenever new data is ready. We simply iterate over the appropriate
	//data (given the current calibration we're doing) and add it to the internal data vector.

	if (m_state & CalibrationModeState::RECORDING_DATA) //only add date if we're actively recording
	{
		int calibrationType;
		if (m_state & CalibrationModeState::ACCELEROMETER) calibrationType = raw_acceleration;
		else if (m_state & CalibrationModeState::GYROSCOPE) calibrationType = raw_rotation;
		else if (m_state & CalibrationModeState::MAGNETOMETER) calibrationType = raw_magnetic;

		float timeIncrement = 1.0f / sensorODR;
		int current_acc_stage = m_currentStage / 2 - 1;

		for (int i = 0; i < sensorData[calibrationType][0].size(); i++)
		{
			//x, y and z data gets added to the overall data vectors
			m_graphDataX.push_back({ m_timeStamp, sensorData[calibrationType][0][i] });
			m_graphDataY.push_back({ m_timeStamp, sensorData[calibrationType][1][i] });
			m_graphDataZ.push_back({ m_timeStamp, sensorData[calibrationType][2][i] });

			//add x, y and z data to accerlometer matrix (even if we aren't doing the accelerometer test)
			acc_cal[0][current_acc_stage] += sensorData[calibrationType][0][i];
			acc_cal[1][current_acc_stage] += sensorData[calibrationType][1][i];
			acc_cal[2][current_acc_stage] += sensorData[calibrationType][2][i];
			avg_count++;

			m_timeStamp += timeIncrement;
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
			data_timer_duration = 1000; //the acceleromter needs 5 seconds of data at each stage
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
				m_stageSet = true;
			}
		}
		break;
	}
	case 14:
	{
		//Go back to the beginning of the calibration mode
		m_state ^= CalibrationModeState::ACCELEROMETER;
		m_uiElements[0]->setState(m_uiElements[0]->getState() ^ UIElementState::Invisible);
		m_uiElements[1]->setState(m_uiElements[1]->getState() ^ UIElementState::Invisible);
		m_uiElements[2]->setState(m_uiElements[2]->getState() ^ UIElementState::Invisible);

		//Update the body text
		((TextOverlay*)m_uiElements[4].get())->updateText(L"In calibration mode we can manually calculate the offsets and cross-axis gains for the accelerometer, gyroscope and magnetometer on the Personal Caddie to get more accurate data. Select one of the sensors using the buttons below to begin the calibration process.");

		m_uiElements[3]->setState(m_uiElements[3]->getState() | UIElementState::Invisible); //make the continue button invisible
		m_uiElements[5]->setState(m_uiElements[5]->getState() | UIElementState::Invisible); //make the sub-title invisible
		m_uiElements[6]->setState(m_uiElements[6]->getState() ^ UIElementState::Invisible); //make the graph invisible
		//TODO: Clear data and lines from graph here

		m_currentStage = 0; //reset the current stage to 0
		m_state ^= ModeState::Active;
	}
	}
}

void CalibrationMode::gyroscopeCalibration()
{

}

void CalibrationMode::magnetometerCalibration()
{

}

void CalibrationMode::calculateCalNumbers()
{
	if (m_state & CalibrationModeState::ACCELEROMETER)
	{
		//The axis order for the tumble calibration is: [+Y, -X, +Z, +X, -Z, -Y]
		//Order of tumble point text is [+Y, -X, +Z, +X, -Z, -Y]
		acc_off[0] = (acc_cal[0][1] + acc_cal[0][3]) / 2.0;
		acc_off[1] = (acc_cal[1][0] + acc_cal[1][5]) / 2.0;
		acc_off[2] = (acc_cal[2][2] + acc_cal[2][4]) / 2.0;

		//Calculate Gain Matrix
		//(positive_reading - negative_reading) / 2G
		acc_gain[0][0] = (acc_cal[0][3] - acc_cal[0][1]) / (2 * GRAVITY);
		acc_gain[1][0] = (acc_cal[1][3] - acc_cal[1][1]) / (2 * GRAVITY);
		acc_gain[2][0] = (acc_cal[2][3] - acc_cal[2][1]) / (2 * GRAVITY);

		acc_gain[0][1] = (acc_cal[0][0] - acc_cal[0][5]) / (2 * GRAVITY);
		acc_gain[1][1] = (acc_cal[1][0] - acc_cal[1][5]) / (2 * GRAVITY);
		acc_gain[2][1] = (acc_cal[2][0] - acc_cal[2][5]) / (2 * GRAVITY);

		acc_gain[0][2] = (acc_cal[0][2] - acc_cal[0][4]) / (2 * GRAVITY);
		acc_gain[1][2] = (acc_cal[1][2] - acc_cal[1][4]) / (2 * GRAVITY);
		acc_gain[2][2] = (acc_cal[2][2] - acc_cal[2][4]) / (2 * GRAVITY);
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

	if (m_graphDataX.size() >= 2)
	{
		//set the min and max data values for the graph, this value will change depending on which 
		//calibration is being carried out.
		//TODO: Add gyro and mag stuff when I get there
		//add a few axis lines to the graph
		float centerLineLocation, upperLineLocation, lowerLineLocation;

		if (m_state & CalibrationModeState::ACCELEROMETER)
		{
			((Graph*)m_uiElements[6].get())->setAxisMaxAndMins({ 0,  -12.0 }, { m_graphDataX.back().x, 12.0 });
			//add a few axis lines to the graph
			centerLineLocation = 0.0f; //The average of the highest and lowest data point
			upperLineLocation = GRAVITY; //95% of the highest data point
			lowerLineLocation = -GRAVITY; //95% of the lowest data point
		}

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