#include "pch.h"
#include "MainMenuMode.h"

MainMenuMode::MainMenuMode()
{
	//set a black background color for the mode
	m_backgroundColor = UIColor::Black;
}

uint32_t MainMenuMode::initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState)
{
	//Take the current screen size and pass it to the UIElementManager, this is so that the manager knows
	//how large to make each element.
	m_uiManager.updateScreenSize(windowSize);

	initializeTextOverlay();
	initializeToolTips();

	//When this mode is initialzed we go into a state of CanTransfer
	return ModeState::CanTransfer;
}

void MainMenuMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	m_uiManager.removeAllElements();
}

void MainMenuMode::handleKeyPress(winrt::Windows::System::VirtualKey pressedKey)
{
	//The only thing we can do in main menu mode is navigate to a different mode. Simply 
	//figure out what the new mode is from the keypress and navigate there.

	ModeType newMode = ModeType::MAIN_MENU;
	switch (pressedKey)
	{
	case winrt::Windows::System::VirtualKey::Escape:
	{
		//TODO: Pressing the escape key from the main mode should quite out of 
		//the program. I should implement this at some point in the future
		break;
	}
	case winrt::Windows::System::VirtualKey::Number1:
	{
		if (m_connected) newMode = ModeType::FREE;
		else createAlert(L"Must be connected to a Personal Caddie to go to the IMU Settings Mode.", UIColor::Red);
		break;
	}
	case winrt::Windows::System::VirtualKey::Number2:
	{
		//These modes haven't been implemented yet so for now just display
		//an alert letting the user know.
		createAlert(L"This mode hasn't been implemented yet.", UIColor::Red);
		break;
	}
	case winrt::Windows::System::VirtualKey::Number3:
	{
		newMode = ModeType::TRAINING_MENU;
		break;
	}
	case winrt::Windows::System::VirtualKey::Number4:
	{
		newMode = ModeType::SETTINGS_MENU;
		break;
	}
	case winrt::Windows::System::VirtualKey::Number5:
	{
		newMode = ModeType::DEVELOPER_TOOLS;
		break;
	}
	}

	if (newMode != ModeType::MAIN_MENU) m_mode_screen_handler(ModeAction::ChangeMode, (void*)&newMode);
}

void MainMenuMode::initializeTextOverlay()
{
	//Title information
	std::wstring title_message = L"Personal Caddie v1.1";
	TextOverlay title(m_uiManager.getScreenSize(), { UIConstants::TitleTextLocationX, UIConstants::TitleTextLocationY }, { UIConstants::TitleTextSizeX, UIConstants::TitleTextSizeY },
		title_message, UIConstants::TitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)title_message.length() }, UITextJustification::CenterCenter);
	m_uiManager.addElement<TextOverlay>(title, L"Title Text");

	//Sub-Title information
	std::wstring subtitle_message = L"(Press one of the keys listed below to select a mode)";
	TextOverlay subtitle(m_uiManager.getScreenSize(), { UIConstants::SubTitleTextLocationX, UIConstants::SubTitleTextLocationY }, { UIConstants::SubTitleTextSizeX, UIConstants::SubTitleTextSizeY },
		subtitle_message, UIConstants::SubTitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)subtitle_message.length() }, UITextJustification::CenterCenter);
	m_uiManager.addElement<TextOverlay>(subtitle, L"Subtitle Text");

	//Body information
	std::wstring body_message_1 = L"1. Free Swing Mode";
	std::wstring body_message_2 = L"2. Driving Range Mode";
	std::wstring body_message_3 = L"3. Training Mode";
	std::wstring body_message_4 = L"4. Sensor Settings";
	std::wstring body_message_5 = L"5. Developer Tools";

	ClickableTextOverlay body1(m_uiManager.getScreenSize(), { UIConstants::BodyTextLocationX, 0.3f }, { UIConstants::BodyTextSizeX,  0.1f },
		body_message_1, 0.75f, { UIColor::FreeSwingMode}, { 0,  (unsigned int)body_message_1.length() }, UITextJustification::CenterLeft, false);

	ClickableTextOverlay body2(m_uiManager.getScreenSize(), { UIConstants::BodyTextLocationX, 0.4f }, { UIConstants::BodyTextSizeX, 0.1f },
		body_message_2, 0.75f, { UIColor::SwingAnalysisMode }, { 0, (unsigned int)body_message_2.length() }, UITextJustification::CenterLeft, false);

	ClickableTextOverlay body3(m_uiManager.getScreenSize(), { UIConstants::BodyTextLocationX, 0.5f }, { UIConstants::BodyTextSizeX, 0.1f },
		body_message_3, 0.75f, { UIColor::TrainingMode }, { 0, (unsigned int)body_message_3.length() }, UITextJustification::CenterLeft, false);
	
	ClickableTextOverlay body4(m_uiManager.getScreenSize(), { UIConstants::BodyTextLocationX, 0.6f }, { UIConstants::BodyTextSizeX, 0.1f },
		body_message_4, 0.75f, { UIColor::CalibrationMode }, { 0, (unsigned int)body_message_4.length() }, UITextJustification::CenterLeft, false);
	
	ClickableTextOverlay body5(m_uiManager.getScreenSize(), { UIConstants::BodyTextLocationX, 0.7f }, { UIConstants::BodyTextSizeX, 0.1f },
		body_message_5, 0.75f, { UIColor::PaleGray }, { 0, (unsigned int)body_message_5.length() }, UITextJustification::CenterLeft, false);
	
	m_uiManager.addElement<ClickableTextOverlay>(body1, L"Body Text 1");
	m_uiManager.addElement<ClickableTextOverlay>(body2, L"Body Text 2");
	m_uiManager.addElement<ClickableTextOverlay>(body3, L"Body Text 3");
	m_uiManager.addElement<ClickableTextOverlay>(body4, L"Body Text 4");
	m_uiManager.addElement<ClickableTextOverlay>(body5, L"Body Text 5");

	//Footnote information
	std::wstring footnote_message = L"Press Esc. to exit the program.";
	TextOverlay footnote(m_uiManager.getScreenSize(), { UIConstants::FootNoteTextLocationX, UIConstants::FootNoteTextLocationY }, { UIConstants::FootNoteTextSizeX, UIConstants::FootNoteTextSizeY },
		footnote_message, UIConstants::FootNoteTextPointSize, { UIColor::White }, { 0,  (unsigned int)footnote_message.length() }, UITextJustification::LowerRight);
	m_uiManager.addElement<TextOverlay>(footnote, L"Footnote Text");
}

void MainMenuMode::initializeToolTips()
{
	//Create tool tips that contain information about each mode
	std::wstring tool_tip1 = L"The purpose of free swing mode is to hit golf balls and get an array of stats about the swing such as swing path, club head speed, angle of attack, etc."
		L" For each swing you must address the ball and remain still for a second or two until the red light comes on. From here you can swing as normal. Once each distinct phase of the swing"
		L" is entered (backswing, transition, downswing, impact and follow through) a different colored light will illuminate on the screen. Once each swing is complete the stats will display"
		L" on the screen. To take another swing, simply setup to the ball and once again remain stationary for a few seconds.";

	std::wstring tool_tip2 = L"This mode isn't implemented yet, but it will allow you to hit virtual golf balls and give details about the shot such as distance and curve.";

	std::wstring tool_tip3 = L"Training mode contains modules that are meant to train individual parts of the golf swing. From getting the club shallow in the down swing, to squaring the clubface"
		L" at impact and more, you can hone your game with training mode.";

	std::wstring tool_tip4 = L"In settings mode you can do things like scan for and connect/disconnect to nearby Personal Caddie devices, change physical settings on a Personal Caddie device such"
		" as ODR and Full Scale range for the IMU sensors, and calibrate the device to ensure data is accurate.";

	std::wstring tool_tip5 = L"Developer mode contains tools that help with development and debugging of the Personal Caddie. There are separate modes for graphing data from the IMU sensors, tweaking"
		L" of fusion filters, etc. Once developemnt of the Personal Caddie is complete this mode will be removed from the application.";

	TextBox textBox1(m_uiManager.getScreenSize(), { 0.7f, 0.525f }, { 0.45f, 0.5f }, tool_tip1, 0.065f,
		{ UIColor::Black }, { 0, (unsigned long long)tool_tip1.length() });
	TextBox textBox2(m_uiManager.getScreenSize(), { 0.7f, 0.525f }, { 0.45f, 0.5f }, tool_tip2, 0.065f,
		{ UIColor::Black }, { 0, (unsigned long long)tool_tip2.length() });
	TextBox textBox3(m_uiManager.getScreenSize(), { 0.7f, 0.525f }, { 0.45f, 0.5f }, tool_tip3, 0.065f,
		{ UIColor::Black }, { 0, (unsigned long long)tool_tip3.length() });
	TextBox textBox4(m_uiManager.getScreenSize(), { 0.7f, 0.525f }, { 0.45f, 0.5f }, tool_tip4, 0.065f,
		{ UIColor::Black }, { 0, (unsigned long long)tool_tip4.length() });
	TextBox textBox5(m_uiManager.getScreenSize(), { 0.7f, 0.525f }, { 0.45f, 0.5f }, tool_tip5, 0.065f,
		{ UIColor::Black }, { 0, (unsigned long long)tool_tip5.length() });

	m_uiManager.addElement<TextBox>(textBox1, L"Tool Tip 1");
	m_uiManager.addElement<TextBox>(textBox2, L"Tool Tip 2");
	m_uiManager.addElement<TextBox>(textBox3, L"Tool Tip 3");
	m_uiManager.addElement<TextBox>(textBox4, L"Tool Tip 4");
	m_uiManager.addElement<TextBox>(textBox5, L"Tool Tip 5");

	m_uiManager.getElement<TextBox>(L"Tool Tip 1")->updateState(UIElementState::Invisible);
	m_uiManager.getElement<TextBox>(L"Tool Tip 2")->updateState(UIElementState::Invisible);
	m_uiManager.getElement<TextBox>(L"Tool Tip 3")->updateState(UIElementState::Invisible);
	m_uiManager.getElement<TextBox>(L"Tool Tip 4")->updateState(UIElementState::Invisible);
	m_uiManager.getElement<TextBox>(L"Tool Tip 5")->updateState(UIElementState::Invisible);
}

void MainMenuMode::uiElementStateChangeHandler(std::shared_ptr<ManagedUIElement> element)
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

void MainMenuMode::handlePersonalCaddieConnectionEvent(bool connectionStatus)
{
	//In the case that the Personal Caddie becomes asynchronously connected or disconnected
	//while in this mode, this method will get called and updated the m_connected variable.
	//This will in turn allow us, or prevent us, from travelling to certain modes
	m_connected = connectionStatus;
}