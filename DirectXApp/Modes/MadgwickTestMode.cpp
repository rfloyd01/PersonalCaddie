#include "pch.h"
#include "MadgwickTestMode.h"
#include "Graphics/Objects/3D/Elements/Face.h"
#include "Math/quaternion_functions.h"

MadgwickTestMode::MadgwickTestMode()
{
	//set a very light gray background color for the mode
	m_backgroundColor = UIColor::FreeSwingMode;
}

uint32_t MadgwickTestMode::initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState)
{
	//Create UI Elements on the page
	initializeTextOverlay(windowSize);

	m_needsCamera = true; //alerts the mode screen that 3d rendering will take place in this mode

	float sensorHeight = 0.5f, sensorLength = 0.3f, sensorWidth = 0.052f;

	std::shared_ptr<Face> sensorTop = std::make_shared<Face>(DirectX::XMFLOAT3(-0.15f, 0.026f, -0.25f), DirectX::XMFLOAT3(0.15f, 0.026f, -0.25f), DirectX::XMFLOAT3(-0.15f, 0.026f, 0.25f));
	std::shared_ptr<Face> sensorLeft = std::make_shared<Face>(DirectX::XMFLOAT3(-0.15f, 0.0f, -0.25f), DirectX::XMFLOAT3(-0.15f, -0.052f, -0.25f), DirectX::XMFLOAT3(-0.15f, 0.0f, 0.25f));
	std::shared_ptr<Face> sensorRight = std::make_shared<Face>(DirectX::XMFLOAT3(0.15f, 0.0f, -0.25f), DirectX::XMFLOAT3(0.15f, -0.052f, -0.25f), DirectX::XMFLOAT3(0.15f, 0.0f, 0.25f));
	std::shared_ptr<Face> sensorFront = std::make_shared<Face>(DirectX::XMFLOAT3(0.15f, -0.026f, 0.25f), DirectX::XMFLOAT3(-0.15f, -0.026f, 0.25f), DirectX::XMFLOAT3(0.15f, 0.026f,0.25f));
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

	m_currentRotation = 0.0f;
	m_currentDegree = PI / 2.0f;
	m_currentQuaternion = 0;

	//Default to acceleration for display data
	m_display_data_index = 0;
	m_display_data_type = L"Acceleration";
	m_display_data_units = L"m/s^2";

	m_quaternions.clear();
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
		m_timeStamps.push_back(0.0f);
		m_display_data[0].push_back(0.0f);
		m_display_data[1].push_back(0.0f);
		m_display_data[2].push_back(0.0f);
	}
	m_renderQuaternion = { m_quaternions[0].x, m_quaternions[0].y, m_quaternions[0].z, m_quaternions[0].w };

	//The NeedMaterial modeState lets the mode screen know that it needs to pass
	//a list of materials to this mode that it can use to initialize 3d objects
	return (ModeState::CanTransfer | ModeState::NeedMaterial | ModeState::Active | ModeState::PersonalCaddieSensorIdleMode);
}

void MadgwickTestMode::setAbsoluteTimer()
{
	//As soon as the Personal Caddie is placed into active mode we will begin receiving
	//data. The moment this happens we start a timer, this will let us keep track of 
	data_start_timer = std::chrono::steady_clock::now();
}

void MadgwickTestMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	
	for (int i = 0; i < m_uiElements.size(); i++) m_uiElements[i] = nullptr;
	m_uiElements.clear();

	for (int i = 0; i < m_volumeElements.size(); i++) m_volumeElements[i] = nullptr;
	m_volumeElements.clear();
}

void MadgwickTestMode::initializeTextOverlay(winrt::Windows::Foundation::Size windowSize)
{
	//Title information
	std::wstring title_message = L"Madgwick Filter Testing";
	TextOverlay title(windowSize, { UIConstants::TitleTextLocationX, UIConstants::TitleTextLocationY }, { UIConstants::TitleTextSizeX, UIConstants::TitleTextSizeY },
		title_message, UIConstants::TitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)title_message.length() }, UITextJustification::CenterCenter);
	m_uiElements.push_back(std::make_shared<TextOverlay>(title));

	//Footnote information
	std::wstring footnote_message = L"Press Esc. to return to settings menu";
	TextOverlay footnote(windowSize, { UIConstants::FootNoteTextLocationX, UIConstants::FootNoteTextLocationY }, { UIConstants::FootNoteTextSizeX, UIConstants::FootNoteTextSizeY },
		footnote_message, UIConstants::FootNoteTextPointSize, { UIColor::White }, { 0,  (unsigned int)footnote_message.length() }, UITextJustification::LowerRight);
	m_uiElements.push_back(std::make_shared<TextOverlay>(footnote));

	//Sensor data display information
	std::wstring sensor_info_message_one = L"\n";
	std::wstring sensor_info_message_two = L"\n";
	std::wstring sensor_info_message_three = L"\n";
	std::wstring sensor_info_message_four = L"";
	TextOverlay sensor_info(windowSize, { UIConstants::SensorInfoTextLocationX, UIConstants::SensorInfoTextLocationY }, { UIConstants::SensorInfoTextSizeX, UIConstants::SensorInfoTextSizeY },
		sensor_info_message_one + sensor_info_message_two + sensor_info_message_three + sensor_info_message_four, UIConstants::SensorInfoTextPointSize * 0.8f, { UIColor::White, UIColor::Red, UIColor::Blue, UIColor::Green },
		{ 0,  (unsigned int)sensor_info_message_one.length(),  (unsigned int)sensor_info_message_two.length(),  (unsigned int)sensor_info_message_three.length(),  (unsigned int)sensor_info_message_four.length() }, UITextJustification::LowerLeft);
	m_uiElements.push_back(std::make_shared<TextOverlay>(sensor_info));

	sensor_info_message_one = L"\n";
	sensor_info_message_two = L"\n";
	sensor_info_message_three = L"\n";
	sensor_info_message_four = L"";
	TextOverlay sensor_info_two(windowSize, { UIConstants::SensorInfoTextLocationX, UIConstants::SensorInfoTextLocationY }, { UIConstants::SensorInfoTextSizeX, UIConstants::SensorInfoTextSizeY },
		sensor_info_message_one + sensor_info_message_two + sensor_info_message_three + sensor_info_message_four, UIConstants::SensorInfoTextPointSize * 0.8f, { UIColor::White, UIColor::Red, UIColor::Blue, UIColor::Green },
		{ 0,  (unsigned int)sensor_info_message_one.length(),  (unsigned int)sensor_info_message_two.length(),  (unsigned int)sensor_info_message_three.length(),  (unsigned int)sensor_info_message_four.length() }, UITextJustification::LowerRight);
	m_uiElements.push_back(std::make_shared<TextOverlay>(sensor_info_two));

	//View data message
	std::wstring view_data_message = L"Press Enter to see live Data from Sensor";
	TextOverlay view_data(windowSize, { UIConstants::FootNoteTextLocationX - 0.33f, UIConstants::FootNoteTextLocationY }, { UIConstants::FootNoteTextSizeX, UIConstants::FootNoteTextSizeY },
		view_data_message, UIConstants::FootNoteTextPointSize, { UIColor::White }, { 0,  (unsigned int)view_data_message.length() }, UITextJustification::LowerCenter);
	m_uiElements.push_back(std::make_shared<TextOverlay>(view_data));
}

void MadgwickTestMode::updateDisplayText()
{
	//This method is called to update the sensor data being displayed on screen
	if (!m_show_live_data) return; //we currently don't want to display data
	if ((m_display_data[0].size() == 0) || (m_display_data[1].size() == 0) || (m_display_data[2].size() == 0)) return; //currently we have no data to display, this can happen as old data is asynchronously overwritten

	std::wstring sensor_info_message_one = m_display_data_type + L":\nSensor Frame\n";
	std::wstring sensor_info_message_two = std::to_wstring(m_display_data[0][m_currentQuaternion]) + m_display_data_units + L"\n";
	std::wstring sensor_info_message_three = std::to_wstring(m_display_data[1][m_currentQuaternion]) + m_display_data_units + L"\n";
	std::wstring sensor_info_message_four = std::to_wstring(m_display_data[2][m_currentQuaternion]) + m_display_data_units + L"\n";
	((TextOverlay*)m_uiElements[2].get())->updateText(sensor_info_message_one + sensor_info_message_two + sensor_info_message_three + sensor_info_message_four);
	((TextOverlay*)m_uiElements[2].get())->updateColorLocations({ 0,  (unsigned int)sensor_info_message_one.length(),  (unsigned int)sensor_info_message_two.length(),  (unsigned int)sensor_info_message_three.length(),  (unsigned int)sensor_info_message_four.length() });

	sensor_info_message_one = L"\nDirectX Frame\n";
	sensor_info_message_two = std::to_wstring(m_display_data[computer_axis_from_sensor_axis[0]][m_currentQuaternion] * sensor_axis_polarity[computer_axis_from_sensor_axis[0]]) + m_display_data_units + L"\n";
	sensor_info_message_three = std::to_wstring(m_display_data[computer_axis_from_sensor_axis[1]][m_currentQuaternion] * sensor_axis_polarity[computer_axis_from_sensor_axis[1]]) + m_display_data_units + L"\n";
	sensor_info_message_four = std::to_wstring(m_display_data[computer_axis_from_sensor_axis[2]][m_currentQuaternion] * sensor_axis_polarity[computer_axis_from_sensor_axis[2]]) + m_display_data_units + L"\n";
	((TextOverlay*)m_uiElements[3].get())->updateText(sensor_info_message_one + sensor_info_message_two + sensor_info_message_three + sensor_info_message_four);
	((TextOverlay*)m_uiElements[3].get())->updateColorLocations({0,  (unsigned int)sensor_info_message_one.length(),  (unsigned int)sensor_info_message_two.length(),  (unsigned int)sensor_info_message_three.length(),  (unsigned int)sensor_info_message_four.length()});
}

uint32_t MadgwickTestMode::handleUIElementStateChange(int i)
{
	if (i == 1)
	{
		return 1;
	}
	return 0;
}

void MadgwickTestMode::addQuaternions(std::vector<glm::quat> const& quaternions, int quaternion_number, float time_stamp, float delta_t)
{
	//It's possible that the quaternions vector will have some empty/junk values at the end. This is because the number of 
	//samples coming from the sensor is dependent on the current sensor ODR (which can change). Because of this we 
	//only add the first quaternion_number quaternions to the vector on this page

	//make sure that the length of the m_quaternion and m_timestamp vectors are the same as the quaternion_number parameter
	if (m_quaternions.size() != quaternion_number)
	{
		m_quaternions.erase(m_quaternions.begin() + quaternion_number, m_quaternions.end());
		m_timeStamps.erase(m_timeStamps.begin() + quaternion_number, m_timeStamps.end());
	}

	m_currentQuaternion = 0; //reset the current quaternion to be rendered
	m_update_in_process = true;

	for (int i = 0; i < quaternion_number; i++)
	{
		m_quaternions[i] = quaternions[i];
		m_timeStamps[i] = time_stamp + i * delta_t;
	}
	m_update_in_process = false;
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
}

void MadgwickTestMode::update()
{
	//Animate the current rotation quaternion obtained from the Personal Caddie. We need to look at the 
	//time stamp to figure out which quaternion is correct. We do this since the ODR of the sensors won't always
	//match up with the frame rate of the current screen.
	if (m_update_in_process) return; //data is currently being updated asynchronously, return and come back later

	float time_elapsed_since_data_start = (float)std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - data_start_timer).count() / 1000000000.0f;
	float quat[3];

	//Due to difference between the refresh rate of the screen and the ODR of the sensor, it may not make sense
	//to render each quaternion. The time_elapsed_since_data_update is used to track which quaternions should
	//actually get rendered.
	for (int i = m_currentQuaternion; i < m_quaternions.size(); i++)
	{
		if (time_elapsed_since_data_start < m_timeStamps[i])
		{
			/*glm::quat correctedQuaternion = QuaternionMultiply(m_offsetQuaternion, m_quaternions[m_currentQuaternion]);
			correctedQuaternion = QuaternionMultiply({ 0.7071f, 0.0f, 0.0f, 0.7071f }, correctedQuaternion);

			quat[0] = correctedQuaternion.x;
			quat[1] = correctedQuaternion.y;
			quat[2] = correctedQuaternion.z;*/

			//m_renderQuaternion = { quat[computer_axis_from_sensor_axis[0]][m_currentQuaternion] * sensor_axis_polarity[computer_axis_from_sensor_axis[0]], quat[axes_swap[1]] * axes_invert[1], quat[axes_swap[2]] * axes_invert[2], correctedQuaternion.w};
			//m_currentQuaternion = i;

			float Q_sensor[3] = { m_quaternions[m_currentQuaternion].x, m_quaternions[m_currentQuaternion].y, m_quaternions[m_currentQuaternion].z };
			float Q_computer[3] = { Q_sensor[computer_axis_from_sensor_axis[0]], Q_sensor[computer_axis_from_sensor_axis[1]], Q_sensor[computer_axis_from_sensor_axis[2]] };

			m_renderQuaternion = { Q_computer[0], Q_computer[1], Q_computer[2], m_quaternions[m_currentQuaternion].w };
			m_currentQuaternion = i;

			//DEBUG
			//std::wstring renderQ = L"[" + std::to_wstring(Q_computer[0]) + L", " + std::to_wstring(Q_computer[1]) + L", " + std::to_wstring(Q_computer[2]) + L", " + std::to_wstring(m_quaternions[m_currentQuaternion].w) + L"]\n";
			//OutputDebugString(&renderQ[0]);

			break;
		}
	}

	//Rotate each face according to the given quaternion
	for (int i = 0; i < m_volumeElements.size(); i++) ((Face*)m_volumeElements[i].get())->translateAndRotateFace({ 0.0f, 0.0f, 1.0f }, m_renderQuaternion);
	
	//Move to the next quaternion. If we've reached the end of the current 
	//set of quaternions just keep rendering the last one in the set
	if (m_currentQuaternion < m_quaternions.size() - 1) m_currentQuaternion++;

	//Update any sensor display text currently being rendered
	updateDisplayText();
}

void MadgwickTestMode::toggleDisplayData()
{ 
	m_show_live_data = !m_show_live_data;
	std::wstring data_display_message;

	if (m_show_live_data) data_display_message = L"Press Enter to hide live Data from Sensor";
	else
	{
		data_display_message = L"Press Enter to see live Data from Sensor";

		std::wstring sensor_info_message_one = L"\n";
		std::wstring sensor_info_message_two = L"\n";
		std::wstring sensor_info_message_three = L"\n";
		std::wstring sensor_info_message_four = L"";


		((TextOverlay*)m_uiElements[2].get())->updateText(sensor_info_message_one + sensor_info_message_two + sensor_info_message_three + sensor_info_message_four);
		((TextOverlay*)m_uiElements[2].get())->updateColorLocations({ 0,  (unsigned int)sensor_info_message_one.length(), (unsigned int)sensor_info_message_two.length(), (unsigned int)sensor_info_message_three.length(), (unsigned int)sensor_info_message_four.length() });

		((TextOverlay*)m_uiElements[3].get())->updateText(sensor_info_message_one + sensor_info_message_two + sensor_info_message_three + sensor_info_message_four);
		((TextOverlay*)m_uiElements[3].get())->updateColorLocations({ 0,  (unsigned int)sensor_info_message_one.length(), (unsigned int)sensor_info_message_two.length(), (unsigned int)sensor_info_message_three.length(), (unsigned int)sensor_info_message_four.length() });
	}

	((TextOverlay*)m_uiElements[4].get())->updateText(data_display_message);
	((TextOverlay*)m_uiElements[4].get())->updateColorLocations({ 0,  (unsigned int)data_display_message.length() });
}

void MadgwickTestMode::switchDisplayDataType(int n)
{
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
	}

	m_display_data_index = n - 1;
}