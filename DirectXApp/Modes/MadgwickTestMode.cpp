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

void MadgwickTestMode::update()
{
	//for now, rotate the sensor by a small amount
	//auto rando_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	DirectX::XMVECTOR q({ 0.0f, sinf(PI / 4.0f), 0.0f, cosf(PI / 4.0f) });

	for (int i = 0; i < m_volumeElements.size(); i++)
	{
		//((Face*)m_volumeElements[i].get())->translateAndRotateFace({ sinf(m_currentDegree), 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}, m_currentDegree);
		((Face*)m_volumeElements[i].get())->translateAndRotateFace({ 0.0f, 0.0f, 1.0f }, q);
	}

	m_currentDegree += PI / 180.0f;
}