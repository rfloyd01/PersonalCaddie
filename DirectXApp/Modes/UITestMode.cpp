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
	initializeTextOverlay(windowSize);

	//Graph Testing
	/*Graph test_graph(windowSize, { 0.5f, 0.5F }, { 0.5f, 0.5f });
	std::vector<DirectX::XMFLOAT2> test_data_1, test_data_2, test_data_3 = { {0.0f, 1.25f}, {2.0f * PI, 1.25f} };

	for (int i = 0; i < 1000; i++)
	{
		test_data_1.push_back({ (float)(i * 2.0f * PI / 1000.0f), (float)sin(i * 2.0f * PI / 1000.0f) });
		test_data_2.push_back({ (float)(i * 2.0f * PI / 1000.0f), (float)cos(i * 2.0f * PI / 1000.0f) });
	}

	test_graph.setAxisMaxAndMins({ 0.0f, -1.5f }, { 2.0f * PI, 1.5f });
	test_graph.addGraphData(windowSize, test_data_1, UIColor::Red);
	test_graph.addGraphData(windowSize, test_data_2, UIColor::Blue);
	test_graph.addGraphData(windowSize, test_data_3, UIColor::Green);

	m_uiManager.addElement<Graph>(test_graph, L"Test Graph");*/

	//CheckBox test_check(windowSize, { 0.33f, 0.9f }, { 0.1f, 0.1f });
	//m_uiManager.addElement<CheckBox>(test_check, L"Test Check Box");

	RelativeBox relative_box_1(windowSize, { 0.471875f, 0.45f }, { 0.05625f, 0.1f });
	RelativeBox relative_box_2(windowSize, { 0.528125f, 0.45f }, { 0.05625f, 0.1f }, UIColor::White);
	RelativeBox relative_box_3(windowSize, { 0.584375f, 0.35f}, { 0.05625f, 0.1f }, UIColor::Gray); //0.371875f
	RelativeBox relative_box_4(windowSize, { 0.471875f, 0.55f }, { 0.05625f, 0.1f }, UIColor::Blue);
	RelativeBox relative_box_5(windowSize, { 0.528125f, 0.55f }, { 0.05625f, 0.1f }, UIColor::Red);
	RelativeBox relative_box_6(windowSize, { 0.6f, 0.55f }, { 0.05625f, 0.1f }, UIColor::DarkGray);

	Box box_1(windowSize, { 0.4f, 0.4f }, { 0.0772f, 0.1f }, UIColor::Blue);
	Box box_2(windowSize, { 0.4f, 0.6f }, { 0.0772f, 0.1f });
	Box box_3(windowSize, { 0.6f, 0.4f }, { 0.0772f, 0.1f }, UIColor::White);
	Box box_4(windowSize, { 0.6f, 0.6f }, { 0.0772f, 0.1f }, UIColor::Gray);

	Line line1(windowSize, { 0.5f, 0.25f }, { 0.5f, 0.95f }, UIColor::Red);
	Line line2(windowSize, { 0.05f, 0.5f }, { 0.95f, 0.5f }, UIColor::Red);

	m_uiManager.addElement<RelativeBox>(relative_box_1, L"Relative Box 1");
	m_uiManager.addElement<RelativeBox>(relative_box_2, L"Relative Box 2");
	m_uiManager.addElement<RelativeBox>(relative_box_3, L"Relative Box 3");
	m_uiManager.addElement<RelativeBox>(relative_box_4, L"Relative Box 4");
	m_uiManager.addElement<RelativeBox>(relative_box_5, L"Relative Box 5");
	//m_uiManager.addElement<RelativeBox>(relative_box_6, L"Relative Box 6");

	/*m_uiManager.addElement<Box>(box_1, L"Box 1");
	m_uiManager.addElement<Box>(box_2, L"Box 2");
	m_uiManager.addElement<Box>(box_3, L"Box 3");
	m_uiManager.addElement<Box>(box_4, L"Box 4");*/

	//m_uiManager.addElement<Line>(line1, L"Line 1");
	//m_uiManager.addElement<Line>(line2, L"Line 2");

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