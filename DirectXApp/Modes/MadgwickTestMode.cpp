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

	//std::shared_ptr<Face> target = std::make_shared<Face>(DirectX::XMFLOAT3(-2.5f, -1.0f, 1.5f), DirectX::XMFLOAT3(-1.5f, -1.0f, 2.0f), DirectX::XMFLOAT3(-2.5f, 1.0f, 1.5f));
	std::shared_ptr<Face> target = std::make_shared<Face>(DirectX::XMFLOAT3(-0.25f, 0.25f, 1.0f), DirectX::XMFLOAT3(0.25f, 0.25f, 1.0f), DirectX::XMFLOAT3(-0.25f, -0.25f, 1.0f));
	m_volumeElements.push_back(target);

	//The NeedMaterial modeState lets the mode screen know that it needs to pass
	//a list of materials to this mode that it can use to initialize 3d objects
	return (ModeState::CanTransfer | ModeState::NeedMaterial);
}

void MadgwickTestMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	
	for (int i = 0; i < m_uiElements.size(); i++) m_uiElements[i] = nullptr;
	m_uiElements.clear();
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

void MadgwickTestMode::pickMaterial(std::vector<std::shared_ptr<Material>> const& materials)
{
	//In this mode we render a model of the current sesnor, so all materials having to do 
	//with different sensors are picked here.
	if (materials.size() == 0)
	{
		//the materials haven't actually been loaded yet so don't update anything. This method
		//will keep getting called in the main render loop until the materials are actually
		//loaded
		int x = 5;
		return;
	}
	else
	{
		m_volumeElements[0]->setMaterial(materials[0]);
	}
}

uint32_t MadgwickTestMode::handleUIElementStateChange(int i)
{
	if (i == 1)
	{
		return 1;
	}
	return 0;
}