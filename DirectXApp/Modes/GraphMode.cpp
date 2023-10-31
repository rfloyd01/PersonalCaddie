#include "pch.h"
#include "GraphMode.h"

GraphMode::GraphMode()
{
	//set a gray background color for the mode
	m_backgroundColor = UIColor::Blue;
}

uint32_t GraphMode::initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState)
{
	//Create a button that will generate a sin graph
	TextButton recordButton(windowSize, { 0.1, 0.2 }, { 0.12, 0.1 }, L"Start Recording Data");

	std::wstring options = L"Acceleration\nAngular Velocity\nMagnetic Field\nRaw Acceleration\nRaw Angular Velocity\nRaw Magnetic Field";
	DropDownMenu dataSelection(windowSize, { 0.9, 0.2 }, { 0.2, 0.1 }, options, 0.025, 5, false);

	Graph graph(windowSize, { 0.5, 0.65 }, { 0.9, 0.6 });

	m_uiElements.push_back(std::make_shared<TextButton>(recordButton));
	m_uiElements.push_back(std::make_shared<DropDownMenu>(dataSelection));
	m_uiElements.push_back(std::make_shared<Graph>(graph));

	//Initialize all overlay text
	initializeTextOverlay(windowSize);

	m_state = 0;
	
	//When this mode is initialzed we go into a state of CanTransfer
	return (ModeState::CanTransfer | ModeState::PersonalCaddieSensorIdleMode);
}

void GraphMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	for (int i = 0; i < m_uiElements.size(); i++) m_uiElements[i] = nullptr;
	m_uiElements.clear();
}

void GraphMode::initializeTextOverlay(winrt::Windows::Foundation::Size windowSize)
{
	//Title information
	std::wstring title_message = L"Graph Mode";
	TextOverlay title(windowSize, { UIConstants::TitleTextLocationX, UIConstants::TitleTextLocationY }, { UIConstants::TitleTextSizeX, UIConstants::TitleTextSizeY },
		title_message, UIConstants::TitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)title_message.length() }, UITextJustification::CenterCenter);
	m_uiElements.push_back(std::make_shared<TextOverlay>(title));

	//Footnote information
	std::wstring footnote_message = L"Press Esc. to return to main menu";
	TextOverlay footnote(windowSize, { UIConstants::FootNoteTextLocationX, UIConstants::FootNoteTextLocationY }, { UIConstants::FootNoteTextSizeX, UIConstants::FootNoteTextSizeY },
		footnote_message, UIConstants::FootNoteTextPointSize, { UIColor::White }, { 0,  (unsigned int)footnote_message.length() }, UITextJustification::LowerRight);
	m_uiElements.push_back(std::make_shared<TextOverlay>(footnote));
}

uint32_t GraphMode::handleUIElementStateChange(int i)
{
	if (i == 0)
	{
		//UI Element 0 is begin/stop recording button. This will bring the personal caddie
		//into or out of sensor active mode. If coming out of active mode it will also display
		//a graph with the data collected.
		if (!(m_state & GraphModeState::RECORDING))
		{
			//We aren't currently recording anything so we put the Personal Caddie into sensor
			//active mode and start listening for data updates.
			m_uiElements[0]->getText()->message = L"Stop Recording Data";

			//Clear out any existing data points
			((Graph*)m_uiElements[2].get())->removeAllLines(); //this will also clear out any text on the graph

			//clear out all existing data, and then add a single dummy point for later use
			m_graphDataX.clear();
			m_graphDataY.clear();
			m_graphDataZ.clear();

			m_graphDataX.push_back({ 0, 0 });
			m_graphDataY.push_back({ 0, 0 });
			m_graphDataZ.push_back({ 0, 0 });

			//rest the local minimums and maximums. Use values that are just about guaranteed
			//to be overwritten.
			m_minimalPoint = { 5000.0f, 5000.0f }; //max reading should be from gyroscope at +/-2000 dps
			m_maximalPoint = { -5000.0f, -5000.0f };

			//initialize the data collection clock
			data_collection_start = std::chrono::steady_clock::now();
		}
		else
		{
			//We're currently recording data, we stop by alerting the Personal Caddie to enter
			//the sensor idle power mode. We then display any data gethered during the recording
			//session
			m_uiElements[0]->getText()->message = L"Start Recording Data";

			if (m_graphDataX.size() >= 2)
			{
				/*m_graphDataX[0].y = m_graphDataX[1].y;
				m_graphDataY[0].y = m_graphDataY[1].y;
				m_graphDataZ[0].y = m_graphDataZ[1].y;*/

				//Data collection starts as soon as the sensor is turned on, which means that
				//the first half second or so of data will be junk (this is the nature of 
				//IMU readings before they warm up). Because of this, drop the first 25
				//data points. This corresponds to 0.5 seconds of data at 50Hz ODR and 0.25
				//seconds of data at 100Hz ODR.
				if (m_graphDataX.size() > 25)
				{
					m_graphDataX.erase(m_graphDataX.begin(), m_graphDataX.begin() + 25);
					m_graphDataY.erase(m_graphDataY.begin(), m_graphDataY.begin() + 25);
					m_graphDataZ.erase(m_graphDataZ.begin(), m_graphDataZ.begin() + 25);
				}

				//set the min and max data values for the graph
				((Graph*)m_uiElements[2].get())->setAxisMaxAndMins({ m_graphDataX[0].x,  m_minimalPoint.y}, {m_graphDataX.back().x, m_maximalPoint.y});

				((Graph*)m_uiElements[2].get())->addDataSet(m_graphDataX, UIColor::Red);
				((Graph*)m_uiElements[2].get())->addDataSet(m_graphDataY, UIColor::Blue);
				((Graph*)m_uiElements[2].get())->addDataSet(m_graphDataZ, UIColor::Green);

				//add a few axis lines to the graph
				float centerLineLocation = (m_minimalPoint.y + m_maximalPoint.y) / 2.0f; //The average of the highest and lowest data point
				float upperLineLocation = m_maximalPoint.y - 0.05f * (m_maximalPoint.y - m_minimalPoint.y); //95% of the highest data point
				float lowerLineLocation = m_minimalPoint.y + 0.05f * (m_maximalPoint.y - m_minimalPoint.y); //95% of the lowest data point

				float totalRotation = 0.0f;
				for (int i = 1; i < m_graphDataZ.size(); i++) totalRotation += testIntegrateData(m_graphDataZ[i].y, m_graphDataZ[i - 1].y, m_graphDataZ[i].x - m_graphDataZ[i - 1].x);

				((Graph*)m_uiElements[2].get())->addAxisLine(X, centerLineLocation);
				((Graph*)m_uiElements[2].get())->addAxisLine(X, upperLineLocation);
				((Graph*)m_uiElements[2].get())->addAxisLine(X, lowerLineLocation);

				std::wstring axisText = std::to_wstring(centerLineLocation);
				((Graph*)m_uiElements[2].get())->addAxisLabel(axisText, centerLineLocation);

				axisText = std::to_wstring(upperLineLocation);
				((Graph*)m_uiElements[2].get())->addAxisLabel(axisText, upperLineLocation);

				axisText = std::to_wstring(lowerLineLocation);
				((Graph*)m_uiElements[2].get())->addAxisLabel(axisText, lowerLineLocation);
			}
		}
		m_state ^= GraphModeState::RECORDING; //toggle the recording state
	}
	else if (i == 1)
	{
		
	}
	return m_state;
}

void GraphMode::addData(std::vector<std::vector<std::vector<float> > > const& sensorData, float sensorODR, float timeStamp, int totalSamples)
{
	//If we're currently recording data, then every time a new set of data is ready this method will
	//get called and add the selected data to the data set for each axis.
	if (!m_state & GraphModeState::RECORDING) return; //only add data if we're actually recording

	std::wstring dataType = ((DropDownMenu*)m_uiElements[1].get())->getSelectedOption();
	DataType selectedDataType = getCurrentlySelectedDataType(dataType);
	//float time_increment = 1.0 / sensorODR, current_time = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - data_collection_start).count() / 1000000000.0f;

	for (int i = 0; i < totalSamples; i++)
	{
		float x_data = sensorData[static_cast<int>(selectedDataType)][X][i];
		float y_data = sensorData[static_cast<int>(selectedDataType)][Y][i];
		float z_data = sensorData[static_cast<int>(selectedDataType)][Z][i];

		//Uncomment below two lines to see feed of incoming data
		/*std::wstring data = std::to_wstring(x_data) + L", " + std::to_wstring(y_data) + L", " + std::to_wstring(z_data) + L"\n";
		OutputDebugString(&data[0]);*/

		//overwrite the local maxima and minima if necessary
		if (x_data > m_maximalPoint.y) m_maximalPoint.y = x_data;
		else if (x_data < m_minimalPoint.y) m_minimalPoint.y = x_data;

		if (y_data > m_maximalPoint.y) m_maximalPoint.y = y_data;
		else if (y_data < m_minimalPoint.y) m_minimalPoint.y = y_data;

		if (z_data > m_maximalPoint.y) m_maximalPoint.y = z_data;
		else if (z_data < m_minimalPoint.y) m_minimalPoint.y = z_data;

		//TODO: Every now and then one of these push_back calls leads to a crash,
		//need to investigate why this is the case.
		/*m_graphDataX.push_back({ m_graphDataX.back().x + time_increment, x_data });
		m_graphDataY.push_back({ m_graphDataY.back().x + time_increment, y_data });
		m_graphDataZ.push_back({ m_graphDataZ.back().x + time_increment, z_data });*/
		m_graphDataX.push_back({ timeStamp + i / sensorODR, x_data });
		m_graphDataY.push_back({ timeStamp + i / sensorODR, y_data });
		m_graphDataZ.push_back({ timeStamp + i / sensorODR, z_data });

		//current_time += time_increment;
	}
}

DataType GraphMode::getCurrentlySelectedDataType(std::wstring dropDownSelection)
{
	//TODO: Use raw data only until calibration files get created
	if (dropDownSelection == L"Acceleration") return DataType::ACCELERATION;
	else if (dropDownSelection == L"Angular Velocity") return DataType::ROTATION;
	else if (dropDownSelection == L"Magnetic Field") return DataType::MAGNETIC;
	else if (dropDownSelection == L"Raw Acceleration") return DataType::RAW_ACCELERATION;
	else if (dropDownSelection == L"Raw Angular Velocity") return DataType::RAW_ROTATION;
	else if (dropDownSelection == L"Raw Magnetic Field") return DataType::RAW_MAGNETIC;
	else return DataType::ACCELERATION; //default to acceleration
}

float GraphMode::testIntegrateData(float p1, float p2, float t)
{
	return t * ((p1 + p2) / 2);
}