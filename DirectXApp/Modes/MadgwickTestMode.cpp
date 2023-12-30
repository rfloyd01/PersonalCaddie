#include "pch.h"
#include "MadgwickTestMode.h"
#include "Graphics/Objects/3D/Elements/Model.h"
#include "Math/quaternion_functions.h"

MadgwickTestMode::MadgwickTestMode()
{
	//set a very light gray background color for the mode
	m_backgroundColor = UIColor::FreeSwingMode;
}

uint32_t MadgwickTestMode::initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState)
{
	//Take the current screen size and pass it to the UIElementManager, this is so that the manager knows
	//how large to make each element.
	m_uiManager.updateScreenSize(windowSize);

	//Create UI Elements on the page
	initializeTextOverlay(windowSize);

	//Load the model of the Personal Caddie to render on screen
	loadModel();

	//m_needsCamera = true; //alerts the mode screen that 3d rendering will take place in this mode

	//float sensorHeight = 0.5f, sensorLength = 0.3f, sensorWidth = 0.052f; //TODO: Utilize these dimension variables to create the sensor instead of all the numbers seen below
	//std::shared_ptr<Face> sensorTop = std::make_shared<Face>(DirectX::XMFLOAT3(-0.15f, 0.026f, -0.25f), DirectX::XMFLOAT3(0.15f, 0.026f, -0.25f), DirectX::XMFLOAT3(-0.15f, 0.026f, 0.25f));
	//std::shared_ptr<Face> sensorLeft = std::make_shared<Face>(DirectX::XMFLOAT3(-0.15f, 0.0f, -0.25f), DirectX::XMFLOAT3(-0.15f, -0.052f, -0.25f), DirectX::XMFLOAT3(-0.15f, 0.0f, 0.25f));
	//std::shared_ptr<Face> sensorRight = std::make_shared<Face>(DirectX::XMFLOAT3(0.15f, 0.0f, -0.25f), DirectX::XMFLOAT3(0.15f, -0.052f, -0.25f), DirectX::XMFLOAT3(0.15f, 0.0f, 0.25f));
	//std::shared_ptr<Face> sensorFront = std::make_shared<Face>(DirectX::XMFLOAT3(0.15f, -0.026f, 0.25f), DirectX::XMFLOAT3(-0.15f, -0.026f, 0.25f), DirectX::XMFLOAT3(0.15f, 0.026f,0.25f));
	//std::shared_ptr<Face> sensorBack = std::make_shared<Face>(DirectX::XMFLOAT3(-0.15f, -0.026f, -0.25f), DirectX::XMFLOAT3(0.15f, -0.026f, -0.25f), DirectX::XMFLOAT3(-0.15f, 0.026f, -0.25f));
	//std::shared_ptr<Face> sensorBottom = std::make_shared<Face>(DirectX::XMFLOAT3(0.15f, -0.026, -0.25f), DirectX::XMFLOAT3(-0.15f, -0.026f, -0.25f), DirectX::XMFLOAT3(0.15f, -0.026f, 0.25f));
	////note* - the left and right face don't seem to be in the right spot to me, but the sensor is rendered correctly so I'm leaving it

	//m_volumeElements.push_back(sensorTop);
	//m_volumeElements.push_back(sensorLeft);
	//m_volumeElements.push_back(sensorRight);
	//m_volumeElements.push_back(sensorFront);
	//m_volumeElements.push_back(sensorBack);
	//m_volumeElements.push_back(sensorBottom);

	////After creating the faces of the sensor, add the appropriate material types for each face.
	////The index of each material type in the vector needs to match the index of the appropriate
	////face in the m_volumeElements vector.
	//m_materialTypes.push_back(MaterialType::SENSOR_TOP);
	//m_materialTypes.push_back(MaterialType::SENSOR_LONG_SIDE);
	//m_materialTypes.push_back(MaterialType::SENSOR_LONG_SIDE);
	//m_materialTypes.push_back(MaterialType::SENSOR_SHORT_SIDE);
	//m_materialTypes.push_back(MaterialType::SENSOR_SHORT_SIDE);
	//m_materialTypes.push_back(MaterialType::SENSOR_BOTTOM);

	////After creating the necessary volume elements and material types, map each volume element 
	////to its given material through the master renderer class (accessed via the ModeScreenHandler)
	//m_mode_screen_handler(ModeAction::RendererGetMaterial, nullptr);

	m_currentRotation = 0.0f;
	m_currentDegree = PI / 2.0f;
	m_currentQuaternion = 0;

	//Default to acceleration for display data
	m_display_data_index = 0;
	m_display_data_type = L"Acceleration";
	m_display_data_units = L"m/s^2";
	m_current_data_type = DataType::ACCELERATION;

	m_quaternions.clear();
	m_testQuaternions.clear();
	m_timeStamps.clear();
	m_show_live_data = false;
	m_display_data[0] = {};
	m_display_data[1] = {};
	m_display_data[2] = {};

	//Load the quaternion vector with default quaternions and the 
	//time stamp vector with default times. We use 39 as this is
	//the maximum number of samples we can have, this will get 
	//pruned down when the first bit of data actuall comes in.
	for (int i = 0; i < 39; i++)
	{
		m_quaternions.push_back({ 1.0f, 0.0f, 0.0f, 0.0f });
		m_testQuaternions.push_back({ 1.0f, 0.0f, 0.0f, 0.0f });
		m_timeStamps.push_back(0.0f);
		m_display_data[0].push_back(0.0f);
		m_display_data[1].push_back(0.0f);
		m_display_data[2].push_back(0.0f);
	}
	m_renderQuaternion = { m_quaternions[0].x, m_quaternions[0].y, m_quaternions[0].z, m_quaternions[0].w };

	m_converged = false; //We need to let the filter re-converge every time this mode is opened
	m_convergenceQuaternions = {};

	//TESTING: Attempting to use Madgwick's latest version of his algorithm. For now I've 
	//manually put in an ODR of 50 Hz, but I should get this from the Personal Caddie class
	FusionOffsetInitialise(&m_offset, 50);
	FusionAhrsInitialise(&m_ahrs);

	const FusionAhrsSettings settings = {
			FusionConventionNwu,  /*Initializes with NWU magnetic orientation, I may need Enu */
			0.5f,
			2000.0f, /* replace this with actual gyroscope range in degrees/s */
			10.0f,
			10.0f,
			5 * 50, /* 5 seconds */
	};
	FusionAhrsSetSettings(&m_ahrs, &settings);
	m_useNewFilter = false;

	//We spend the entirety of our time in this mode with the Personal Caddie in Sensor Active
	//Mode. To get there we need to first put the Sensor into Idle mode
	auto mode = PersonalCaddiePowerMode::SENSOR_IDLE_MODE;
	m_mode_screen_handler(ModeAction::PersonalCaddieChangeMode, (void*)&mode);

	//The NeedMaterial modeState lets the mode screen know that it needs to pass
	//a list of materials to this mode that it can use to initialize 3d objects
	return (ModeState::CanTransfer | ModeState::NeedMaterial | ModeState::Active);
}

void MadgwickTestMode::loadModel()
{
	//Load the model of the Personal Caddie sensor
	m_volumeElements.push_back(std::make_shared<Model>());
	((Model*)m_volumeElements[0].get())->loadModel("Assets/Models/personal_caddie.gltf");
	((Model*)m_volumeElements[0].get())->setScale({ 0.01f, 0.01f, 0.01f });

	m_materialTypes.push_back(MaterialType::DEFAULT); //This actually doesn't matter for loading models, but is need to avoid a nullptr exception

	//Set the mesh and materials for the model
	m_mode_screen_handler(ModeAction::RendererGetMaterial, nullptr);

	m_needsCamera = true;
}

void MadgwickTestMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	m_uiManager.removeAllElements();

	for (int i = 0; i < m_volumeElements.size(); i++) m_volumeElements[i] = nullptr;
	m_volumeElements.clear();

	//If an extrapolated data type has currently been selected, turn off calculations in 
	//the Personal Caddie now
	if (m_current_data_type == DataType::LINEAR_ACCELERATION || m_current_data_type == DataType::VELOCITY || m_current_data_type == DataType::LOCATION)
		m_mode_screen_handler(ModeAction::PersonalCaddieToggleCalculatedData, (void*)&m_current_data_type);

	//Put the Personal Caddie back into Connected Mode when leaving this page. This can be 
	//done without going into Sensor Idle Mode first.
	auto mode = PersonalCaddiePowerMode::CONNECTED_MODE;
	m_mode_screen_handler(ModeAction::PersonalCaddieChangeMode, (void*)&mode);
}

void MadgwickTestMode::initializeTextOverlay(winrt::Windows::Foundation::Size windowSize)
{
	//Title information
	std::wstring title_message = L"Madgwick Filter Testing";
	TextOverlay title(windowSize, { UIConstants::TitleTextLocationX, UIConstants::TitleTextLocationY }, { UIConstants::TitleTextSizeX, UIConstants::TitleTextSizeY },
		title_message, UIConstants::TitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)title_message.length() }, UITextJustification::CenterCenter);
	m_uiManager.addElement<TextOverlay>(title, L"Title Text");

	//Footnote information
	std::wstring footnote_message = L"Press Esc. to return to settings menu";
	TextOverlay footnote(windowSize, { UIConstants::FootNoteTextLocationX, UIConstants::FootNoteTextLocationY }, { UIConstants::FootNoteTextSizeX, UIConstants::FootNoteTextSizeY },
		footnote_message, UIConstants::FootNoteTextPointSize, { UIColor::White }, { 0,  (unsigned int)footnote_message.length() }, UITextJustification::LowerRight);
	m_uiManager.addElement<TextOverlay>(footnote, L"Footnote Text");

	//Sensor data display information
	std::wstring sensor_info_message_one = L"\n";
	std::wstring sensor_info_message_two = L"\n";
	std::wstring sensor_info_message_three = L"\n";
	std::wstring sensor_info_message_four = L"";
	TextOverlay sensor_info(windowSize, { UIConstants::SensorInfoTextLocationX, UIConstants::SensorInfoTextLocationY }, { UIConstants::SensorInfoTextSizeX, UIConstants::SensorInfoTextSizeY },
		sensor_info_message_one + sensor_info_message_two + sensor_info_message_three + sensor_info_message_four, UIConstants::SensorInfoTextPointSize * 0.8f, { UIColor::White, UIColor::Red, UIColor::Blue, UIColor::Green },
		{ 0,  (unsigned int)sensor_info_message_one.length(),  (unsigned int)sensor_info_message_two.length(),  (unsigned int)sensor_info_message_three.length(),  (unsigned int)sensor_info_message_four.length() }, UITextJustification::LowerLeft);
	m_uiManager.addElement<TextOverlay>(sensor_info, L"Sensor Info 1 Text");

	sensor_info_message_one = L"\n";
	sensor_info_message_two = L"\n";
	sensor_info_message_three = L"\n";
	sensor_info_message_four = L"";
	TextOverlay sensor_info_two(windowSize, { UIConstants::SensorInfoTextLocationX, UIConstants::SensorInfoTextLocationY }, { UIConstants::SensorInfoTextSizeX, UIConstants::SensorInfoTextSizeY },
		sensor_info_message_one + sensor_info_message_two + sensor_info_message_three + sensor_info_message_four, UIConstants::SensorInfoTextPointSize * 0.8f, { UIColor::White, UIColor::Red, UIColor::Blue, UIColor::Green },
		{ 0,  (unsigned int)sensor_info_message_one.length(),  (unsigned int)sensor_info_message_two.length(),  (unsigned int)sensor_info_message_three.length(),  (unsigned int)sensor_info_message_four.length() }, UITextJustification::LowerRight);
	m_uiManager.addElement<TextOverlay>(sensor_info_two, L"Sensor Info 2 Text");

	//View data message
	std::wstring view_data_message = L"Press Enter to see live Data from Sensor\nPress Space to Center the Sensor\nPress ^ Arrow to Swap Filter";
	TextOverlay view_data(windowSize, { UIConstants::FootNoteTextLocationX - 0.33f, UIConstants::FootNoteTextLocationY }, { UIConstants::FootNoteTextSizeX, UIConstants::FootNoteTextSizeY },
		view_data_message, UIConstants::FootNoteTextPointSize, { UIColor::White }, { 0,  (unsigned int)view_data_message.length() }, UITextJustification::LowerCenter);
	m_uiManager.addElement<TextOverlay>(view_data, L"View Data Text");
}

void MadgwickTestMode::updateDisplayText()
{
	//This method is called to update the sensor data being displayed on screen
	if (!m_show_live_data) return; //we currently don't want to display data
	if ((m_display_data[0].size() == 0) || (m_display_data[1].size() == 0) || (m_display_data[2].size() == 0)) return; //currently we have no data to display, this can happen as old data is asynchronously overwritten
	if (m_currentQuaternion < 0 || m_currentQuaternion >= m_display_data[0].size()) return; //this should avoid any array index out of bounds exceptions
	int index = m_currentQuaternion; //m_currentQuaternion is volatile and can be changed from another thread. Create an index variable that won't change midway through this method

	std::wstring sensor_info_message_one = m_display_data_type + L":\nSensor Frame\n";
	std::wstring sensor_info_message_two = std::to_wstring(m_display_data[0][index]) + m_display_data_units + L"\n";
	std::wstring sensor_info_message_three = std::to_wstring(m_display_data[1][index]) + m_display_data_units + L"\n";
	std::wstring sensor_info_message_four = std::to_wstring(m_display_data[2][index]) + m_display_data_units + L"\n";
	m_uiManager.getElement<TextOverlay>(L"Sensor Info 1 Text")->updateText(sensor_info_message_one + sensor_info_message_two + sensor_info_message_three + sensor_info_message_four);
	m_uiManager.getElement<TextOverlay>(L"Sensor Info 1 Text")->updateColorLocations({ 0,  (unsigned int)sensor_info_message_one.length(),  (unsigned int)sensor_info_message_two.length(),  (unsigned int)sensor_info_message_three.length(),  (unsigned int)sensor_info_message_four.length() });

	sensor_info_message_one = L"\nDirectX Frame\n";
	sensor_info_message_two = std::to_wstring(m_display_data[computer_axis_from_sensor_axis[0]][index]) + m_display_data_units + L"\n";
	sensor_info_message_three = std::to_wstring(m_display_data[computer_axis_from_sensor_axis[1]][index]) + m_display_data_units + L"\n";
	sensor_info_message_four = std::to_wstring(m_display_data[computer_axis_from_sensor_axis[2]][index]) + m_display_data_units + L"\n";
	m_uiManager.getElement<TextOverlay>(L"Sensor Info 2 Text")->updateText(sensor_info_message_one + sensor_info_message_two + sensor_info_message_three + sensor_info_message_four);
	m_uiManager.getElement<TextOverlay>(L"Sensor Info 2 Text")->updateColorLocations({0,  (unsigned int)sensor_info_message_one.length(),  (unsigned int)sensor_info_message_two.length(),  (unsigned int)sensor_info_message_three.length(),  (unsigned int)sensor_info_message_four.length()});
}

void MadgwickTestMode::pc_ModeChange(PersonalCaddiePowerMode newMode)
{
	//As soon as we enter Sensor Idle Mode we jump straight into Sensor Active mode. Before that,
	//however, we also get the current Heading for the sensor which is located in the Personal Caddie
	//class and temporarily update the beta value of the Madgewick filter. The heading allows the sensor
	//to line up with the orientation of the computer screen while the beta value allows the sensor to 
	//quickly converge to the correct location.
	if (newMode == PersonalCaddiePowerMode::SENSOR_IDLE_MODE)
	{
		//Get the current Heading Offset
		m_mode_screen_handler(ModeAction::IMUHeading, nullptr);

		//Temporarily increase Madgwick beta gain to 2.5
		float initial_beta_value = 2.5f;
		m_mode_screen_handler(ModeAction::MadgwickUpdateFilter, (void*)&initial_beta_value);

		//Put the Sensor into Active mode to start taking readings
		auto mode = PersonalCaddiePowerMode::SENSOR_ACTIVE_MODE;
		m_mode_screen_handler(ModeAction::PersonalCaddieChangeMode, (void*)&mode);
	}
}

void MadgwickTestMode::addQuaternions(std::vector<glm::quat> const& quaternions, int quaternion_number, float time_stamp, float delta_t)
{
	//It's possible that the quaternions vector will have some empty/junk values at the end. This is because the number of 
	//samples coming from the sensor is dependent on the current sensor ODR (which can change). Because of this we 
	//only add the first quaternion_number quaternions to the vector on this page

	//make sure that the length of the m_quaternion and m_timestamp vectors are the same as the quaternion_number parameter.
	if (m_quaternions.size() != quaternion_number)
	{
		m_quaternions.erase(m_quaternions.begin() + quaternion_number, m_quaternions.end());
		m_testQuaternions.erase(m_testQuaternions.begin() + quaternion_number, m_testQuaternions.end());
		m_timeStamps.erase(m_timeStamps.begin() + quaternion_number, m_timeStamps.end());
	}

	m_currentQuaternion = -1; //reset the current quaternion to be rendered
	m_update_in_process = true;

	for (int i = 0; i < quaternion_number; i++)
	{
		m_quaternions[i] = quaternions[i];
		m_timeStamps[i] = time_stamp + i * delta_t;
	}

	data_start_timer = std::chrono::steady_clock::now(); //set relative time

	//m_update_in_process = false;

	if (!m_converged)
	{
		//if the filter hasn't yet converged add the first quaternion from this set to the convergence array
		//and call the conergenceCheck() method
		m_convergenceQuaternions.push_back(m_quaternions[0]);
		convergenceCheck();
	}
}

void MadgwickTestMode::addData(std::vector<std::vector<std::vector<float> > > const& sensorData, float sensorODR, float timeStamp, int totalSamples)
{
	//make sure that the length of the m_display_data vectors are the same as the totalSamples parameter
	if (m_display_data[0].size() != totalSamples)
	{
		m_display_data[0].erase(m_display_data[0].begin() + totalSamples, m_display_data[0].end());
		m_display_data[1].erase(m_display_data[1].begin() + totalSamples, m_display_data[1].end());
		m_display_data[2].erase(m_display_data[2].begin() + totalSamples, m_display_data[2].end());
	}

	for (int i = 0; i < totalSamples; i++)
	{
		m_display_data[0][i] = sensorData[m_display_data_index][0][i];
		m_display_data[1][i] = sensorData[m_display_data_index][1][i];
		m_display_data[2][i] = sensorData[m_display_data_index][2][i];
	}

	//TESTING of new Madgwick Filter
	float deltaT = timeStamp - m_timeStamps.back();
	int i = 0;

	while (true)
	{
		//Calibrated data is read in from the sensor
		FusionVector gyroscope = { sensorData[1][0][i], sensorData[1][1][i], sensorData[1][2][i] };
		FusionVector accelerometer = { sensorData[0][0][i], sensorData[0][1][i], sensorData[0][2][i] };
		FusionVector magnetometer = { sensorData[2][0][i], sensorData[2][1][i], sensorData[2][2][i] };

		//Update gyroscope offset correction algorithm
		gyroscope = FusionOffsetUpdate(&m_offset, gyroscope);

		FusionAhrsUpdate(&m_ahrs, gyroscope, accelerometer, magnetometer, deltaT);
		auto newQuat = FusionAhrsGetQuaternion(&m_ahrs);
		m_testQuaternions[i] = { newQuat.element.w, newQuat.element.x, newQuat.element.y, newQuat.element.z };

		i++;
		if (i >= m_timeStamps.size()) break;

		deltaT = m_timeStamps[i] - m_timeStamps[i - 1]; //should always equal sensor ODR since only the first time stamp is recorded 
	}
	m_update_in_process = false; //set this to false after addQuaternion and addData has been called
}

void MadgwickTestMode::update()
{
	//Animate the current rotation quaternion obtained from the Personal Caddie. We need to look at the 
	//time stamp to figure out which quaternion is correct. We do this since the ODR of the sensors won't always
	//match up with the frame rate of the current screen.
	
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
		if (m_useNewFilter) adjusted_q = QuaternionMultiply(m_headingOffset, m_testQuaternions[m_currentQuaternion]);
		else adjusted_q = QuaternionMultiply(m_headingOffset, m_quaternions[m_currentQuaternion]);

		float Q_sensor[3] = { adjusted_q.x, adjusted_q.y, adjusted_q.z };
		float Q_computer[3] = { Q_sensor[computer_axis_from_sensor_axis[0]], Q_sensor[computer_axis_from_sensor_axis[1]], Q_sensor[computer_axis_from_sensor_axis[2]] };

		m_renderQuaternion = { Q_computer[0], Q_computer[1], Q_computer[2], adjusted_q.w };
	}

	//Rotate each face according to the given quaternion
	((Model*)m_volumeElements[0].get())->translateAndRotateFace({ 0.0f, 0.0f, 1.0f }, m_renderQuaternion);

	//Update any sensor display text currently being rendered
	updateDisplayText();
}

void MadgwickTestMode::handleKeyPress(winrt::Windows::System::VirtualKey pressedKey)
{
	//There are few different keys we can process here. Like in the other modes, pressing the escape
	//key will go back to the previous menu. Pressing the space key will update the heading offset for
	//the Personal Caddie, pressing enter will toggle a live data stream, pressing the number keys will
	//alter the data seen on the stream, and pressing the up arrow will switch between different implementations
	//of the Madgwick filter.
	switch (pressedKey)
	{
	case winrt::Windows::System::VirtualKey::Escape:
	{
		ModeType newMode = ModeType::DEVELOPER_TOOLS;
		m_mode_screen_handler(ModeAction::ChangeMode, (void*)&newMode);
		break;
	}
	case winrt::Windows::System::VirtualKey::Number1:
	case winrt::Windows::System::VirtualKey::Number2:
	case winrt::Windows::System::VirtualKey::Number3:
	case winrt::Windows::System::VirtualKey::Number4:
	case winrt::Windows::System::VirtualKey::Number5:
	case winrt::Windows::System::VirtualKey::Number6:
	case winrt::Windows::System::VirtualKey::Number7:
	{
		switchDisplayDataType(static_cast<int>(pressedKey) - static_cast<int>(winrt::Windows::System::VirtualKey::Number0));
		break;
	}
	case winrt::Windows::System::VirtualKey::Enter:
	{
		toggleDisplayData();
		break;
	}
	case winrt::Windows::System::VirtualKey::Space:
	{
		setCurrentHeadingOffset();
		break;
	}
	case winrt::Windows::System::VirtualKey::Up:
	{
		toggleFilter();
		break;
	}
	//case winrt::Windows::System::VirtualKey::Down:
	//{
	//	//Test separating linear acceleration from gravitational acceleration
	//	glm::vec3 x_axis = { 1, 0, 0 }, y_axis = { 0, 1, 0 }, z_axis = { 0, 0, 1 };

	//	//Rotate each axis by the current quaternion
	//	QuatRotate(m_quaternions[0], x_axis);
	//	QuatRotate(m_quaternions[0], y_axis);
	//	QuatRotate(m_quaternions[0], z_axis);

	//	//Multiply the z component of each rotated vector to see how much gravity
	//	//it should be feeling
	//	float x_axis_gravity = GRAVITY * x_axis.z;
	//	float y_axis_gravity = GRAVITY * y_axis.z;
	//	float z_axis_gravity = GRAVITY * z_axis.z;

	//	//Compare the results of the above method to a much quicker method which needs ~90%
	//	//fewer mathematical operations
	//	glm::quat q = m_quaternions[0];

	//	float Gx = 2 * GRAVITY * (q.x * q.z - q.w * q.y);
	//	float Gy = 2 * GRAVITY * (q.y * q.z + q.w * q.x);
	//	float Gz = GRAVITY * (q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z);

	//	std::wstring gravString = L"Gravity Components: [" + std::to_wstring(x_axis_gravity) + L", " + std::to_wstring(y_axis_gravity) + L", " + std::to_wstring(z_axis_gravity) + L"]\n";
	//	std::wstring fastString = L"Fast Gravity Components: [" + std::to_wstring(Gx) + L", " + std::to_wstring(Gy) + L", " + std::to_wstring(Gz) + L"]\n\n";

	//	OutputDebugString(&gravString[0]);
	//	OutputDebugString(&fastString[0]);
	//	break;
	//}
	}
}

void MadgwickTestMode::toggleDisplayData()
{ 
	m_show_live_data = !m_show_live_data;
	std::wstring data_display_message;

	if (m_show_live_data) data_display_message = L"Press Enter to hide live Data from Sensor\nPress Space to Center the Sensor\nPress ^ Arrow to Swap Filter";
	else
	{
		data_display_message = L"Press Enter to see live Data from Sensor\nPress Space to Center the Sensor\nPress ^ Arrow to Swap Filter";

		std::wstring sensor_info_message_one = L"\n";
		std::wstring sensor_info_message_two = L"\n";
		std::wstring sensor_info_message_three = L"\n";
		std::wstring sensor_info_message_four = L"";


		m_uiManager.getElement<TextOverlay>(L"Sensor Info 1 Text")->updateText(sensor_info_message_one + sensor_info_message_two + sensor_info_message_three + sensor_info_message_four);
		m_uiManager.getElement<TextOverlay>(L"Sensor Info 1 Text")->updateColorLocations({ 0,  (unsigned int)sensor_info_message_one.length(), (unsigned int)sensor_info_message_two.length(), (unsigned int)sensor_info_message_three.length(), (unsigned int)sensor_info_message_four.length() });

		m_uiManager.getElement<TextOverlay>(L"Sensor Info 2 Text")->updateText(sensor_info_message_one + sensor_info_message_two + sensor_info_message_three + sensor_info_message_four);
		m_uiManager.getElement<TextOverlay>(L"Sensor Info 2 Text")->updateColorLocations({ 0,  (unsigned int)sensor_info_message_one.length(), (unsigned int)sensor_info_message_two.length(), (unsigned int)sensor_info_message_three.length(), (unsigned int)sensor_info_message_four.length() });
	}

	m_uiManager.getElement<TextOverlay>(L"View Data Text")->updateText(data_display_message);
	m_uiManager.getElement<TextOverlay>(L"View Data Text")->updateColorLocations({ 0,  (unsigned int)data_display_message.length() });
}

void MadgwickTestMode::switchDisplayDataType(int n)
{
	DataType newDataType = static_cast<DataType>(n - 1);;

	switch (n)
	{
	case 1:
		m_display_data_type = L"Acceleration";
		m_display_data_units = L" m/s^2";
		break;
	case 4:
		m_display_data_type = L"Acceleration (uncalibrated)";
		m_display_data_units = L" m/s^2";
		break;
	case 2:
		m_display_data_type = L"Rotation";
		m_display_data_units = L" deg./s";
		break;
	case 5:
		m_display_data_type = L"Rotation (uncalibrated)";
		m_display_data_units = L" deg./s";
		break;
	case 3:
		m_display_data_type = L"Magnetic Field";
		m_display_data_units = L" Gauss";
		break;
	case 6:
		m_display_data_type = L"Magnetic Field (uncalibrated)";
		m_display_data_units = L" Gauss";
		break;
	case 7:
		m_display_data_type = L"Linear Acceleration";
		m_display_data_units = L" m/s^2";
		break;
	}

	//After updating the data type, either turn on or turn off
	//any extrapolated data types as needed
	if (n <= 6)
	{
		if (m_current_data_type == DataType::LINEAR_ACCELERATION || m_current_data_type == DataType::VELOCITY || m_current_data_type == DataType::LOCATION)
			m_mode_screen_handler(ModeAction::PersonalCaddieToggleCalculatedData, (void*)&m_current_data_type);
	}
	else
	{
		//Check to make sure we haven't selected the data type that is currently being displayed
		if (static_cast<int>(m_current_data_type) != (n - 1))
		{
			//Turn off the current data type and turn on the new one
			m_mode_screen_handler(ModeAction::PersonalCaddieToggleCalculatedData, (void*)&m_current_data_type);
			m_mode_screen_handler(ModeAction::PersonalCaddieToggleCalculatedData, (void*)&newDataType);
		}
	}

	m_current_data_type = newDataType;
	m_display_data_index = n - 1;
}

void MadgwickTestMode::getIMUHeadingOffset(glm::quat heading)
{
	//This method gets the heading offset saved in the IMU class and updates
	//the local heading offset variable with it. This variable is used to make
	//sure the rendered image aligns with the computer monitor
	m_headingOffset = heading;
}

void MadgwickTestMode::setCurrentHeadingOffset()
{
	//The Madgwick filter uses due North as a reference for the magnetic field, so if the computer monitor
	//isn't aligned with this direction then the sensor will appear to have an improper heading while being
	//rendered on screen. This function can be called to get the current heading offset from due North
	//which can in turn be used to align the calculated reference direction of North from the Madwick filter
	//to align with the computer monitor. All of the following calculations use the sensor coordinate frame
	//(where +Z axis is up instead of +Y).

	//To calculate the heading offset start with a point vector pointing due north (straight along the +X-axis)
	//by the current rotation quaternion. We then project the resulting rotated vector back into the XY plane
	//by removing itz Z component. We then calculate the angle between true north and the newly projected vector
	//using the dot product, which gives us the heading offset.

	//Rotate the north vector by one of the most current quaternions we have and project it into the 
	//XY plane by removing the Z component of the resultant rotated vector.
	glm::vec3 north = { 1, 0, 0 }, rotated_projected_vector = { 1, 0, 0 };
	QuatRotate(m_quaternions[0], rotated_projected_vector);
	rotated_projected_vector.z = 0;

	//normalize the rotated and projected vector for better use in the dot product equation
	float magnitude = sqrt(rotated_projected_vector.x * rotated_projected_vector.x + rotated_projected_vector.y * rotated_projected_vector.y);
	rotated_projected_vector.x /= magnitude;
	rotated_projected_vector.y /= magnitude;

	//Calculate the angle between north and the vector that's been projected into the XY plane
	//using the dot product equation. Since the largest angle we can get is 180 degrees the acos()
	//method will always give us a positive value. If the y value of the projected and rotated vector
	//is negative then the heading offset angle should also be negative.
	float offset_angle = acos(rotated_projected_vector.x); 
	if (rotated_projected_vector.y < 0) offset_angle *= -1; //we'll only get positive angles with the above equation so correct for that here.

	//Since we need to rotate by the opposite amount of the heading offset to recenter the 
	//image, reverse the polarity of the offset angle while creating the offset quaternion.
	m_headingOffset = { cos(-offset_angle / 2.0f), 0.0f, 0.0f, sin(-offset_angle / 2.0f)};
	m_mode_screen_handler(ModeAction::IMUHeading, (void*)&m_headingOffset);
}

void MadgwickTestMode::convergenceCheck()
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
		m_convergenceQuaternions.clear();
		m_converged = true; //prevents this convergenceCheck() from being called again
	}
}