#include "pch.h"
#include "GraphMode.h"

GraphMode::GraphMode()
{
	//set a gray background color for the mode
	m_backgroundColor = UIColor::Blue;
}

uint32_t GraphMode::initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState)
{
	//Take the current screen size and pass it to the UIElementManager, this is so that the manager knows
	//how large to make each element.
	m_uiManager.updateScreenSize(windowSize);

	//Create a button that will generate a sin graph
	TextButton recordButton(windowSize, { 0.1, 0.2 }, { 0.12, 0.1 }, L"Start Recording Data");

	std::wstring options = L"Acceleration\nAngular Velocity\nMagnetic Field\nRaw Acceleration\nRaw Angular Velocity\nRaw Magnetic Field\nLinear Acceleration";
	DropDownMenu dataSelection(windowSize, { 0.9, 0.2 }, { 0.2, 0.1 }, options, 0.025, 5, false);

	Graph graph(windowSize, { 0.5, 0.65 }, { 0.9, 0.6 });

	m_uiManager.addElement<TextButton>(recordButton, L"Record Button");
	m_uiManager.addElement<DropDownMenu>(dataSelection, L"Data Dropdown Menu");
	m_uiManager.addElement<Graph>(graph, L"Graph");

	//Initialize all overlay text
	initializeTextOverlay(windowSize);

	m_state = 0;
	m_recording = false;
	m_currentDataType = DataType::ACCELERATION; //Default to graphing acceleration data

	//For this mode we immediately put the Personal Caddie into Sensor Idle mode
	auto mode = PersonalCaddiePowerMode::SENSOR_IDLE_MODE;
	m_mode_screen_handler(ModeAction::PersonalCaddieChangeMode, (void*)&mode);//request the Personal Caddie to be placed into active mode to start recording data
	
	//When this mode is initialzed we go into a state of CanTransfer
	return ModeState::CanTransfer;
}

void GraphMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	m_uiManager.removeAllElements();

	//Put the sensor back into connected mode before exiting
	auto mode = PersonalCaddiePowerMode::CONNECTED_MODE;
	m_mode_screen_handler(ModeAction::PersonalCaddieChangeMode, (void*)&mode);//request the Personal Caddie to be placed into active mode to start recording data
}

void GraphMode::initializeTextOverlay(winrt::Windows::Foundation::Size windowSize)
{
	//Title information
	std::wstring title_message = L"Graph Mode";
	TextOverlay title(windowSize, { UIConstants::TitleTextLocationX, UIConstants::TitleTextLocationY }, { UIConstants::TitleTextSizeX, UIConstants::TitleTextSizeY },
		title_message, UIConstants::TitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)title_message.length() }, UITextJustification::CenterCenter);
	m_uiManager.addElement<TextOverlay>(title, L"Title Text");

	//Footnote information
	std::wstring footnote_message = L"Press Esc. to return to main menu";
	TextOverlay footnote(windowSize, { UIConstants::FootNoteTextLocationX, UIConstants::FootNoteTextLocationY }, { UIConstants::FootNoteTextSizeX, UIConstants::FootNoteTextSizeY },
		footnote_message, UIConstants::FootNoteTextPointSize, { UIColor::White }, { 0,  (unsigned int)footnote_message.length() }, UITextJustification::LowerRight);
	m_uiManager.addElement<TextOverlay>(footnote, L"Footnote Text");
}

void GraphMode::handleKeyPress(winrt::Windows::System::VirtualKey pressedKey)
{
	//The only key we can press in this mode is the escape key. All this key does is exit the 
	//mode and go back to the development menu.
	if (pressedKey == winrt::Windows::System::VirtualKey::Escape)
	{
		ModeType newMode = ModeType::DEVELOPER_TOOLS;
		m_mode_screen_handler(ModeAction::ChangeMode, (void*)&newMode);
	}
}

void GraphMode::uiElementStateChangeHandler(std::shared_ptr<ManagedUIElement> element)
{
	if (element->name == L"Record Button")
	{
		//Pressing this button will either bring the personal caddie
		//into or out of sensor active mode. If coming out of active mode it will also display
		//a graph with the data collected.
		if (!m_recording)
		{
			//We aren't currently recording anything so we put the Personal Caddie into sensor
			//active mode and start listening for data updates.
			m_uiManager.getElement<TextButton>(L"Record Button")->updateText(L"Stop Recording Data");

			//Clear out any existing data points
			m_uiManager.getElement<Graph>(L"Graph")->removeAllLines(); //this will also clear out any text on the graph

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

			//Put the Personal Caddie into Sensor Active mode to start recording data
			auto mode = PersonalCaddiePowerMode::SENSOR_ACTIVE_MODE;
			m_mode_screen_handler(ModeAction::PersonalCaddieChangeMode, (void*)&mode);//request the Personal Caddie to be placed into active mode to start recording data

			//Certain extrapolated data types (like linear acceleration) require using the current
			//rotation quaternion from the Personal Caddie, which requires the Madgwick filter to converge first.
			//If one of these data types is selected set the m_converge variable to false, manually change the 
			//value of the Madgwick filter's beta value, and alert the Personal Caddie to start calculating the 
			//specific data type
			if (static_cast<int>(m_currentDataType) > 5)
			{
				m_mode_screen_handler(ModeAction::PersonalCaddieToggleCalculatedData, (void*)&m_currentDataType); //Turn on calculations for the extrapolated data type
				
				//Temporarily increase the beta value for the Madgwick filter
				float initial_beta_value = 2.5f;
				m_mode_screen_handler(ModeAction::MadgwickUpdateFilter, (void*)&initial_beta_value);
				m_converged = false; //this prevents data collection from starting until the Madgwick filter quaternions converge
			}
		}
		else
		{
			//We're currently recording data, we stop by alerting the Personal Caddie to enter
			//the sensor idle power mode. We then display any data gethered during the recording
			//session
			m_uiManager.getElement<TextButton>(L"Record Button")->updateText(L"Start Recording Data");
			m_recording = false; //We set the recording flag to false immediately to stop gathering data

			//Put the Personal Caddie back into Sensor Idle mode
			auto mode = PersonalCaddiePowerMode::SENSOR_IDLE_MODE;
			m_mode_screen_handler(ModeAction::PersonalCaddieChangeMode, (void*)&mode);//request the Personal Caddie to be placed into active mode to start recording data

			//If an extraploated data type was being recorded, tell the Personal Caddie to stop
			//calculations on it
			m_mode_screen_handler(ModeAction::PersonalCaddieToggleCalculatedData, (void*)&m_currentDataType);

			if (m_graphDataX.size() >= 2)
			{
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
				m_uiManager.getElement<Graph>(L"Graph")->setAxisMaxAndMins({ m_graphDataX[0].x,  m_minimalPoint.y }, { m_graphDataX.back().x, m_maximalPoint.y });

				m_uiManager.getElement<Graph>(L"Graph")->addDataSet(m_uiManager.getScreenSize(), m_graphDataX, UIColor::Red);
				m_uiManager.getElement<Graph>(L"Graph")->addDataSet(m_uiManager.getScreenSize(), m_graphDataY, UIColor::Blue);
				m_uiManager.getElement<Graph>(L"Graph")->addDataSet(m_uiManager.getScreenSize(), m_graphDataZ, UIColor::Green);

				//add a few axis lines to the graph
				float centerLineLocation = (m_minimalPoint.y + m_maximalPoint.y) / 2.0f; //The average of the highest and lowest data point
				float upperLineLocation = m_maximalPoint.y - 0.05f * (m_maximalPoint.y - m_minimalPoint.y); //95% of the highest data point
				float lowerLineLocation = m_minimalPoint.y + 0.05f * (m_maximalPoint.y - m_minimalPoint.y); //95% of the lowest data point

				m_uiManager.getElement<Graph>(L"Graph")->addAxisLine(m_uiManager.getScreenSize(), X, centerLineLocation);
				m_uiManager.getElement<Graph>(L"Graph")->addAxisLine(m_uiManager.getScreenSize(), X, upperLineLocation);
				m_uiManager.getElement<Graph>(L"Graph")->addAxisLine(m_uiManager.getScreenSize(), X, lowerLineLocation);

				std::wstring axisText = std::to_wstring(centerLineLocation);
				m_uiManager.getElement<Graph>(L"Graph")->addAxisLabel(m_uiManager.getScreenSize(), axisText, X, centerLineLocation);

				axisText = std::to_wstring(upperLineLocation);
				m_uiManager.getElement<Graph>(L"Graph")->addAxisLabel(m_uiManager.getScreenSize(), axisText, X, upperLineLocation);

				axisText = std::to_wstring(lowerLineLocation);
				m_uiManager.getElement<Graph>(L"Graph")->addAxisLabel(m_uiManager.getScreenSize(), axisText, X, lowerLineLocation);
			}
		}
	}
	else if (element->name == L"Data Dropdown Menu")
	{
		//If we've selected a new data type from the drop down menu then we need to 
		//update the data type variable for the mode.
		if (element->element->getChildren()[2]->getState() & UIElementState::Invisible)
		{
			//A new option has been selected
			std::wstring dataType = m_uiManager.getElement<DropDownMenu>(L"Data Dropdown Menu")->getSelectedOption();
			m_currentDataType = getCurrentlySelectedDataType(dataType);
		}
	}
}

void GraphMode::addData(std::vector<std::vector<std::vector<float> > > const& sensorData, float sensorODR, float timeStamp, int totalSamples)
{
	//If we're currently recording data, then every time a new set of data is ready this method will
	//get called and add the selected data to the data set for each axis.
	if (!m_recording) return; //only add data if we're actually recording
	if (!m_converged) return; //if the current data type needs to Madgwick filter to converge first don't record data yet

	//DataType dt = m_currentDataType;
	//if (dt == DataType::LINEAR_ACCELERATION) dt = DataType::ACCELERATION; //linear acceleration builds off of normal acceleration

	for (int i = 0; i < totalSamples; i++)
	{
		float x_data = sensorData[static_cast<int>(m_currentDataType)][X][i];
		float y_data = sensorData[static_cast<int>(m_currentDataType)][Y][i];
		float z_data = sensorData[static_cast<int>(m_currentDataType)][Z][i];

		//Uncomment below two lines to see feed of incoming data
		/*std::wstring data = std::to_wstring(x_data) + L", " + std::to_wstring(y_data) + L", " + std::to_wstring(z_data) + L"\n";
		OutputDebugString(&data[0]);*/

		//overwrite the local maxima and minima if necessary. These are used for 
		//creating reference lines on the graph
		if (x_data > m_maximalPoint.y) m_maximalPoint.y = x_data;
		else if (x_data < m_minimalPoint.y) m_minimalPoint.y = x_data;

		if (y_data > m_maximalPoint.y) m_maximalPoint.y = y_data;
		else if (y_data < m_minimalPoint.y) m_minimalPoint.y = y_data;

		if (z_data > m_maximalPoint.y) m_maximalPoint.y = z_data;
		else if (z_data < m_minimalPoint.y) m_minimalPoint.y = z_data;

		m_graphDataX.push_back({ timeStamp + i / sensorODR, x_data });
		m_graphDataY.push_back({ timeStamp + i / sensorODR, y_data });
		m_graphDataZ.push_back({ timeStamp + i / sensorODR, z_data });
	}
}

void GraphMode::addQuaternions(std::vector<glm::quat> const& quaternions, int quaternion_number, float time_stamp, float delta_t)
{
	//Certain data types require that the calculated rotation quaternion has correctly converge on 
	//the sensor's real life orientation. Linear Acceleration for example is calculated by removing 
	//the effects of gravity calculated by looking at the current orientation. For these data types 
	//we wait for the convergence to happen before any data is recorded.
	if (!m_converged)
	{
		//if the filter hasn't yet converged add the first quaternion from this set to the convergence array
		//and call the conergenceCheck() method
		for (int i = 0; i < quaternion_number; i++) m_convergenceQuaternions.push_back(quaternions[i]);
		convergenceCheck();
	}

	//else
	//{
	//	//Mark the location in the data vectors where we need to start remove the effects of gravity
	//	int start = m_graphDataX.size() - quaternion_number;

	//	//The addQuaternions() method gets called directly after the addData() method so there should always be 
	//	//a fresh batch of acceleration data for us to manipulate
	//	for (int i = 0; i < quaternion_number; i++)
	//	{
	//		//calculate and then remove the acceleration due to gravity obtained from each 
	//		//rotation quaternion
	//		float gx = 2 * GRAVITY * (quaternions[i].x * quaternions[i].z - quaternions[i].w * quaternions[i].y);
	//		float gy = 2 * GRAVITY * (quaternions[i].y * quaternions[i].z + quaternions[i].w * quaternions[i].x);
	//		float gz = GRAVITY * (quaternions[i].w * quaternions[i].w - quaternions[i].x * quaternions[i].x - quaternions[i].y * quaternions[i].y + quaternions[i].z * quaternions[i].z);

	//		std::wstring tester = L"Total Acceleration Y: " + std::to_wstring(m_graphDataY[start + i].y) + L"\nGravitational Acceleration Y: " + std::to_wstring(gy) + L"\n\n";
	//		OutputDebugString(&tester[0]);

	//		//Update the minimum and maximum values for the graph accordingly so the graph 
	//		//doesn't use the raw acceleration values
	//		/*if (m_graphDataX[start + i].y == m_maximalPoint.y) m_maximalPoint.y -= gx;
	//		else if (m_graphDataX[start + i].y == m_minimalPoint.y) m_minimalPoint.y -= gx;

	//		if (m_graphDataY[start + i].y == m_maximalPoint.y) m_maximalPoint.y -= gy;
	//		else if (m_graphDataY[start + i].y == m_minimalPoint.y) m_minimalPoint.y -= gy;

	//		if (m_graphDataZ[start + i].y == m_maximalPoint.y) m_maximalPoint.y -= gz;
	//		else if (m_graphDataZ[start + i].y == m_minimalPoint.y) m_minimalPoint.y -= gz;*/

	//		//And then update the actual data points
	//		m_graphDataX[start + i].y -= gx;
	//		m_graphDataY[start + i].y -= gy;
	//		m_graphDataZ[start + i].y -= gz;
	//	}
	//}
}

void GraphMode::pc_ModeChange(PersonalCaddiePowerMode newMode)
{
	//We wait for the Personal Caddie to Physically be placed into active mode before
	//initializing the data timer and to start recording data
	if (newMode == PersonalCaddiePowerMode::SENSOR_ACTIVE_MODE)
	{
		m_recording = true;
		data_collection_start = std::chrono::steady_clock::now();
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
	else if (dropDownSelection == L"Linear Acceleration") return DataType::LINEAR_ACCELERATION;
	else return DataType::ACCELERATION; //default to acceleration
}

float GraphMode::testIntegrateData(float p1, float p2, float t)
{
	return t * ((p1 + p2) / 2);
}

void GraphMode::convergenceCheck()
{
	//We check to see if the filter has converged so that we can update the filter's beta value
	//to something more useful. To check for convergence, we average the last 10 quaternions together
	//and check the error between this and the current quaternion. If the error in the w, x, y and z
	//fields are all below a certain threshold then the convergence check passes and we reset the
	//beta value of the filter.
	if (m_convergenceQuaternions.size() >= 10)
	{
		glm::quat averageQuaternion = { 0, 0, 0, 0 };
		float error_threshold = 0.05f;

		for (int i = m_convergenceQuaternions.size() - 10; i < m_convergenceQuaternions.size(); i++) averageQuaternion += m_convergenceQuaternions[i];
		averageQuaternion /= 10;

		float w_error = (averageQuaternion.w - m_convergenceQuaternions.back().w) / (averageQuaternion.w + m_convergenceQuaternions.back().w);
		float x_error = (averageQuaternion.x - m_convergenceQuaternions.back().x) / (averageQuaternion.x + m_convergenceQuaternions.back().x);
		float y_error = (averageQuaternion.y - m_convergenceQuaternions.back().y) / (averageQuaternion.y + m_convergenceQuaternions.back().y);
		float z_error = (averageQuaternion.z - m_convergenceQuaternions.back().z) / (averageQuaternion.z + m_convergenceQuaternions.back().z);

		if (w_error >= 1.0f) w_error = 1.0f / w_error;

		if (w_error > error_threshold || w_error < -error_threshold) return;
		if (x_error > error_threshold || x_error < -error_threshold) return;
		if (y_error > error_threshold || y_error < -error_threshold) return;
		if (z_error > error_threshold || z_error < -error_threshold) return;

		//If all error check pass we reset the beta value of the filter to once again bias the gyro readings
		float initial_beta_value = 0.041f;
		m_mode_screen_handler(ModeAction::MadgwickUpdateFilter, (void*)&initial_beta_value);;

		//And do a little clean up
		createAlert(L"Convergence Complete", UIColor::DarkGray, m_uiManager.getScreenSize());
		m_convergenceQuaternions.clear();
		m_converged = true; //prevents this convergenceCheck() from being called again and starts data capture
	}
}