#include "pch.h"
#include "UITestMode.h"

#include "Graphics/Objects/3D/Elements/model.h"
#include "Math/quaternion_functions.h"

UITestMode::UITestMode()
{
	//set a very light gray background color for the mode
	m_backgroundColor = UIColor::LightBlue;
}

uint32_t UITestMode::initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState)
{
	//Take the current screen size and pass it to the UIElementManager, this is so that the manager knows
	//how large to make each element.
	m_uiManager.updateScreenSize(windowSize);

	//Create UI Text Elements on the page
	initializeTextOverlay(windowSize);

	DropDownMenu acc_menu(windowSize, { 0.5, 0.47 }, { 0.15, 0.1 }, L"One\nTwo\nThree\nFour\nFive", 0.0225); //the locations will get set by a separate method
	DropDownMenu gyr_menu(windowSize, { 0.5, 0.43 }, { 0.15, 0.1 }, L"LongerOne\nLongerTwo\nLongerThree\nLongerFour\nLongerFive", 0.0225); //the locations will get set by a separate method
	DropDownMenu mag_menu(windowSize, { 0.85, 0.47 }, { 0.25, 0.15 }, L"One\nTwo\nThree\nFour\nFive", 0.0225); //the locations will get set by a separate method
	
	m_uiManager.addElement<DropDownMenu>(gyr_menu, L"Gyr Model Drop Down Menu");
	m_uiManager.addElement<DropDownMenu>(acc_menu, L"Acc Model Drop Down Menu");
	m_uiManager.addElement<DropDownMenu>(mag_menu, L"Mag Model Drop Down Menu");

	//Create a Model volume element
	m_volumeElements.push_back(std::make_shared<Model>());
	((Model*)m_volumeElements[0].get())->loadModel("Assets/Models/golf_club.gltf");

	//Set a test material
	m_materialTypes.push_back(MaterialType::SENSOR_TOP);

	//Set the mesh and materials for the model
	m_mode_screen_handler(ModeAction::RendererGetMaterial, nullptr);

	//ALlow 3d rendering
	m_needsCamera = true;

	//When this mode is initialzed we go into a state of CanTransfer and Active.
	//Can Transfer allows us to use the esc. key to go back to the settings menu
	//while active diverts state control to this mode
	//return (ModeState::CanTransfer | ModeState::NeedTextUpdate);
	return ModeState::CanTransfer;
}

void UITestMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	
	//for (int i = 0; i < m_uiElements.size(); i++) m_uiElements[i] = nullptr;
	//m_uiElements.clear();

	m_uiManager.removeAllElements();
	m_volumeElements.clear();
}

void UITestMode::initializeTextOverlay(winrt::Windows::Foundation::Size windowSize)
{
	//Title information
	std::wstring title_message = L"UI Testing";
	HighlightableTextOverlay title(windowSize, { UIConstants::TitleTextLocationX, UIConstants::TitleTextLocationY }, { UIConstants::TitleTextSizeX, UIConstants::TitleTextSizeY },
		title_message, UIConstants::TitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)title_message.length() }, UITextJustification::CenterCenter);
	title.updateSecondaryColor(UIColor::Red);
	m_uiManager.addElement<HighlightableTextOverlay>(title, L"Title");

	//Sub-Title information
	std::wstring subtitle_message = L"A place to develop custom UI Elements (hover over the title to see some stuff in action!)";
	TextOverlay subtitle(windowSize, { UIConstants::SubTitleTextLocationX, UIConstants::SubTitleTextLocationY }, { UIConstants::SubTitleTextSizeX, UIConstants::SubTitleTextSizeY },
		subtitle_message, UIConstants::SubTitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)subtitle_message.length() }, UITextJustification::CenterCenter);
	m_uiManager.addElement<TextOverlay>(subtitle, L"Sub-Title");

	//Footnote information
	std::wstring footnote_message = L"Press Esc. to return to settings menu";
	TextOverlay footnote(windowSize, { UIConstants::FootNoteTextLocationX, UIConstants::FootNoteTextLocationY }, { UIConstants::FootNoteTextSizeX, UIConstants::FootNoteTextSizeY },
		footnote_message, UIConstants::FootNoteTextPointSize, { UIColor::White }, { 0,  (unsigned int)footnote_message.length() }, UITextJustification::LowerRight);
	m_uiManager.addElement<TextOverlay>(footnote, L"Footnote");
}

void UITestMode::update()
{
	//Check the action array of the UIElement Manager to see if anything needs addressing
	if (m_uiManager.getActionElements().size() > 0)
	{
		//iterate backwards so we can pop each action from the back when complete
		for (int i = m_uiManager.getActionElements().size() - 1; i >= 0; i--)
		{
			auto action = m_uiManager.getActionElements()[i];
			m_uiManager.getActionElements().pop_back(); //TODO: For now just remove elements, need to implement actions in the future though
		}
	}

	if (m_needsCamera)
	{
		//Rotate the club model on screen. The goal is to rotate 360 degrees over the span of 5 seconds. Since the refresh rate
		//of the monitor is 1/60 s then we need to rotate 360 deg / 5s ==  60s * x deg --> x =  1.2 deg / frame to achieve this
		
		m_angle += 1.2f * PI / 180.0f;
		glm::quat first_rotation = { 0.707f, 0.707f, 0.0f, 0.0f };
		glm::quat second_rotation = { cos(m_angle / 2.0f), 0.0f, 0.0f, sin(m_angle / 2.0f) };
		auto q = QuaternionMultiply(second_rotation, first_rotation); //reverse order for rotations
		((Model*)m_volumeElements[0].get())->translateAndRotateFace({ 0.0f, -0.25f, 1.0f }, { q.x, q.y, q.z, q.w});
	}
}

void UITestMode::handleKeyPress(winrt::Windows::System::VirtualKey pressedKey)
{
	//The only key we can press in this mode is the escape key. All this key does is exit the 
	//mode and go back to the development menu.
	if (pressedKey == winrt::Windows::System::VirtualKey::Escape)
	{
		ModeType newMode = ModeType::DEVELOPER_TOOLS;
		m_mode_screen_handler(ModeAction::ChangeMode, (void*)&newMode);
	}
}