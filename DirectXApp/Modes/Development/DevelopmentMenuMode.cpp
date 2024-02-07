#include "pch.h"
#include "DevelopmentMenuMode.h"

DevelopmentMenuMode::DevelopmentMenuMode()
{
	//set a gray background color for the mode
	m_backgroundColor = UIColor::Gray;
}

uint32_t DevelopmentMenuMode::initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState)
{
	//Take the current screen size and pass it to the UIElementManager, this is so that the manager knows
	//how large to make each element.
	m_uiManager.updateScreenSize(windowSize);

	//Initialize all overlay text
	initializeTextOverlay();
	initializeToolTips();

	//See if we're currently connected to a Personal Caddie device or not, transferring to some modes from
	//this menu requires it
	std::pair<BLEState, uint64_t> action = { BLEState::Connected, 0 };
	m_mode_screen_handler(ModeAction::BLEConnection, (void*)&action);
	
	//When this mode is initialzed we go into a state of CanTransfer
	return ModeState::CanTransfer;
}

void DevelopmentMenuMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	m_uiManager.removeAllElements();
}

void DevelopmentMenuMode::handleKeyPress(winrt::Windows::System::VirtualKey pressedKey)
{
	//Like the main menu mode the only thing we can really do here with key presses is 
	//transfer to a different mode. The difference here though, is that some of the modes
	//we can traverse to require an active connection to the Personal Caddie device. 

	ModeType newMode = ModeType::DEVELOPER_TOOLS;
	switch (pressedKey)
	{
	case winrt::Windows::System::VirtualKey::Escape:
	{
		newMode = ModeType::MAIN_MENU;
		break;
	}
	case winrt::Windows::System::VirtualKey::Number1:
	{
		newMode = ModeType::UI_TEST_MODE;
		break;
	}
	case winrt::Windows::System::VirtualKey::Number2:
	{
		if (m_connected) newMode = ModeType::GRAPH_MODE;
		else createAlert(L"Must be connected to a Personal Caddie to go to Graph Mode.", UIColor::Red);
		break;
	}
	case winrt::Windows::System::VirtualKey::Number3:
	{
		if (m_connected) newMode = ModeType::MADGWICK;
		else createAlert(L"Must be connected to a Personal Caddie to go to Madgwick Mode.", UIColor::Red);
		break;
	}
	}

	if (newMode != ModeType::DEVELOPER_TOOLS) m_mode_screen_handler(ModeAction::ChangeMode, (void*)&newMode);
}

void DevelopmentMenuMode::initializeTextOverlay()
{
	//Title information
	std::wstring title_message = L"Developer Tools";
	TextOverlay title(m_uiManager.getScreenSize(), {UIConstants::TitleTextLocationX, UIConstants::TitleTextLocationY}, {UIConstants::TitleTextSizeX, UIConstants::TitleTextSizeY},
		title_message, UIConstants::TitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)title_message.length() }, UITextJustification::CenterCenter);
	m_uiManager.addElement<TextOverlay>(title, L"Title Text");

	//Sub-Title information
	std::wstring subtitle_message = L"Modes to aid in the development of the Personal Caddie";
	TextOverlay subtitle(m_uiManager.getScreenSize(), { UIConstants::SubTitleTextLocationX, UIConstants::SubTitleTextLocationY }, { UIConstants::SubTitleTextSizeX, UIConstants::SubTitleTextSizeY },
		subtitle_message, UIConstants::SubTitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)subtitle_message.length() }, UITextJustification::CenterCenter);
	m_uiManager.addElement<TextOverlay>(subtitle, L"Subtitle Text");

	//Body information
	std::wstring body_message_1 = L"1. UI Element Testing";
	std::wstring body_message_2 = L"2. Graph IMU Data";
	std::wstring body_message_3 = L"3. Madgwick Filter Testing";
	
	ClickableTextOverlay body1(m_uiManager.getScreenSize(), { UIConstants::BodyTextLocationX, 0.3f }, { UIConstants::BodyTextSizeX,  0.1f },
		body_message_1, 0.75f, { UIColor::FreeSwingMode }, { 0,  (unsigned int)body_message_1.length() }, UITextJustification::CenterLeft, false);

	ClickableTextOverlay body2(m_uiManager.getScreenSize(), { UIConstants::BodyTextLocationX, 0.4f }, { UIConstants::BodyTextSizeX, 0.1f },
		body_message_2, 0.75f, { UIColor::SwingAnalysisMode }, { 0, (unsigned int)body_message_2.length() }, UITextJustification::CenterLeft, false);

	ClickableTextOverlay body3(m_uiManager.getScreenSize(), { UIConstants::BodyTextLocationX, 0.5f }, { UIConstants::BodyTextSizeX, 0.1f },
		body_message_3, 0.75f, { UIColor::TrainingMode }, { 0, (unsigned int)body_message_3.length() }, UITextJustification::CenterLeft, false);

	m_uiManager.addElement<ClickableTextOverlay>(body1, L"Body Text 1");
	m_uiManager.addElement<ClickableTextOverlay>(body2, L"Body Text 2");
	m_uiManager.addElement<ClickableTextOverlay>(body3, L"Body Text 3");

	//Footnote information
	std::wstring footnote_message = L"Press Esc. to return to main menu";
	TextOverlay footnote(m_uiManager.getScreenSize(), { UIConstants::FootNoteTextLocationX, UIConstants::FootNoteTextLocationY }, { UIConstants::FootNoteTextSizeX, UIConstants::FootNoteTextSizeY },
		footnote_message, UIConstants::FootNoteTextPointSize, { UIColor::White }, { 0,  (unsigned int)footnote_message.length() }, UITextJustification::LowerRight);
	m_uiManager.addElement<TextOverlay>(footnote, L"Footnote Text");
}

void DevelopmentMenuMode::initializeToolTips()
{
	//Create tool tips that contain information about each mode
	std::wstring tool_tip1 = L"A sandbox mode for the development of UI Elements and their logic.";

	std::wstring tool_tip2 = L"This mode allows for graphing of data from the IMU sensors of the Personal Caddie. Individual or multiple data types"
		L" can be graphed simultaneously. This mode is very useful to help develop algorithms that are specific to a golf swing, and also for testing that"
		L" the IMU sensors are operating as expected.";

	std::wstring tool_tip3 = L"Madgwick's filter is used for fusing data from the three IMU sensors together to figure out the current orientation of"
		L" the golf club in space. It's possible to change certain variables to achieve different performance from the filter so this mode is used as"
		L" a means for fine tuning the filter.";

	TextBox textBox1(m_uiManager.getScreenSize(), { 0.7f, 0.6f }, { 0.45f, 0.5f }, tool_tip1, 0.065f,
		{ UIColor::Black }, { 0, (unsigned long long)tool_tip1.length() });
	TextBox textBox2(m_uiManager.getScreenSize(), { 0.7f, 0.6f }, { 0.45f, 0.5f }, tool_tip2, 0.065f,
		{ UIColor::Black }, { 0, (unsigned long long)tool_tip2.length() });
	TextBox textBox3(m_uiManager.getScreenSize(), { 0.7f, 0.6f }, { 0.45f, 0.5f }, tool_tip3, 0.065f,
		{ UIColor::Black }, { 0, (unsigned long long)tool_tip3.length() });

	m_uiManager.addElement<TextBox>(textBox1, L"Tool Tip 1");
	m_uiManager.addElement<TextBox>(textBox2, L"Tool Tip 2");
	m_uiManager.addElement<TextBox>(textBox3, L"Tool Tip 3");

	m_uiManager.getElement<TextBox>(L"Tool Tip 1")->updateState(UIElementState::Invisible);
	m_uiManager.getElement<TextBox>(L"Tool Tip 2")->updateState(UIElementState::Invisible);
	m_uiManager.getElement<TextBox>(L"Tool Tip 3")->updateState(UIElementState::Invisible);
}

void DevelopmentMenuMode::getBLEConnectionStatus(bool status) { m_connected = status; }

void DevelopmentMenuMode::handlePersonalCaddieConnectionEvent(bool connectionStatus)
{
	//In the case that the Personal Caddie becomes asynchronously connected or disconnected
	//while in this mode, this method will get called and updated the m_connected variable.
	//This will in turn allow us, or prevent us, from travelling to certain modes
	m_connected = connectionStatus;
}

void DevelopmentMenuMode::uiElementStateChangeHandler(std::shared_ptr<ManagedUIElement> element)
{
	//When one of the available modes is hovered over it will reveal a tool tip
	//with information about that specific mode.

	//First, make all tool tips invisible
	int tool_tip_number = (int)(element->name.back() - L'0');
	auto tool_tips = m_uiManager.getElementsMap().at(UIElementType::TEXT_BOX);
	for (int i = 0; i < tool_tips.size(); i++)
	{
		if ((int)(tool_tips[i]->name.back() - L'0') == tool_tip_number)
		{
			m_uiManager.getElement<TextBox>(tool_tips[i]->name)->removeState(UIElementState::Invisible);

			//Also check to see if the element passed into this method has the 'released' state.
			//If so we change modes using the keyboard press method.
			if (element->element->getState() & (UIElementState::Released))
			{
				winrt::Windows::System::VirtualKey virtualKey = static_cast<winrt::Windows::System::VirtualKey>(static_cast<int>(winrt::Windows::System::VirtualKey::Number0) + tool_tip_number);
				handleKeyPress(virtualKey);
			}
		}
		else
		{
			m_uiManager.getElement<TextBox>(tool_tips[i]->name)->updateState(UIElementState::Invisible);
		}
	}
}