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

	m_quaternions.clear();
	m_timeStamps.clear();

	//Load the quaternion vector with default quaternions and the 
	//time stamp vector with default times. TODO: Shouldn't use a 
	//loop to 10, should instead use the SENSOR_READINGS variable
	for (int i = 0; i < 10; i++)
	{
		m_quaternions.push_back({ 1.0f, 0.0f, 0.0f, 0.0f });
		m_timeStamps.push_back(0.0f);
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
	m_currentQuaternion = 0; //reset the current quaternion to be rendered
	m_quaternions.clear(); //clear out existing quaternions
	m_timeStamps.clear();
	for (int i = 0; i < quaternion_number; i++)
	{
		m_quaternions.push_back(quaternions[i]);
		m_timeStamps.push_back(time_stamp + i * delta_t);
	}
}

void MadgwickTestMode::update()
{
	//Animate the current rotation quaternion obtained from the Personal Caddie. We need to look at the 
	//time stamp to figure out which quaternion is correct. We do this since the ODR of the sensors won't always
	//match up with the frame rate of the current screen.
	float time_elapsed_since_data_start = (float)std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - data_start_timer).count() / 1000000000.0f;

	float quat[3];

	//Due to difference between the refresh rate of the screen and the ODR of the sensor, it may not make sense
	//to render each quaternion. The time_elapsed_since_data_update is used to track which quaternions should
	//actually get rendered.
	for (int i = m_currentQuaternion; i < m_quaternions.size(); i++)
	{
		if (time_elapsed_since_data_start < m_timeStamps[i])
		{
			glm::quat correctedQuaternion = QuaternionMultiply(m_offsetQuaternion, m_quaternions[m_currentQuaternion]);
			correctedQuaternion = QuaternionMultiply({ 0.7071f, 0.0f, 0.0f, 0.7071f }, correctedQuaternion);

			quat[0] = correctedQuaternion.x;
			quat[1] = correctedQuaternion.y;
			quat[2] = correctedQuaternion.z;

			m_renderQuaternion = { quat[axes_swap[0]] * axes_invert[0], quat[axes_swap[1]] * axes_invert[1], quat[axes_swap[2]] * axes_invert[2], correctedQuaternion.w};
			m_currentQuaternion = i;
			/*std::wstring displayed = L"Displayed quaterion " + std::to_wstring(i) + L".\n";
			displayed += L"Current Time Stamp: " + std::to_wstring(time_elapsed_since_data_update) + L".\n";
			displayed += L"ODR Time Stamp: " + std::to_wstring(m_timeStamps[i]) + L".\n\n";
			OutputDebugString(&displayed[0]);*/

			std::wstring displayed = L"Displayed quaterion: [" + std::to_wstring(quat[axes_swap[0]] * axes_invert[0]) + L", " + std::to_wstring(quat[axes_swap[1]] * axes_invert[1]) + L", "
				+ std::to_wstring(quat[axes_swap[2]] * axes_invert[2]) + L", " + std::to_wstring(correctedQuaternion.w) + L"].\n";

			displayed += L"Actual quaterion: [" + std::to_wstring(m_quaternions[m_currentQuaternion].x) + L", " + std::to_wstring(m_quaternions[m_currentQuaternion].y) + L", "
				+ std::to_wstring(m_quaternions[m_currentQuaternion].z) + L", " + std::to_wstring(m_quaternions[m_currentQuaternion].w) + L"].\n\n";
			OutputDebugString(&displayed[0]);
			break;
		}
	}

	//Rotate each face according to the given quaternion
	for (int i = 0; i < m_volumeElements.size(); i++) ((Face*)m_volumeElements[i].get())->translateAndRotateFace({ 0.0f, 0.0f, 1.0f }, m_renderQuaternion);
	
	//Move to the next quaternion. If we've reached the end of the current 
	//set of quaternions just keep rendering the last one in the set
	if (m_currentQuaternion < m_quaternions.size() - 1) m_currentQuaternion++;
}