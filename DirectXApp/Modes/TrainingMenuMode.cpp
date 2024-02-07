#include "pch.h"
#include "TrainingMenuMode.h"

TrainingMenuMode::TrainingMenuMode()
{
	//set a gray background color for the mode
	m_backgroundColor = UIColor::TrainingModeLight;
}

uint32_t TrainingMenuMode::initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState)
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

void TrainingMenuMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	m_uiManager.removeAllElements();
}

void TrainingMenuMode::handleKeyPress(winrt::Windows::System::VirtualKey pressedKey)
{
	//Like the main menu mode the only thing we can really do here with key presses is 
	//transfer to a different mode. The difference here though, is that some of the modes
	//we can traverse to require an active connection to the Personal Caddie device. 

	ModeType newMode = ModeType::TRAINING_MENU;
	switch (pressedKey)
	{
	case winrt::Windows::System::VirtualKey::Escape:
	{
		newMode = ModeType::MAIN_MENU;
		break;
	}
	case winrt::Windows::System::VirtualKey::Number1:
	{
		if (m_connected) createAlert(L"This mode hasn't been implemented yet.", UIColor::Red);
		else createAlert(L"Must be connected to a Personal Caddie to go to Graph Mode.", UIColor::Red);
		break;
	}
	case winrt::Windows::System::VirtualKey::Number2:
	{
		if (m_connected) createAlert(L"This mode hasn't been implemented yet.", UIColor::Red);
		else createAlert(L"Must be connected to a Personal Caddie to go to Madgwick Mode.", UIColor::Red);
		break;
	}
	case winrt::Windows::System::VirtualKey::Number3:
	{
		if (m_connected) createAlert(L"This mode hasn't been implemented yet.", UIColor::Red);
		else createAlert(L"Must be connected to a Personal Caddie to go to Madgwick Mode.", UIColor::Red);
		break;
	}
	}

	if (newMode != ModeType::TRAINING_MENU) m_mode_screen_handler(ModeAction::ChangeMode, (void*)&newMode);
}

void TrainingMenuMode::initializeTextOverlay()
{
	//Title information
	std::wstring title_message = L"Training Menu";
	TextOverlay title(m_uiManager.getScreenSize(), {UIConstants::TitleTextLocationX, UIConstants::TitleTextLocationY}, {UIConstants::TitleTextSizeX, UIConstants::TitleTextSizeY},
		title_message, UIConstants::TitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)title_message.length() }, UITextJustification::CenterCenter);
	m_uiManager.addElement<TextOverlay>(title, L"Title Text");

	//Sub-Title information
	std::wstring subtitle_message = L"Modes to aid in the Training of the Personal Caddie";
	TextOverlay subtitle(m_uiManager.getScreenSize(), { UIConstants::SubTitleTextLocationX, UIConstants::SubTitleTextLocationY }, { UIConstants::SubTitleTextSizeX, UIConstants::SubTitleTextSizeY },
		subtitle_message, UIConstants::SubTitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)subtitle_message.length() }, UITextJustification::CenterCenter);
	m_uiManager.addElement<TextOverlay>(subtitle, L"Subtitle Text");

	//Body information
	std::wstring body_message_1 = L"1. Swing Path Control";
	std::wstring body_message_2 = L"2. Face Squareness Control";
	std::wstring body_message_3 = L"3. Downswing Shallowness";
	
	ClickableTextOverlay body1(m_uiManager.getScreenSize(), { UIConstants::BodyTextLocationX, 0.3f }, { UIConstants::BodyTextSizeX,  0.1f },
		body_message_1, 0.75f, { UIColor::Brown }, { 0,  (unsigned int)body_message_1.length() }, UITextJustification::CenterLeft, false);

	ClickableTextOverlay body2(m_uiManager.getScreenSize(), { UIConstants::BodyTextLocationX, 0.4f }, { UIConstants::BodyTextSizeX, 0.1f },
		body_message_2, 0.75f, { UIColor::Orange }, { 0, (unsigned int)body_message_2.length() }, UITextJustification::CenterLeft, false);

	ClickableTextOverlay body3(m_uiManager.getScreenSize(), { UIConstants::BodyTextLocationX, 0.5f }, { UIConstants::BodyTextSizeX, 0.1f },
		body_message_3, 0.75f, { UIColor::Yellow }, { 0, (unsigned int)body_message_3.length() }, UITextJustification::CenterLeft, false);

	m_uiManager.addElement<ClickableTextOverlay>(body1, L"Body Text 1");
	m_uiManager.addElement<ClickableTextOverlay>(body2, L"Body Text 2");
	m_uiManager.addElement<ClickableTextOverlay>(body3, L"Body Text 3");

	//Footnote information
	std::wstring footnote_message = L"Press Esc. to return to main menu";
	TextOverlay footnote(m_uiManager.getScreenSize(), { UIConstants::FootNoteTextLocationX, UIConstants::FootNoteTextLocationY }, { UIConstants::FootNoteTextSizeX, UIConstants::FootNoteTextSizeY },
		footnote_message, UIConstants::FootNoteTextPointSize, { UIColor::White }, { 0,  (unsigned int)footnote_message.length() }, UITextJustification::LowerRight);
	m_uiManager.addElement<TextOverlay>(footnote, L"Footnote Text");
}

void TrainingMenuMode::initializeToolTips()
{
	//Create tool tips that contain information about each mode
	for (int i = 0; i < 3; i++)
	{
		std::wstring tip_text;
		if (i == 0)
		{
			tip_text = L"Swing path is the direction that the club head is traveling at impact with regards to the target line. The swing path"
				L" has a large outcome on the curve of the ball, for example swinging over the top of the target line produces a fade while swinging from inside"
				L" the target line produces a draw. This mode will ask you to try and swing the club through a certain window, and will get harder as you improve.";
		}
		else if (i == 1)
		{
			tip_text = L"The direction that the club face points at impact dictates the majority of the direction for the ball flight. If the club face"
				L" is pointing left of the target line then the ball will go to the left, if the club face is pointint to the right then the ball will go"
				L" to the right, etc. This training module will tell you how many degrees open or closed the clubface should be at impact and you need"
				L" to try and match it.";
		}
		else if (i == 2)
		{
			tip_text = L"Having a shallow club shaft during the downsing is beneficial for a few reasons. Namely, it promotes a good angle of attack into"
				L" the ball at impact which produces a low ball flight that will rise over time, but it will also reduce the negative effects of hitting a fat"
				L" shot. In this mode when you line up to the ball two lines will appear on teh screen. The goal is to have the club stay between these"
				L" two lines during the down swing";
		}

		TextBox textBox(m_uiManager.getScreenSize(), { 0.75f, 0.55f }, { 0.45f, 0.5f }, tip_text, 0.065f,
			{ UIColor::Black }, { 0, (unsigned long long)tip_text.length() });

		m_uiManager.addElement<TextBox>(textBox, L"Tool Tip " + std::to_wstring(i + 1));
		m_uiManager.getElement<TextBox>(L"Tool Tip " + std::to_wstring(i + 1))->updateState(UIElementState::Invisible);
	}
}

void TrainingMenuMode::getBLEConnectionStatus(bool status) { m_connected = status; }

void TrainingMenuMode::handlePersonalCaddieConnectionEvent(bool connectionStatus)
{
	//In the case that the Personal Caddie becomes asynchronously connected or disconnected
	//while in this mode, this method will get called and updated the m_connected variable.
	//This will in turn allow us, or prevent us, from travelling to certain modes
	m_connected = connectionStatus;
}

void TrainingMenuMode::uiElementStateChangeHandler(std::shared_ptr<ManagedUIElement> element)
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