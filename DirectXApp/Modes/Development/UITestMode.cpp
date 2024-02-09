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
	Graph graph(m_uiManager.getScreenSize(), { 0.15f, 0.4f }, { screen_ratio * 0.25f, 0.25f }, false, UIColor::White, UIColor::Black, false, false);

	graph.setAxisMaxAndMins({ -1.0f, -1.0f }, { 1.0f, 1.0f });

	std::vector<DirectX::XMFLOAT2> data1, data2, data3;
	for (float i = -0.5f; i <= 0.5f; i += 0.01f)
	{
		data1.push_back({ i, i / 3.0f });
		data2.push_back({ i, i / -2.0f });
	}

	for (float i = -PI; i <= PI; i += PI / 50.0f) data3.push_back({ sin(i) / 10.0f, cos(i) / 10.0f });

	graph.addGraphData(data3, UIColor::Black, L"Golf Ball");
	graph.addGraphData(data1, UIColor::Green, L"Goal Line");
	graph.addGraphData(data2, UIColor::Red, L"Actual Line");

	m_uiManager.addElement<Graph>(graph, L"Graph 1");

	m_uiManager.getElement<Graph>(L"Graph 1")->addAxisLine(0, 0.0f);
	m_uiManager.getElement<Graph>(L"Graph 1")->addAxisLine(1, 0.0f);

	std::wstring options = L"Game Mode\nTraining Mode";
	DropDownMenu mode_selection(m_uiManager.getScreenSize(), { 0.15f, 0.85f }, { 0.2f, 0.08f }, options, 0.5f, 2);

	m_uiManager.addElement<DropDownMenu>(mode_selection, L"Dropdown Menu 1");

	std::wstring level_message = L"Current Level: 2";
	std::wstring threshold_message = L"Degree Threshold: +/- 5";
	std::wstring streak_message = L"Current Streak: 3";
	std::wstring swing_message = L"Swings to Next Level: 3";
	std::wstring goal_message = L"Current Goal: -2 deg.";

	TextOverlay level(m_uiManager.getScreenSize(), { 0.85f, 0.35f }, { 0.2f, 0.08f }, level_message, UIConstants::SubTitleTextPointSize,
		{ UIColor::White }, { 0,  (unsigned int)level_message.length() }, UITextJustification::CenterLeft);
	TextOverlay threshold(m_uiManager.getScreenSize(), { 0.85f, 0.425f }, { 0.2f, 0.08f }, threshold_message, UIConstants::SubTitleTextPointSize,
		{ UIColor::White }, { 0,  (unsigned int)threshold_message.length() }, UITextJustification::CenterLeft);
	TextOverlay streak(m_uiManager.getScreenSize(), { 0.85f, 0.50f }, { 0.2f, 0.08f }, streak_message, UIConstants::SubTitleTextPointSize,
		{ UIColor::White }, { 0,  (unsigned int)streak_message.length() }, UITextJustification::CenterLeft);
	TextOverlay swing(m_uiManager.getScreenSize(), { 0.85f, 0.575f }, { 0.2f, 0.08f }, swing_message, UIConstants::SubTitleTextPointSize,
		{ UIColor::White }, { 0,  (unsigned int)swing_message.length() }, UITextJustification::CenterLeft);
	TextOverlay goal(m_uiManager.getScreenSize(), { 0.85f, 0.65f }, { 0.2f, 0.08f }, goal_message, UIConstants::SubTitleTextPointSize,
		{ UIColor::White }, { 0,  (unsigned int)goal_message.length() }, UITextJustification::CenterLeft);

	/*m_uiManager.addElement<TextOverlay>(level, L"Game Message 1");
	m_uiManager.addElement<TextOverlay>(threshold, L"Game Message 2");
	m_uiManager.addElement<TextOverlay>(streak, L"Game Message 3");
	m_uiManager.addElement<TextOverlay>(swing, L"Game Message 4");
	m_uiManager.addElement<TextOverlay>(goal, L"Game Message 5");*/

	std::wstring threshold_options = L"+/- 1\n+/- 5\n+ 3\n- 2\n- 5";
	DropDownMenu threshold_selection(m_uiManager.getScreenSize(), { 0.85f, 0.425f }, { 0.2f, 0.08f }, threshold_options, 0.5f, 2);

	std::wstring target_options = L"-10 deg.\n-5 deg.\n0 deg.\n2 deg.\n4 deg.";
	DropDownMenu target_selection(m_uiManager.getScreenSize(), { 0.85f, 0.525f }, { 0.2f, 0.08f }, target_options, 0.5f, 2);

	m_uiManager.addElement<DropDownMenu>(threshold_selection, L"Dropdown Menu 2");
	m_uiManager.addElement<DropDownMenu>(target_selection, L"Dropdown Menu 3");

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
	TextOverlay subtitle(m_uiManager.getScreenSize(), { UIConstants::SubTitleTextLocationX, UIConstants::SubTitleTextLocationY - 0.04f }, { UIConstants::SubTitleTextSizeX, UIConstants::SubTitleTextSizeY },
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
		auto currentSize = m_uiManager.getElement<Graph>(L"Graph 1")->getAbsoluteSize();
		m_uiManager.getElement<Graph>(L"Graph 1")->setAbsoluteSize({ currentSize.x * 1.2f, currentSize.y * 1.2f }, true);
		m_uiManager.refreshGrid(); //Any top level calls to setAbsoluteSize() require the ui manager's grid system to be refreshed.
	}
	if (element->name == L"Button 2")
	{
		m_uiManager.drawDebugOutline(m_uiManager.getElement<Graph>(L"Graph 1"), false);
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