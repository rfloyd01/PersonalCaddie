#include "pch.h"
#include "UITestMode.h"

#include "Graphics/Objects/3D/Elements/model.h"
#include "Graphics/Objects/2D/Buttons/CheckBox.h"
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
	initializeTextOverlay();

	float screen_ratio = MAX_SCREEN_HEIGHT / MAX_SCREEN_WIDTH;

	std::wstring scrollText = L"Once upon a time\nit was the best of times\nit was the worst of times\nOnce upon a time\nit was the best of times\nit was the worst of times\nyeet\nyeet\nyote";
	DropDownMenu deviceWatcherResults(m_uiManager.getScreenSize(), { 0.65f, 0.575f }, { 0.5f, 0.35f }, scrollText, 0.125f);
	m_uiManager.addElement<DropDownMenu>(deviceWatcherResults, L"Device Watcher Text Box");
	//m_uiManager.drawDebugOutline(m_uiManager.getElement<FullScrollingTextBox>(L"Device Watcher Text Box"), false);

	TextButton shrink(m_uiManager.getScreenSize(), { 0.15f, 0.35f }, { 0.15f, 0.15f }, L"Shrink");
	TextButton toggle(m_uiManager.getScreenSize(), { 0.15f, 0.75f }, { 0.15f, 0.15f }, L"Toggle Outline");
	CheckBox box1(m_uiManager.getScreenSize(), { 0.75f, 0.5f }, { screen_ratio * 0.15f, 0.15f });
	OutlinedBox box2(m_uiManager.getScreenSize(), { 0.75f, 0.6f }, { 0.1f, 0.1f });

	m_uiManager.addElement<TextButton>(shrink, L"Button 1");
	m_uiManager.addElement<TextButton>(toggle, L"Button 2");
	//m_uiManager.addElement<CheckBox>(box1, L"Device Watcher Text Box");
	//m_uiManager.addElement<OutlinedBox>(box2, L"Box 2");

	return ModeState::CanTransfer;
}

void UITestMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	m_uiManager.removeAllElements();
	m_volumeElements.clear();
}

void UITestMode::initializeTextOverlay()
{
	//Title information
	std::wstring title_message = L"UI Testing";
	HighlightableTextOverlay title(m_uiManager.getScreenSize(), { UIConstants::TitleTextLocationX, UIConstants::TitleTextLocationY }, { UIConstants::TitleTextSizeX, UIConstants::TitleTextSizeY },
		title_message, UIConstants::TitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)title_message.length() }, UITextJustification::CenterCenter);
	title.updateSecondaryColor(UIColor::Red);
	m_uiManager.addElement<HighlightableTextOverlay>(title, L"Title");

	//Sub-Title information
	std::wstring subtitle_message = L"A place to develop custom UI Elements (hover over the title to see some stuff in action!)";
	TextOverlay subtitle(m_uiManager.getScreenSize(), { UIConstants::SubTitleTextLocationX, UIConstants::SubTitleTextLocationY }, { UIConstants::SubTitleTextSizeX, UIConstants::SubTitleTextSizeY },
		subtitle_message, UIConstants::SubTitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)subtitle_message.length() }, UITextJustification::CenterCenter);
	m_uiManager.addElement<TextOverlay>(subtitle, L"Sub-Title");

	//Footnote information
	std::wstring footnote_message = L"Press Esc. to return to settings menu";
	TextOverlay footnote(m_uiManager.getScreenSize(), { UIConstants::FootNoteTextLocationX, UIConstants::FootNoteTextLocationY }, { UIConstants::FootNoteTextSizeX, UIConstants::FootNoteTextSizeY },
		footnote_message, UIConstants::FootNoteTextPointSize, { UIColor::White }, { 0,  (unsigned int)footnote_message.length() }, UITextJustification::LowerRight);
	m_uiManager.addElement<TextOverlay>(footnote, L"Footnote");
}

void UITestMode::uiElementStateChangeHandler(std::shared_ptr<ManagedUIElement> element)
{
	if (element->name == L"Button 1")
	{
		auto currentSize = m_uiManager.getElement<DropDownMenu>(L"Device Watcher Text Box")->getAbsoluteSize();
		m_uiManager.getElement<DropDownMenu>(L"Device Watcher Text Box")->setAbsoluteSize({ currentSize.x / 1.2f, currentSize.y / 1.2f }, true);
	}
	if (element->name == L"Button 2")
	{
		m_uiManager.drawDebugOutline(m_uiManager.getElement<DropDownMenu>(L"Device Watcher Text Box"), false);
	}
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