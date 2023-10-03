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
	
	m_uiElements.push_back(std::make_shared<TextButton>(accButton));
	m_uiElements.push_back(std::make_shared<TextButton>(gyrButton));
	m_uiElements.push_back(std::make_shared<TextButton>(magButton));
	m_uiElements.push_back(std::make_shared<TextButton>(continueButton));
	m_uiElements.push_back(std::make_shared<TextOverlay>(body));
	m_uiElements.push_back(std::make_shared<TextOverlay>(subTitle));

	m_state = initialState;
	m_currentStage = 0;
	m_stageSet = false;

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

void CalibrationMode::startDataCapture()
{
	//We can't start data capture until the Personal Caddie has been placed into active mode, which happens asynchronously.
	//Because of that, this method is called from the ModeScreen class once the Personal Caddie has been placed into 
	//sensor active mode and is ready to start recording data. Furthermore, all IMU sensors have a slight wake up time
	//where data they generate will be "junk". We can also use this method to wait for a brief moment after turning on
	//the sensors to actually record data.
	if (m_state & CalibrationModeState::READY_TO_RECORD)
	{
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
		advanceToNextStage(); //when recording is over we advance to the next stage of the calibration
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
		}
	}

	switch (m_currentStage)
	{
	case 1:
	{
		if (!m_stageSet)
		{
			((TextOverlay*)m_uiElements[4].get())->updateText(L"Lay the sensor flat on the table with the +Y axis pointing upwards like shown in the image. Leave the sensor stationary like this for 5 seconds as it collects data. Press the continue button when ready.");
			data_timer_duration = 5000; //set the data timer for 5 seconds
			m_stageSet = true;
		}
		
		break;
	}
	case 2:
	{
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
		break;
	}
	case 3:
	{
		if (!m_stageSet)
		{
			((TextOverlay*)m_uiElements[4].get())->updateText(L"Data collection complete. Rotate the sensor so that the +X axis is pointing down as shown in the image. When ready, press the continue button. Make sure to keep the sensor as still as possible until the timer runs out.");
			m_state |= CalibrationModeState::READY_TO_RECORD;
			m_stageSet = true;
			break;
		}
	}
	case 4:
	{
		
		break;
	}
	}
}

void CalibrationMode::gyroscopeCalibration()
{

}

void CalibrationMode::magnetometerCalibration()
{

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