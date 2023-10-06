#include "pch.h"
#include "MadgwickTestMode.h"
#include "Graphics/Objects/3D/Elements/Face.h"

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

	std::shared_ptr<Face> sensorTop = std::make_shared<Face>(DirectX::XMFLOAT3(-0.15f, -0.25f, -0.026f), DirectX::XMFLOAT3(0.15f, -0.25f, -0.026f), DirectX::XMFLOAT3(-0.15f, 0.25f, -0.026f));
	std::shared_ptr<Face> sensorLongSideOne = std::make_shared<Face>(DirectX::XMFLOAT3(-0.15f, -0.25f, 0.026f ), DirectX::XMFLOAT3(-0.15f, -0.25f, -0.026f ), DirectX::XMFLOAT3(-0.15f, 0.25f, 0.026f));
	std::shared_ptr<Face> sensorLongSideTwo = std::make_shared<Face>(DirectX::XMFLOAT3(0.15f, -0.25f, -0.026f), DirectX::XMFLOAT3(0.15f, -0.25f, 0.026f), DirectX::XMFLOAT3(0.15f, 0.25f, -0.026f));
	std::shared_ptr<Face> sensorShortSideOne = std::make_shared<Face>(DirectX::XMFLOAT3(-0.15f, 0.25f, -0.026f), DirectX::XMFLOAT3(0.15f, 0.25f, -0.026f), DirectX::XMFLOAT3(-0.15f, 0.25f, 0.026f));
	std::shared_ptr<Face> sensorShortSideTwo = std::make_shared<Face>(DirectX::XMFLOAT3(-0.15f, -0.25f, 0.026f), DirectX::XMFLOAT3(0.15f, -0.25f, 0.026), DirectX::XMFLOAT3(-0.15f, -0.25f, -0.026f));
	std::shared_ptr<Face> sensorBottom = std::make_shared<Face>(DirectX::XMFLOAT3(-0.15f, -0.25f, 0.026f), DirectX::XMFLOAT3(0.15f, -0.25f, 0.026f), DirectX::XMFLOAT3(-0.15f, 0.25f, 0.026f));

	m_volumeElements.push_back(sensorTop);
	m_volumeElements.push_back(sensorLongSideOne);
	m_volumeElements.push_back(sensorLongSideTwo);
	m_volumeElements.push_back(sensorShortSideOne);
	m_volumeElements.push_back(sensorShortSideTwo);
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

	current_time = std::chrono::steady_clock::now();

	//Load the quaternion vector with default quaternions
	for (int i = 0; i < 10; i++) m_quaternions.push_back({ 1.0f, 0.0f, 0.0f, 0.0f });

	//The NeedMaterial modeState lets the mode screen know that it needs to pass
	//a list of materials to this mode that it can use to initialize 3d objects
	return (ModeState::CanTransfer | ModeState::NeedMaterial | ModeState::Active | ModeState::PersonalCaddieSensorIdleMode);
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

void MadgwickTestMode::addQuaternions(std::vector<glm::quat> const& quaternions)
{
	m_currentQuaternion = 0; //reset the current quaternion to be rendered
	m_quaternions.clear(); //clear out existing quaternions
	for (int i = 0; i < quaternions.size(); i++) m_quaternions.push_back(quaternions[i]);
}

void MadgwickTestMode::update()
{
	//Animate the current rotation quaternion obtained from the Personal Caddie
	DirectX::XMVECTOR q({ m_quaternions[m_currentQuaternion].x, m_quaternions[m_currentQuaternion].y, m_quaternions[m_currentQuaternion].z, m_quaternions[m_currentQuaternion].w });

	//Rotate each face according to the given quaternion
	for (int i = 0; i < m_volumeElements.size(); i++) ((Face*)m_volumeElements[i].get())->translateAndRotateFace({ 0.0f, 0.0f, 1.0f }, q);
	
	//Move to the next quaternion. If we've reached the end of the current 
	//set of quaternions just keep rendering the last one in the set
	if (m_currentQuaternion < m_quaternions.size() - 1) m_currentQuaternion++;
}