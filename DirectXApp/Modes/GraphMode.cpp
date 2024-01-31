#include "pch.h"
#include "GraphMode.h"
#include "Graphics/Objects/3D/Elements/model.h"
#include "Math/quaternion_functions.h"

#include <limits>

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
	TextButton recordButton(m_uiManager.getScreenSize(), { 0.1, 0.2 }, { 0.12, 0.1 }, L"Start Recording Data");

	std::wstring options = L"Acceleration\nAngular Velocity\nMagnetic Field\nRaw Acceleration\nRaw Angular Velocity\nRaw Magnetic Field\nLinear Acceleration";
	DropDownMenu dataSelection(m_uiManager.getScreenSize(), { 0.9, 0.2 }, { 0.2, 0.1 }, options, 0.25, 5, false);

	Graph graph(m_uiManager.getScreenSize(), { 0.5, 0.65 }, { 0.9, 0.6 }, true, UIColor::White, UIColor::Black, false, true, true);

	m_uiManager.addElement<TextButton>(recordButton, L"Record Button");
	//m_uiManager.addElement<DropDownMenu>(dataSelection, L"Data Dropdown Menu");
	m_uiManager.addElement<Graph>(graph, L"Graph");

	//NEW: Use Check boxes for selecting data type instead of a drop down which
	//will allow multiple data types to be plotted at the samme time.
	float square_ratio = MAX_SCREEN_HEIGHT / MAX_SCREEN_WIDTH;
	CheckBox accelerationCheckBox(m_uiManager.getScreenSize(), { 0.8f, 0.2f }, { square_ratio * 0.025f, 0.025f }, true);
	CheckBox angularVelocityCheckBox(m_uiManager.getScreenSize(), { 0.8f, 0.235f }, { square_ratio * 0.025f, 0.025f });
	CheckBox magneticCheckBox(m_uiManager.getScreenSize(), { 0.8f, 0.27f }, { square_ratio * 0.025f, 0.025f });
	CheckBox rawAccelerationCheckBox(m_uiManager.getScreenSize(), { 0.8f + square_ratio * 0.195f, 0.2f }, { square_ratio * 0.025f, 0.025f });
	CheckBox rawAngularVelocityCheckBox(m_uiManager.getScreenSize(), { 0.8f + square_ratio * 0.195f, 0.235f }, { square_ratio * 0.025f, 0.025f });
	CheckBox rawMagneticCheckBox(m_uiManager.getScreenSize(), { 0.8f + square_ratio * 0.195f, 0.27f }, { square_ratio * 0.025f, 0.025f });
	CheckBox linearAccelerationCheckBox(m_uiManager.getScreenSize(), { 0.8f + square_ratio * 0.195f, 0.305f }, { square_ratio * 0.025f, 0.025f });

	TextOverlay accelerationBoxLabel(m_uiManager.getScreenSize(), { 0.735f, 0.2f }, { square_ratio * 0.195f, 0.025 }, L"Acceleration", 0.85f, { UIColor::White }, { 0, 12 }, UITextJustification::CenterRight, false);
	TextOverlay angularVelocityBoxLabel(m_uiManager.getScreenSize(), { 0.735f, 0.235f }, { square_ratio * 0.195f, 0.025 }, L"Angular Velocity", 0.85f, { UIColor::White }, { 0, 16 }, UITextJustification::CenterRight, false);
	TextOverlay magneticBoxLabel(m_uiManager.getScreenSize(), { 0.735f, 0.27f }, { square_ratio * 0.195f, 0.025 }, L"Magnetic", 0.85f, { UIColor::White }, { 0, 8 }, UITextJustification::CenterRight, false);
	TextOverlay rawAccelerationBoxLabel(m_uiManager.getScreenSize(), { 0.735f + square_ratio * 0.195f, 0.2f }, { square_ratio * 0.195f, 0.025 }, L"Raw Acceleration", 0.85f, { UIColor::White }, { 0, 16 }, UITextJustification::CenterRight, false);
	TextOverlay rawAngularVelocityBoxLabel(m_uiManager.getScreenSize(), { 0.735f + square_ratio * 0.195f, 0.235f }, { square_ratio * 0.195f, 0.025 }, L"Raw Ang. Velocity", 0.85f, { UIColor::White }, { 0, 17 }, UITextJustification::CenterRight, false);
	TextOverlay rawMagneticBoxLabel(m_uiManager.getScreenSize(), { 0.735f + square_ratio * 0.195f, 0.27f }, { square_ratio * 0.195f, 0.025 }, L"Raw Magnetic", 0.85f, { UIColor::White }, { 0, 12 }, UITextJustification::CenterRight, false);
	TextOverlay linearAccelerationBoxLabel(m_uiManager.getScreenSize(), { 0.735f + square_ratio * 0.195f, 0.305f }, { square_ratio * 0.195f, 0.025 }, L"Linear Acceleration", 0.85f, { UIColor::White }, { 0, 19 }, UITextJustification::CenterRight, false);

	m_uiManager.addElement<CheckBox>(accelerationCheckBox, L"Check Box 1");
	m_uiManager.addElement<TextOverlay>(accelerationBoxLabel, L"Label 1");
	m_uiManager.addElement<CheckBox>(angularVelocityCheckBox, L"Check Box 2");
	m_uiManager.addElement<TextOverlay>(angularVelocityBoxLabel, L"Label 2");
	m_uiManager.addElement<CheckBox>(magneticCheckBox, L"Check Box 3");
	m_uiManager.addElement<TextOverlay>(magneticBoxLabel, L"Label 3");
	m_uiManager.addElement<CheckBox>(rawAccelerationCheckBox, L"Check Box 4");
	m_uiManager.addElement<TextOverlay>(rawAccelerationBoxLabel, L"Label 4");
	m_uiManager.addElement<CheckBox>(rawAngularVelocityCheckBox, L"Check Box 5");
	m_uiManager.addElement<TextOverlay>(rawAngularVelocityBoxLabel, L"Label 5");
	m_uiManager.addElement<CheckBox>(rawMagneticCheckBox, L"Check Box 6");
	m_uiManager.addElement<TextOverlay>(rawMagneticBoxLabel, L"Label 6");
	m_uiManager.addElement<CheckBox>(linearAccelerationCheckBox, L"Check Box 7");
	m_uiManager.addElement<TextOverlay>(linearAccelerationBoxLabel, L"Label 7");

	//Initialize all overlay text
	initializeTextOverlay();

	//Load the model of the Personal Caddie to render on screen
	loadModel();

	//Initialize 3D rendering data
	m_quaternions.clear();
	m_timeStamps.clear();
	m_convergenceQuaternions.clear();
	for (int i = 0; i < 39; i++)
	{
		m_quaternions.push_back({ 1.0f, 0.0f, 0.0f, 0.0f });
		m_timeStamps.push_back(0.0f);
	}
	m_renderQuaternion = { m_quaternions[0].x, m_quaternions[0].y, m_quaternions[0].z, m_quaternions[0].w };

	//Get the current Heading Offset so the sensor is squared up when rendered
	m_mode_screen_handler(ModeAction::IMUHeading, nullptr);

	m_state = 0;
	m_recording = false;
	m_selectedDataTypes = 1; //represents acceleration data only

	resetData(); //make sure the graphData vector has no data in it

	//For this mode we immediately put the Personal Caddie into Sensor Idle mode
	auto mode = PersonalCaddiePowerMode::SENSOR_IDLE_MODE;
	m_mode_screen_handler(ModeAction::PersonalCaddieChangeMode, (void*)&mode);//request the Personal Caddie to be placed into active mode to start recording data
	
	//Set up a vector of different colors that can be used when creating graphs with
	//multiple lines of data. If enough lines are plotted that all colors are used,
	//then new lines will start over at the beginning of the vector with the same colors.
	m_lineColors = { UIColor::Red, UIColor::Green, UIColor::Blue, UIColor::Magenta, UIColor::Yellow, UIColor::Cyan, UIColor::Orange, UIColor::Purple, UIColor::Mint, UIColor::Pink, UIColor::PaleGray, UIColor::DarkGray };
	m_currentLineColor = 0;

	//When this mode is initialzed we go into a state of CanTransfer
	return ModeState::CanTransfer;
}

void GraphMode::loadModel()
{
	//Load the model of the Personal Caddie sensor
	m_volumeElements.push_back(std::make_shared<Model>());
	((Model*)m_volumeElements[0].get())->loadModel("Assets/Models/personal_caddie.gltf");
	((Model*)m_volumeElements[0].get())->setScale({ 0.01f, 0.01f, 0.01f });

	m_materialTypes.push_back(MaterialType::DEFAULT); //This actually doesn't matter for loading models, but is need to avoid a nullptr exception

	//Set the mesh and materials for the model
	m_mode_screen_handler(ModeAction::RendererGetMaterial, nullptr);
}

void GraphMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	m_uiManager.removeAllElements();

	//Clear out all volume Elements
	for (int i = 0; i < m_volumeElements.size(); i++) m_volumeElements[i] = nullptr;
	m_volumeElements.clear();

	//Put the sensor back into connected mode before exiting
	auto mode = PersonalCaddiePowerMode::CONNECTED_MODE;
	m_mode_screen_handler(ModeAction::PersonalCaddieChangeMode, (void*)&mode);//request the Personal Caddie to be placed into active mode to start recording data
	m_graphData.clear(); //clear out any recorded data
}

void GraphMode::initializeTextOverlay()
{
	//Title information
	std::wstring title_message = L"Graph Mode";
	TextOverlay title(m_uiManager.getScreenSize(), { UIConstants::TitleTextLocationX, UIConstants::TitleTextLocationY }, { UIConstants::TitleTextSizeX, UIConstants::TitleTextSizeY },
		title_message, UIConstants::TitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)title_message.length() }, UITextJustification::CenterCenter);
	m_uiManager.addElement<TextOverlay>(title, L"Title Text");

	//Footnote information
	std::wstring footnote_message = L"Press Esc. to return to main menu";
	TextOverlay footnote(m_uiManager.getScreenSize(), { UIConstants::FootNoteTextLocationX, UIConstants::FootNoteTextLocationY }, { UIConstants::FootNoteTextSizeX, UIConstants::FootNoteTextSizeY },
		footnote_message, UIConstants::FootNoteTextPointSize, { UIColor::White }, { 0,  (unsigned int)footnote_message.length() }, UITextJustification::LowerRight);
	m_uiManager.addElement<TextOverlay>(footnote, L"Footnote Text");
}

//DEBUG: Render a model of the Personal Caddie to see if the rotation quaternions are 
//converging properly while measuring linear acceleration
void GraphMode::update()
{
	//Animate the current rotation quaternion obtained from the Personal Caddie. We need to look at the 
	//time stamp to figure out which quaternion is correct. We do this since the ODR of the sensors won't always
	//match up with the frame rate of the current screen.
	if (m_needsCamera)
	{
		while (m_update_in_process) {}; //data is currently being updated asynchronously, wait for it to finish

		float time_elapsed_since_data_start = (float)std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - data_start_timer).count() / 1000000000.0f;
		float quat[3];
		bool updated = false;

		for (int i = (m_currentQuaternion + 1); i < m_quaternions.size(); i++)
		{
			if (time_elapsed_since_data_start >= (m_timeStamps[i] - m_timeStamps[0]))
			{
				//since the relative timer is greater than the time distance between the current quaternion
				//and the data set start time, this quaternion can potentially be rendered.
				m_currentQuaternion = i;
				updated = true; //set flag to update render quaternion);
			}
			else break; //we haven't reached the current quaternion in time yet so break out of loop

		}

		if (updated)
		{
			//Rotate the current quaternion from the Madgwick filter by the heading offset to line up with the computer monitor.
			glm::quat adjusted_q;
			adjusted_q = QuaternionMultiply(m_headingOffset, m_quaternions[m_currentQuaternion]);

			float Q_sensor[3] = { adjusted_q.x, adjusted_q.y, adjusted_q.z };
			float Q_computer[3] = { Q_sensor[computer_axis_from_sensor_axis[0]], Q_sensor[computer_axis_from_sensor_axis[1]], Q_sensor[computer_axis_from_sensor_axis[2]] };

			m_renderQuaternion = { Q_computer[0], Q_computer[1], Q_computer[2], adjusted_q.w };
		}

		//Rotate each face according to the given quaternion
		((Model*)m_volumeElements[0].get())->translateAndRotateFace({ 0.0f, 0.95f, 2.0f }, m_renderQuaternion);
	}
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
			
			resetData();

			//Put the Personal Caddie into Sensor Active mode to start recording data
			auto mode = PersonalCaddiePowerMode::SENSOR_ACTIVE_MODE;
			m_mode_screen_handler(ModeAction::PersonalCaddieChangeMode, (void*)&mode);//request the Personal Caddie to be placed into active mode to start recording data

			//Certain extrapolated data types (like linear acceleration) require using the current
			//rotation quaternion from the Personal Caddie, which requires the Madgwick filter to converge first.
			//If one of these data types is selected set the m_converge variable to false, manually change the 
			//value of the Madgwick filter's beta value, and alert the Personal Caddie to start calculating the 
			//specific data type
			if (m_selectedDataTypes >= (1 << static_cast<int>(DataType::LINEAR_ACCELERATION))) //TODO: remove first part of OR when ready
			{
				toggleCalculatedDataTypes();
				
				//Temporarily increase the beta value for the Madgwick filter
				float initial_beta_value = 2.5f;
				m_mode_screen_handler(ModeAction::MadgwickUpdateFilter, (void*)&initial_beta_value);
				m_converged = false; //this prevents data collection from starting until the Madgwick filter quaternions converge
			}

			//DEBUG: If We're currently gathering linear acceleration data, render an image of the sensor with 
			//quaternions from the Personal Caddie to confirm that convergence has happened
			if (m_selectedDataTypes & (1 << static_cast<int>(DataType::LINEAR_ACCELERATION))) m_needsCamera = true; //TODO: same as above todo
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
			toggleCalculatedDataTypes();

			bool mins_maxes_set = false;

			for (int i = 0; i < static_cast<int>(DataType::END); i++)
			{
				if (!(m_selectedDataTypes & (1 << i))) continue; //don't try and add data that isn't actually there
				if (!mins_maxes_set)
				{
					//set the min and max data values for the graph. I'd rather do this outside of this loop,
					//however, this was easier since you don't know which data types are actually getting graphed.
					//TODO: May just be easier to set the max x variable when clicking the stop record button.
					m_uiManager.getElement<Graph>(L"Graph")->setAxisMaxAndMins({ 0.0f,  m_minimalPoint.y }, { m_graphData[i][X].back().x, m_maximalPoint.y });
					mins_maxes_set = true;
				}

				//The color for each line gets selected from the m_lineColors vector. Modular division
				//is used when setting the index to make sure that if more lines are plotted than colors
				//are available, the colors wrap around back to the beginning of the vector.
				m_uiManager.getElement<Graph>(L"Graph")->addGraphData(m_graphData[i][X], m_lineColors[(m_currentLineColor++) % m_lineColors.size()], getDataTypeText(static_cast<DataType>(i)) + L".x");
				m_uiManager.getElement<Graph>(L"Graph")->addGraphData(m_graphData[i][Y], m_lineColors[(m_currentLineColor++) % m_lineColors.size()], getDataTypeText(static_cast<DataType>(i)) + L".y");
				m_uiManager.getElement<Graph>(L"Graph")->addGraphData(m_graphData[i][Z], m_lineColors[(m_currentLineColor++) % m_lineColors.size()], getDataTypeText(static_cast<DataType>(i)) + L".z");
			}

			//DEBUG: If We're currently gathering linear acceleration data, stop rendering image of the sensor
			if (m_selectedDataTypes & (1 << static_cast<int>(DataType::LINEAR_ACCELERATION))) m_needsCamera = false; //TODO: remove first part of OR when ready
		}
	}
	else if (element->name.find(L"Check Box") != std::string::npos)
	{
	    //One of the data type check boxes was selected. If the box is unchecked then remove the appropriate
		//flag from the m_selectedDataTypes varaible, otherwise add the flag
		int dataType = (int)(element->name[element->name.length() - 1] - L'1');
		if (((CheckBox*)element->element.get())->isChecked()) m_selectedDataTypes |= ((1 << static_cast<int>(dataType)));
		else m_selectedDataTypes &= ~((1 << static_cast<int>(dataType)));

		std::wstring debug = std::to_wstring(m_selectedDataTypes) + L"\n";
		OutputDebugString(&debug[0]);
    }
}

void GraphMode::resetData()
{
	//This method simply clears out any data accumlated in the graphData structure and recreates
	//empty arrays for all of the data types. It then looks at the m_selectedDataTypes variable
	//and creates vectors and pairs to hold X, Y and Z data.
	m_graphData.clear();
	for (int i = 0; i < static_cast<int>(DataType::END); i++)
	{
		m_graphData.push_back({});

		if (m_selectedDataTypes & (1 << i))
		{
			for (int j = 0; j < 3; j++)
			{
				m_graphData.back().push_back({});
			}
		}
	}

	//reset the local minimums and maximums. Use the maximum and minimum float values to ensure
	//that they get overwritten
	m_minimalPoint = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max() }; //max reading should be from gyroscope at +/-2000 dps
	m_maximalPoint = { -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max() };
}

bool GraphMode::dataTypeSelected(DataType t)
{
	//returns true if the given data type is currently set as a flag in the m_selectedDataTypes varaible
	uint32_t flag = (1 << static_cast<int>(t));
	return (m_selectedDataTypes & flag);
}

std::wstring GraphMode::getDataTypeText(DataType t)
{
	//A simple method for giving a text representation of the current data type to go
	//in the key of the graph. (TODO: This method should probably exist in the PersonalCaddie.h
	//file where the data types are defined.)
	switch (t)
	{
	case DataType::ACCELERATION: return L"Acceleration";
	case DataType::ROTATION: return L"Rotation";
	case DataType::MAGNETIC: return L"Magnetic";
	case DataType::RAW_ACCELERATION: return L"Raw Acceleration";
	case DataType::RAW_ROTATION: return L"Raw Rotation";
	case DataType::RAW_MAGNETIC: return L"Raw Magnetic";
	case DataType::LINEAR_ACCELERATION: return L"Linear Acceleration";
	case DataType::VELOCITY: return L"Velocity";
	case DataType::LOCATION: return L"Location";
	case DataType::EULER_ANGLES: return L"Euler Angles";
	default: return L"";
	}
}

void GraphMode::addData(std::vector<std::vector<std::vector<float> > > const& sensorData, float sensorODR, float timeStamp, int totalSamples)
{
	//If we're currently recording data, then every time a new set of data is ready this method will
	//get called and add the selected data to the data set for each axis.
	if (!m_recording) return; //only add data if we're actually recording
	if (!m_converged) return; //if the current data type needs the Madgwick filter to converge first don't record data yet

	m_update_in_process = true; //prevent access to data vectors while update is occuring

	for (int i = 0; i < totalSamples; i++)
	{
		for (int j = 0; j < static_cast<int>(DataType::END); j++)
		{
			if (!(m_selectedDataTypes & (1 << j))) continue; //skip over any non-selected data types

			float x_data = sensorData[j][X][i];
			float y_data = sensorData[j][Y][i];
			float z_data = sensorData[j][Z][i];
			float time_stamp = timeStamp + i / sensorODR;

			m_graphData[j][X].push_back({ time_stamp, x_data });
			m_graphData[j][Y].push_back({ time_stamp, y_data });
			m_graphData[j][Z].push_back({ time_stamp, z_data });

			//check to see if any new mins or maxes have been found
			if (x_data > m_maximalPoint.y) m_maximalPoint.y = x_data;
			else if (x_data < m_minimalPoint.y) m_minimalPoint.y = x_data;

			if (y_data > m_maximalPoint.y) m_maximalPoint.y = y_data;
			else if (y_data < m_minimalPoint.y) m_minimalPoint.y = y_data;

			if (z_data > m_maximalPoint.y) m_maximalPoint.y = z_data;
			else if (z_data < m_minimalPoint.y) m_minimalPoint.y = z_data;
		}
	}
}

void GraphMode::addQuaternions(std::vector<glm::quat> const& quaternions, int quaternion_number, float time_stamp, float delta_t)
{
	//Certain data types require that the calculated rotation quaternion has correctly converge on 
	//the sensor's real life orientation. Linear Acceleration for example is calculated by removing 
	//the effects of gravity calculated by looking at the current orientation. For these data types 
	//we wait for the convergence to happen before any data is recorded.
	//make sure that the length of the m_quaternion and m_timestamp vectors are the same as the quaternion_number parameter.
	if (m_quaternions.size() != quaternion_number)
	{
		m_quaternions.erase(m_quaternions.begin() + quaternion_number, m_quaternions.end());
		m_timeStamps.erase(m_timeStamps.begin() + quaternion_number, m_timeStamps.end());
	}

	m_currentQuaternion = -1; //reset the current quaternion to be rendered

	for (int i = 0; i < quaternion_number; i++)
	{
		m_quaternions[i] = quaternions[i];
		m_timeStamps[i] = time_stamp + i * delta_t;
	}

	if (!m_converged)
	{
		//if the filter hasn't yet converged add the first quaternion from this set to the convergence array
		//and call the conergenceCheck() method
		for (int i = 0; i < quaternion_number; i++) m_convergenceQuaternions.push_back(quaternions[i]);
		convergenceCheck();
	}

	data_start_timer = std::chrono::steady_clock::now(); //set relative time
	m_update_in_process = false; //grant access to data vectors once update is complete
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

void GraphMode::toggleCalculatedDataTypes()
{
	//When turning on the calculated data types, only send the highest numbered one as it cascades into turning on the 
	//lower calculated data types. This is because the higher order calculated types require the lower ordered ones (i.e.
	//location depends on velocity, and velocity depends on linear acceleration). Euler angles are turned on separately
	uint32_t position_flag = (1 << static_cast<int>(DataType::LOCATION));
	for (int i = 0; i < 3; i++)
	{
		if (position_flag & m_selectedDataTypes)
		{
			DataType max_calculated_data_type = static_cast<DataType>(log2(position_flag)); //instead of using the log here, I may be better off keeping track of the left shifted distance
			m_mode_screen_handler(ModeAction::PersonalCaddieToggleCalculatedData, (void*)&max_calculated_data_type); //Turn on calculations for the extrapolated data type
			break;
		}
		position_flag >>= 1;
	}

	if (m_selectedDataTypes & (1 << static_cast<int>(DataType::EULER_ANGLES)))
	{
		DataType euler = DataType::EULER_ANGLES;
		m_mode_screen_handler(ModeAction::PersonalCaddieToggleCalculatedData, (void*)&euler);
	}
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
		createAlert(L"Convergence Complete", UIColor::DarkGray);
		m_convergenceQuaternions.clear();
		m_converged = true; //prevents this convergenceCheck() from being called again and starts data capture
	}
}

void GraphMode::getIMUHeadingOffset(glm::quat heading)
{
	//This method gets the heading offset saved in the IMU class and updates
	//the local heading offset variable with it. This variable is used to make
	//sure the rendered image aligns with the computer monitor
	m_headingOffset = heading;
}